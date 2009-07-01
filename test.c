#include "map.h"

#include <stdlib.h>
#include "sensor.h"
#include "stimulus.h"
#include "volume.h"
#include <libtcod.h>

/*
next
  some refactoring?  Lots of places seem to do the swept_bounds stuff, and it would be nice to minimize the propagation of that stuff.
  large objects!  maybe if objects can have a volume -- this requires calculating the intersections of volumes, which sucks a lot.  what if objects had a list of positions, or a list of Parts that were each "one square with some properties"?  the Parts approach seems the lowest-impedance.  It does suggest that we'll want some broad-pass object-presence detection so we don't have to iterate over every object (or every part) for every sensor.
  lights
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
  
  rather than continuations, it may be better to create dummy sensors in the other rooms and figure out the LOS data for the exit tiles; from those two pieces of info, it should be possible to correctly sense the right tiles.  then, the client's job is to aggregate the stimuli from multiple rooms and send out updates to multiple rooms.
  ))
  
lit terrain--either:
  A.) Light source info can be removed from tiles, saving a little storage and some complexity
  B.) Maps create light objects when they read in tiles with light source info

Also, if there's a way to remove some void*s from my mutually recursive dependencies, that would be great.
*/

// make -f osx/makefile test && ./test

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
    0
  );
  Tile wallTile = tile_init(
    tile_new(), 
    1
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
  unsigned char tileIndex;
  unsigned char flags;
  int drawX, drawY;
  Volume vol = sensor_volume(s);
  mapVec borig, bsz;
  volume_swept_bounds(vol, &borig, &bsz);
  mapVec msz = map_size(m);
  float zstart = CLIP(pos.z, 0, msz.z);
  float ystart = CLIP(pos.y, 0, msz.y);
  float xstart = CLIP(pos.x, 0, msz.x);
  float zend = CLIP(pos.z+size.z, 0, msz.z);
  float yend = CLIP(pos.y+size.y, 0, msz.y);
  float xend = CLIP(pos.x+size.x, 0, msz.x);
  for(int z = zstart; z < zend; z++) {
    for(int y = ystart; y < yend; y++) {
      for(int x = xstart; x < xend; x++) {
        index = tile_index(x, y, z, msz, borig, bsz);
        flags = buf[index];
        TCOD_console_print_left(NULL, 0, 18, TCOD_BKGND_NONE, "%i, %i, %i", map_item_lit(flags), map_item_in_volume(flags), map_item_los(flags));
        tileIndex = map_tile_at(m, x, y, z);
        drawX = x*2+z*((msz.x*2)+1);
        drawY = y;
        //TCOD_console_print_left(NULL, drawX, drawY, TCOD_BKGND_NONE, "%i", index);
        if(map_item_visible(flags)) {
           //visible and lit and in volume
           TCOD_console_print_left(NULL, drawX, drawY, TCOD_BKGND_NONE, "%i", tileIndex);
        }
        // else if(!map_item_lit(flags) && map_item_in_volume(flags) && map_item_los(flags)) {
        //   //not lit and viewable
        //   TCOD_console_print_left(NULL, drawX, drawY, TCOD_BKGND_NONE, "_");
        // }
        // else if(!map_item_lit(flags) && map_item_in_volume(flags) && !map_item_los(flags)) {
        //   //not lit and not los
        //   TCOD_console_print_left(NULL, drawX, drawY, TCOD_BKGND_NONE, ",");
        // }
        // else if(!map_item_lit(flags) && !map_item_in_volume(flags) && map_item_los(flags)) {
        //   //not lit and not in vol
        //   TCOD_console_print_left(NULL, drawX, drawY, TCOD_BKGND_NONE, "d");
        // }
        // else if(map_item_lit(flags) && !map_item_in_volume(flags) && map_item_los(flags)) {
        //   //lit and in los, but not in vol
        //   TCOD_console_print_left(NULL, drawX, drawY, TCOD_BKGND_NONE, "a");
        // }
        // else if(map_item_lit(flags) && map_item_in_volume(flags) && !map_item_los(flags)) {
        //   //lit and in vol, but not in los
        //   TCOD_console_print_left(NULL, drawX, drawY, TCOD_BKGND_NONE, "b");
        // }
        // else if(map_item_lit(flags) && !map_item_in_volume(flags) && !map_item_los(flags)) {
        //   //lit and not in vol or los (or los wasn't checked)
        //   TCOD_console_print_left(NULL, drawX, drawY, TCOD_BKGND_NONE, ".");
        // }
        // else if(!map_item_lit(flags) && !map_item_in_volume(flags) && !map_item_los(flags)) { 
        //   //not lit, in vol, or in los
        //   TCOD_console_print_left(NULL, drawX, drawY, TCOD_BKGND_NONE, "x");
        // }
      }
    }
  }
}

void draw_object(Stimulus st) {
  unsigned char visflags = stimulus_obj_sight_change_get_new_flags(st);
  mapVec pos = stimulus_obj_sight_change_get_position(st);
  char *id = stimulus_obj_sight_change_get_id(st);
  if(!map_item_visible(visflags)) {
    TCOD_console_print_left(NULL, pos.x*2, pos.y, TCOD_BKGND_NONE, "X");
  } else {
    TCOD_console_print_left(NULL, pos.x*2, pos.y, TCOD_BKGND_NONE, id);
  }
}

void drawstimuli(Map m, Sensor s) {
  TCOD_list_t stims = sensor_consume_stimuli(s);
  unsigned char *tiles;
  mapVec pos, size, oldPt, delta;
  unsigned char visflags;
  if(TCOD_list_size(stims) > 0) {
    TCOD_console_print_left(NULL, 0, 10, TCOD_BKGND_NONE, "                            ");
  }
  for(int i = 0; i < TCOD_list_size(stims); i++) {
    //this is a very naive approach that completely ignores the possibility of overdraw and 'forgets' object positions
    Stimulus st = TCOD_list_get(stims, i);
    stimtype type = stimulus_type(st);
    TCOD_console_print_left(NULL, i*2, 10, TCOD_BKGND_NONE, "s%i", type);
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
        TCOD_console_print_left(NULL, oldPt.x*2, oldPt.y, TCOD_BKGND_NONE, "x");
        draw_object(st);
        TCOD_console_print_left(NULL, 0, 15, TCOD_BKGND_NONE, "got move");
        break;
      case StimGeneric:
      default:
        TCOD_console_print_left(NULL, i*9, 16, TCOD_BKGND_NONE, "generic %d", i);
        break;
    }
    stimulus_free(st);
  }
  TCOD_list_delete(stims);
}

void drawmap(Map m, Object o) {
  Sensor s;
  for(int i = 0; i < object_sensor_count(o); i++) {
    s = object_get_sensor(o, i);
    drawstimuli(m, s);
    TCOD_console_print_left(NULL, 0, 13+i, TCOD_BKGND_NONE, "<%f %f %f>", sensor_facing(s).x, sensor_facing(s).y, sensor_facing(s).z);
  }
}

#include "flagset.h"

void assert(int fact) {
  if(!fact) { 
    exit(-1); 
  }
}

void test_flagset_raw() {
  Flagset fs = flagset_init_raw(flagset_new_raw(24), 24);
  //bits 0-3: 1011 = 8+2+1 = 11
  //bits 4-11: 11001100 = 128+64+8+4 = 204
  //bits 12-13: 11 = 2+1 = 3
  //bits 14-19: 110011 = 32+16+2+1 = 51
  //bits 20-23: 1011 = 8+2+1 = 11
  //1011 1100  1100 1111  0011 1011
  flagset_set_raw(fs, 0, 4, 11);
  flagset_set_raw(fs, 4, 8, 204);
  flagset_set_raw(fs, 12, 2, 3);
  flagset_set_raw(fs, 14, 6, 51);
  flagset_set_raw(fs, 20, 4, 11);
  assert(flagset_get_raw_large(fs, 0, 16) == 48335);
  assert(flagset_get_raw_large(fs, 4, 16) == 52467);
  assert(flagset_get_raw_large(fs, 4, 20) == 839483);
  assert(flagset_get_raw(fs, 0, 4) == 11);
  assert(flagset_get_raw(fs, 4, 8) == 204);
  assert(flagset_get_raw(fs, 12, 2) == 3);
  assert(flagset_get_raw(fs, 14, 6) == 51);
  assert(flagset_get_raw(fs, 20, 4) == 11);
  flagset_set_raw(fs, 0, 4, 4);
  assert(flagset_get_raw(fs, 0, 4) == 4);
  flagset_set_raw(fs, 4, 8, 113);
  assert(flagset_get_raw(fs, 4, 8) == 113);
  //1100 0001  0010
  flagset_set_raw_large(fs, 0, 12, 3090);
  assert(flagset_get_raw_large(fs, 0, 12) == 3090);
  flagset_free(fs);
  fs = NULL;
}

void test_flagschema() {
  FlagSchema fsc = flagschema_init(flagschema_new());
  flagschema_insert(fsc, "collision.normal", 2);
  flagschema_insert(fsc, "collision.unusual", 2);
  flagschema_insert(fsc, "collision.dry", 1);
  flagschema_insert(fsc, "collision.muffin", 3);
  assert(TCOD_list_size(fsc) == 1);
  assert(flagschema_net_size(fsc) == 8);
  Flagset fs = flagset_init(flagset_new(fsc), fsc);
  //1111 1111
  flagset_set_raw_large(fs, 0, flagschema_net_size(fsc), 0xFFFFFFFF);
  //1110 1111
  flagset_set_path(fs, fsc, "collision.unusual", 2);
  assert(flagset_get_path(fs, fsc, "collision.unusual") == 2);
  //1110 1011
  FlagSchema collisionSub = flagschema_index_get_subschema(fsc, 0);
  flagset_set_index(fs, collisionSub, 3, 3);
  assert(flagset_get_index(fs, collisionSub, 3) == 3);
  
  flagschema_free(fsc);
  flagset_free(fs);
}

int main(int argc, char **argv) {  
//  test_flagset_raw();
  test_flagschema();
  
  int finished = 0;

  char *font="libtcod/fonts/courier12x12_aa_tc.png";
  int nb_char_horiz=0,nb_char_vertic=0;
  int font_flags=TCOD_FONT_TYPE_GREYSCALE|TCOD_FONT_LAYOUT_TCOD;
	TCOD_console_set_custom_font(font,font_flags,nb_char_horiz,nb_char_vertic);
	TCOD_console_init_root(80,24,"tilesense demo",false);

  Map m = createmap();
  Object playerObj = object_init(object_new(), 
    "@", 
    (mapVec){3, 1, 0}, 
    (mapVec){1, 1, 0},
    m
  );
  map_add_object(m, playerObj);
  #warning still need to do lights!
  // Light playerLamp = light_init(light_new(),
  //   "lamp",
  //   sphere_init(
  //     sphere_new(),
  //     mapvec_zero,
  //     2
  //   ),
  //   0, //0 attenuation - stop immediately at max range
  //   2  //brightness 2
  // );
  // object_add_light(playerObj, playerLamp);

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
  
  Sensor player = sensor_init(sensor_new(), 
    "player", 
    sphere_init(sphere_new(),
      mapvec_zero, 
      4
    )
  );
  object_add_sensor(playerObj, player);
    
  object_sense(playerObj);
  
  
  TCOD_sys_set_fps(30);
	TCOD_console_set_foreground_color(NULL,TCOD_white);
	
  drawmap(m, playerObj);
  TCOD_console_flush();

	TCOD_key_t key = {TCODK_NONE,0};
	do {
		/* did the user hit a key ? */
		key = TCOD_console_check_for_keypress(TCOD_KEY_PRESSED);
		if(key.vk != TCODK_NONE) {
		  TCOD_console_clear(NULL);
		}
		drawmap(m, playerObj);
    TCOD_console_print_left(NULL, object_position(playerObj).x*2, object_position(playerObj).y, TCOD_BKGND_NONE,"@");
		/* update the game screen */
		TCOD_console_flush();
    if(key.vk == TCODK_RIGHT) {
      map_turn_object(m, "@", 1);
    } else if(key.vk == TCODK_LEFT) {
      map_turn_object(m, "@", -1);
    } else if(key.vk == TCODK_CHAR) {
      switch(key.c) {
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
    }
	} while (!finished && !TCOD_console_is_window_closed());
  return 0;
}