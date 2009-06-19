#include "map.h"

#include <ncurses.h>
#include <stdlib.h>
#include "sensor.h"
#include "stimulus.h"
#include "volume.h"

/*
next
  lights
  memory optimization (view volume bounding box rather than map-sized storage)
  inter-room vision (option:map_get_visible tiles or objects returns a continuation or NULL.  the continuation says which volume is seeing, which rooms were seen into and through which portals, etc - this is also included with the corresponding stimulus.  The client can then get the continuation, if any, from the stimulus, and process it, potentially reporting the results back to the sensation engine (but probably Room should offer provisions for providing data for a continuation back to the client as if it were another stimulus for the same object [including returning another continuation if necessary].  cases like:
  
  ####     ####
  #  # ### #  #
  #a    b     #
  #  # ### #  #
  #           #
  #############
  
  and
            ####
  ####        b#
  #->b      #  #
  #a #      #  #
  #->b      #  #
  ####      #  #
               #
            ####
  are degenerate and will probably work inconsistently/be slow.  The first case will be slow because of a double continuation.
  the latter case is probably supportable if only one portal is visible, or if the overlap of the two portals can be portrayed somehow.  Perhaps the portals could be evaluated in separate continuations.
  
  WARNING: something similarly fancy will have to be done for lights.  Perhaps light propogation could be done by a continuation after movement or actions.  for instance, light A turning on in room 0 might need to cause a light recalculation in room 1, triggering a stimulus for its sensors.
  ))
  
lit terrain--either:
  A.) Light source info can be removed from tiles, saving a little storage and some complexity
  B.) Maps create light objects when they read in tiles with light source info

Also, if there's a way to remove some void*s from my mutually recursive dependencies, that would be great.
*/
//gcc -std=c99 -Ilibtcod/include main.c map.c tile.c exit.c geom.c sensor.c light.c object.c stimulus.c bresenham3_c.c -lncurses ./libtcod/libtcod.a -o test && ./test

Map createmap() {
  Map m = map_new();
  unsigned short tileMap[] = {
    2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2,
    
    2, 2, 2, 2, 2, 2, 2, 2,
    2, 0, 0, 0, 0, 0, 0, 2,
    2, 0, 0, 0, 0, 0, 0, 2,
    2, 0, 0, 0, 0, 0, 0, 2,
    2, 0, 0, 0, 0, 0, 0, 2,
    2, 0, 0, 0, 0, 0, 0, 2,
    2, 0, 0, 0, 0, 0, 0, 2,
    2, 2, 2, 2, 2, 2, 2, 2,    
    
    2, 2, 2, 2, 2, 2, 2, 2,
    2, 0, 0, 0, 0, 0, 0, 2,
    2, 0, 0, 0, 0, 0, 0, 2,
    2, 0, 0, 0, 0, 0, 0, 2,
    2, 0, 0, 0, 0, 0, 0, 2,
    2, 0, 0, 0, 0, 0, 0, 2,
    2, 0, 0, 0, 0, 0, 0, 2,
    2, 0, 0, 2, 2, 2, 2, 2,    
    
    2, 2, 2, 2, 2, 2, 2, 2,
    2, 1, 1, 1, 1, 1, 1, 1,
    2, 1, 1, 1, 1, 1, 1, 1,
    2, 1, 1, 2, 2, 1, 1, 2,
    2, 1, 1, 2, 2, 1, 1, 2,
    2, 1, 1, 1, 1, 1, 1, 2,
    2, 1, 1, 1, 1, 1, 1, 2,
    2, 1, 1, 2, 1, 1, 2, 2   
    };
  m = map_init(m, 
    "test_room", 
    (mapVec){8, 8, 4}, 
    tileMap,
    3
  ); 
  Tile floorTile = tile_init(
    tile_new(), 
    0, 
    0, 
    0,

    1, //0=null,1=floor,2=step,3=wall
    mapvec_zero,
    0,

    0,
    0,
    0,
    0<<7+0<<5+0 //col lit lit etc etc etc etc etc
  );
  Tile wallTile = tile_init(
    tile_new(), 
    0, 
    0, 
    0,
  
    3, //0=null,1=floor,2=step,3=wall
    mapvec_zero,
    0,
  
    0,
    0,
    0,
    (char)(1<<7)+(char)(3<<5)+0 //col lit lit etc etc etc etc etc
  );
  map_add_tile(m, floorTile);
  map_add_tile(m, wallTile);
  
  map_add_object(m, object_init(object_new(), "a", (mapVec){1, 1, 0}, (mapVec){1, 1, 0}, m));
  map_add_object(m, object_init(object_new(), "b", (mapVec){3, 1, 0}, (mapVec){1, 1, 0}, m));
  map_add_object(m, object_init(object_new(), "c", (mapVec){2, 4, 0}, (mapVec){1, 1, 0}, m));
  map_add_object(m, object_init(object_new(), "d", (mapVec){6, 6, 0}, (mapVec){1, 1, 0}, m));

  return m;
}

void drawtiles(Map m, unsigned char *buf, Sensor s, mapVec pos, mapVec size) {
  int index=0;
  char vz, lt;
  unsigned char tileIndex;
  unsigned char flags;
  int drawX, drawY;
  Volume vol = sensor_volume(s);
  int pairNo = 0;
  for(int z = pos.z; z < pos.z+size.z; z++) {
    for(int y = pos.y; y < pos.y+size.y; y++) {
      for(int x = pos.x; x < pos.x+size.x; x++) {
        index = map_tile_index(m, x, y, z);
        flags = buf[index];
        tileIndex = map_tile_at_index(m, index);
        drawX = x*2+z*((size.x*2)+1);
        drawY = y;
        // mapVec pt = (mapVec){x, y, z};
        // int passLeft = (plane_classify_point(vol->vol.frustum.left, pt, 0.0) != NegativeHalfSpace);
        // int passRight = (plane_classify_point(vol->vol.frustum.right, pt, 0.0) != NegativeHalfSpace);
        // int passUp = (plane_classify_point(vol->vol.frustum.up, pt, 0.0) != NegativeHalfSpace);
        // int passDown = (plane_classify_point(vol->vol.frustum.down, pt, 0.0) != NegativeHalfSpace);
        // int passNear = (plane_classify_point(vol->vol.frustum.near, pt, 0.0) != NegativeHalfSpace);
        // int passFar = (plane_classify_point(vol->vol.frustum.far, pt, 0.0) != NegativeHalfSpace);
        // if(passLeft && passRight && passUp && passDown && passNear && passFar) {
        //   pairNo = 2; //g
        // } else if(passLeft && passRight) {
        //   pairNo = 4; //b
        // } else if(passNear && passFar) {
        //   pairNo = 3; //y
        // } else if(passUp && passDown) {
        //   pairNo = 0; //white
        // } else {
        //   pairNo = 1; //r
        // }
        attron(COLOR_PAIR(pairNo));
        if(map_item_visible(flags)) {
          mvprintw(drawY, drawX, "%i ", tileIndex); //visible and lit and in volume
        }
        else if(!map_item_lit(flags) && map_item_in_volume(flags) && map_item_los(flags)) {
          mvprintw(drawY, drawX, "_ "); //not lit and viewable
        }
        else if(!map_item_lit(flags) && map_item_in_volume(flags) && !map_item_los(flags)) {
          mvprintw(drawY, drawX, ", "); //not lit and not los
        }
        else if(!map_item_lit(flags) && !map_item_in_volume(flags) && map_item_los(flags)) {
          mvprintw(drawY, drawX, "d "); //not lit and not in vol
        }
        else if(map_item_lit(flags) && !map_item_in_volume(flags) && map_item_los(flags)) {
          mvprintw(drawY, drawX, "a "); //lit and in los, but not in vol
        }
        else if(map_item_lit(flags) && map_item_in_volume(flags) && !map_item_los(flags)) {
          mvprintw(drawY, drawX, "b "); //lit and in vol, but not in los
        }
        else if(map_item_lit(flags) && !map_item_in_volume(flags) && !map_item_los(flags)) {
          mvprintw(drawY, drawX, ". "); //lit and not in vol or los (or los wasn't checked)
        }
        else if(!map_item_lit(flags) && !map_item_in_volume(flags) && !map_item_los(flags)) { //not lit, in vol, or in los
          mvprintw(drawY, drawX, "x ");
        }
        attroff(COLOR_PAIR(pairNo));
      }
    }
  }
}

void draw_object(Stimulus st) {
  unsigned char visflags = stimulus_obj_sight_change_get_new_flags(st);
  mapVec pos = stimulus_obj_sight_change_get_position(st);
  char *id = stimulus_obj_sight_change_get_id(st);
  if(!map_item_visible(visflags)) {
    mvprintw(pos.y, pos.x*2, "X");
  } else {
    mvprintw(pos.y, pos.x*2, id);
  }
}

void drawstimuli(Map m, Sensor s) {
  Object o;
  TCOD_list_t stims = sensor_consume_stimuli(s);
  unsigned char *tiles;
  mapVec pos, size, oldPt, delta;
  unsigned char visflags;
  for(int i = 0; i < TCOD_list_size(stims); i++) {
    //this is a very naive approach that completely ignores the possibility of overdraw and 'forgets' object positions
    Stimulus st = TCOD_list_get(stims, i);
    stimtype type = stimulus_type(st);
    mvprintw(10, i*2, "%i", type);
    switch(type) {
      case StimTileLitChange:
      case StimTileVisChange:
        //redraw all tiles
        tiles = stimulus_tile_sight_change_get_new_tiles(st);
        pos = stimulus_tile_sight_change_get_position(st);
        size = stimulus_tile_sight_change_get_size(st);
        drawtiles(m, tiles, s, pos, size);
        break;
      case StimObjLitChange:
      case StimObjVisChange:
        //redraw object
        draw_object(st);
        break;
      case StimObjMoved:
        visflags = stimulus_obj_sight_change_get_new_flags(st);
        pos = stimulus_obj_sight_change_get_position(st);
        delta = stimulus_obj_moved_get_dir(st);
        oldPt = mapvec_subtract(pos, delta);
        mvprintw(oldPt.y, oldPt.x*2, "x");
        draw_object(st);
        mvprintw(15, 0, "got move");
        break;
      case StimGeneric:
      default:
        mvprintw(16, i*9, "generic %d", i);
        break;
    }
    stimulus_free(st);
  }
  TCOD_list_delete(stims);
}

void drawmap(Map m, Object o) {
//  sensor_sense(s);
  Sensor s;
  for(int i = 0; i < object_sensor_count(o); i++) {
    s = object_get_sensor(o, i);
    drawstimuli(m, s);
    mvprintw(13+i, 0, "%f %f %f", sensor_facing(s).x, sensor_facing(s).y, sensor_facing(s).z);
  }
}

int main() {
  int ch;
  int finished = 0;
  initscr(); cbreak(); noecho();
  nonl();
  intrflush(stdscr, FALSE);
  keypad(stdscr, TRUE);
  start_color();
  init_pair(1, COLOR_RED, COLOR_BLACK);
  init_pair(2, COLOR_GREEN, COLOR_BLACK);
  init_pair(3, COLOR_YELLOW, COLOR_BLACK);
  init_pair(4, COLOR_BLUE, COLOR_BLACK);
//  halfdelay();
  Map m = createmap();
  Object playerObj = object_init(object_new(), 
    "@", 
    (mapVec){3, 1, 0}, 
    (mapVec){1, 1, 0},
    m
  );
  map_add_object(m, playerObj);
  Light playerLamp = light_init(light_new(),
    "lamp",
    sphere_init(
      sphere_new(),
      mapvec_zero,
      2
    ),
    0, //0 attenuation - stop immediately at max range
    2  //brightness 2
  );
  object_add_light(playerObj, playerLamp);
  
  Sensor player = sensor_init(sensor_new(), 
    "player", 
    sphere_init(sphere_new(),
      mapvec_zero, 
      4
    )
  );
  object_add_sensor(playerObj, player);
  // Sensor leftEye = sensor_init(sensor_new(), "left_eye",
  //   frustum_init(frustum_new(),
  //     mapvec_zero,
  //     (mapVec){1, -1, 0},
  //     1, 2,
  //     0, 10
  //   )
  // );
  // Sensor rightEye = sensor_init(sensor_new(), "right_eye",
  //   frustum_init(frustum_new(),
  //     mapvec_zero,
  //     (mapVec){1, 1, 0},
  //     1, 2,
  //     0, 10
  //   )
  // );
  // Sensor basicSense = sensor_init(sensor_new(), "basic_sense",
  //   sphere_init(sphere_new(),
  //     mapvec_zero,
  //     2
  //   )
  // );
  // object_add_sensor(playerObj, leftEye);
  // object_add_sensor(playerObj, rightEye);
  // object_add_sensor(playerObj, basicSense);
    
//  sensor_sense(player);
  ch = getch();
  while(!finished) {
    clear();
    drawmap(m, playerObj);
    mvprintw(object_position(playerObj).y, object_position(playerObj).x*2, "@");
    ch = getch();
    switch(ch) {
      case KEY_RIGHT:
        map_turn_object(m, "@", 1);
        break;
      case KEY_LEFT:
        map_turn_object(m, "@", -1);
        break;
      case 'w':
        map_move_object(m, "@", (mapVec){0, -1, 0});
        break;
      case 'a':
        map_move_object(m, "@", (mapVec){-1, 0, 0});
        break;
      case 's':
        map_move_object(m, "@", (mapVec){0, 1, 0});
        break;
      case 'd':
        map_move_object(m, "@", (mapVec){1, 0, 0});
        break;
      case 'i':
        map_move_object(m, "a", (mapVec){0, -1, 0});
        break;
      case 'j':
        map_move_object(m, "a", (mapVec){-1, 0, 0});
        break;
      case 'k':
        map_move_object(m, "a", (mapVec){0, 1, 0});
        break;
      case 'l':
        map_move_object(m, "a", (mapVec){1, 0, 0});
        break;
      case 'q':
        finished = 1;
        break;
      default:
        break;
    }
    refresh();
  }
  endwin();
  return 0;
}