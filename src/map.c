#include "map.h"
#include <stdlib.h>
#include <string.h>
#include "bresenham3.h"

#include <ncurses.h>

#define MAP_TILE_PART       0xFF00 //xxxxxxxx00000000
#define MAP_FLAG_PART       0x00FF //00000000xxxxxxxx
#define MAP_FLAG_LIT_PART   0x00F0 //00000000xxxx0000
#define MAP_FLAG_VOL_PART   0x000C //000000000000xx00
#define MAP_FLAG_LOS_PART   0x0003 //00000000000000xx

#define MAP_IND(flgs) ((flgs & MAP_TILE_PART) >> 8)
#define MAP_SET_IND(flgs, ind) (((ind << 8) | (flgs & (~MAP_TILE_PART))))
#define MAP_FLAGS(flgs) (flgs & MAP_FLAG_PART)
#define MAP_SET_FLAGS(flgs, nflgs) (nflgs | (flgs & (~MAP_FLAG_PART)))
#define MAP_LIT(flgs) ((flgs & MAP_FLAG_LIT_PART) >> 4)
#define MAP_SET_LIT(flgs, lit) (((lit << 4) | (flgs & (~MAP_FLAG_LIT_PART))))
#define MAP_VOL(flgs) ((flgs & MAP_FLAG_VOL_PART) >> 2)
#define MAP_SET_VOL(flgs, vol) (((vol << 2) | (flgs & (~MAP_FLAG_VOL_PART))))
#define MAP_LOS(flgs) (flgs & MAP_FLAG_LOS_PART)
#define MAP_SET_LOS(flgs, los) (((los) | (flgs & (~MAP_FLAG_LOS_PART))))

Map map_new() {
  #warning new should use calloc, init should free all ptrs just in case
  //this is so that an object can be init-ed multiple times in a row to avoid some allocations
  return malloc(sizeof(struct _map));
}
Map map_init(
  Map m, 
  char *id, 
  mapVec sz, 
  unsigned short *tilemap,
  char ambientLight,
  void *ctx,
  void *baseTileCtx
) {
  m->id = strdup(id);
  m->sz = sz;
  m->tilemap = malloc(sz.x*sz.y*sz.z*sizeof(short));
  memcpy(m->tilemap, tilemap, sz.x*sz.y*sz.z*sizeof(short));
  for(int i = 0; i < sz.x*sz.y*sz.z; i++) {
    #warning not really sure about this ambient light stuff, etc.
    m->tilemap[i] = (tilemap[i] << 8) + ((ambientLight << 4) & MAP_FLAG_LIT_PART);
  }
  m->tileset = TCOD_list_new();
  map_add_tile(m, tile_init(tile_new(), 0, baseTileCtx));
  m->ambientLight = ambientLight;
  m->exits = TCOD_list_new();
  m->objects = TCOD_list_new();
  m->context = ctx;
  return m;
}
void map_free(Map m) {
  free(m->id);
  free(m->tilemap);
  for(int i = 0; i < TCOD_list_size(m->exits); i++) {
    exit_free(TCOD_list_get(m->exits, i));
  }
  TCOD_list_delete(m->exits);
  for(int i = 0; i < TCOD_list_size(m->tileset); i++) {
    tile_free(TCOD_list_get(m->tileset, i));
  }
  TCOD_list_delete(m->tileset);
  free(m);
}

void *map_context(Map m) {
  return m->context;
}

char *map_id(Map m) {
  return m->id;
}
void map_add_exit(Map m, Exit ex) {
  TCOD_list_push(m->exits, ex);
}
void map_remove_exit(Map m, Exit ex) {
  TCOD_list_remove(m->exits, ex);
}

void map_add_tile(Map m, Tile t) {
  TCOD_list_push(m->tileset, t);
}
Tile map_get_tile(Map m, int tileIndex) {
  return TCOD_list_get(m->tileset, tileIndex);
}

void map_add_object(Map m, Object o) {
  TCOD_list_push(m->objects, o);
}
void map_remove_object(Map m, Object o) {
  if(o != NULL) {
    TCOD_list_remove(m->objects, o);
    object_free(o);
  }
}
void map_remove_object_at(Map m, int i) {
  map_remove_object(m, map_get_object(m, i));
}
void map_remove_object_named(Map m, char *id) {
  map_remove_object(m, map_get_object_named(m, id));
}
Object map_get_object(Map m, int i) {
  return TCOD_list_get(m->objects, i);
}
Object map_get_object_named(Map m, char *id) {
  for(int i = 0; i < map_object_count(m); i++) {
    Object o = map_get_object(m, i);
    if(strcmp(object_id(o), id) == 0) {
      return o;
    }
  }
  return NULL;
}
int map_object_count(Map m) {
  return TCOD_list_size(m->objects);
}

void map_move_object(Map m, char *id, mapVec delta) {  
  Object o = map_get_object_named(m, id), o2;
  //the move updates its sensors
  object_move(o, delta);
  //update any sensors that might see it now
  for(int i = 0; i < map_object_count(m); i++) {
    o2 = map_get_object(m, i);
    if(o2 != o) {
      object_note_object_moved(o2, o, delta);
    }
  }
}


void map_turn_object(Map m, char *id, int amt) {
  Object o = map_get_object_named(m, id), o2;
  object_turn(o, amt);
  for(int i = 0; i < map_object_count(m); i++) {
    o2 = map_get_object(m, i);
    if(o2 != o) {
      #warning send a stimulus to all sensors that can see this object
    }
  }
}

void map_get_region(Map m, unsigned short *flags, mapVec start, mapVec end, mapVec bpt, mapVec bsz) {
  int index, destIndex;
  mapVec size = m->sz;
  for(int z = start.z; z <= end.z; z++) {
    for(int y = start.y; y <= end.y; y++) {
      for(int x = start.x; x <= end.x; x++) {
        index = map_tile_index(m, x, y, z);
        destIndex = tile_index(x, y, z, size, bpt, bsz);
        flags[destIndex] = MAP_FLAGS(m->tilemap[index]);
      }
    }
  }
}

int map_tile_index(Map m, int x, int y, int z) {
  return tile_index(x, y, z, m->sz, mapvec_zero, m->sz);
}

unsigned char map_tile_at(Map m, int x, int y, int z) {
  return map_tile_at_index(m, map_tile_index(m, x, y, z));
}

unsigned char map_tile_at_index(Map m, int i) {
  return MAP_IND(m->tilemap[i]);
}

Tile map_get_tiledef(Map m, int i) {
  return TCOD_list_get(m->tileset, i);
}

//returns the visibility of the previous tile (x,y,z) based on the visibility of the next tile in the path.
unsigned char map_trace_light(Map m, unsigned char *flags, TCOD_bresenham3_data_t *bd, mapVec bpos, mapVec bsz) {
  //try to trace to position through tiles that allow light to pass
  int x, y, z;
  //if the next tile is the position...
  if(TCOD_line3_step_mt(&x, &y, &z, bd)) {
    //the previous tile is in los.
    return 0x03;
  }
  unsigned int index      = map_tile_index(m, x, y, z);
  unsigned short mapItem  = m->tilemap[index];
  unsigned char tileIndex = MAP_IND(mapItem);
  Tile tileDef            = map_get_tiledef(m, tileIndex);
  unsigned char blockage  = tile_opacity(tileDef);

  unsigned int destIndex    = tile_index(x, y, z, map_size(m), bpos, bsz);
  unsigned char flg         = flags[destIndex];
  unsigned char los         = MAP_LOS(flg);
  
  //if the next tile is not in los...
  if(los == 0x01) {
    //the previous tiles must all be not in los as well.
    return 0x01; //bail and propagate
  } 
  //if the next tile is in los...
  if(los > 0x01)
  {
    //and the next tile is a wall...
    if(blockage == 0x03) {
      //the previous tile is not in los.
      return 0x01;
    //and the next tile is partially in los...
    } else if(blockage > 0x00 && blockage < 0x03) {
      //the previous tile is partially in los (later, use subtraction instead of a direct set)
      return 0x02;
    //and the next tile does not block los...
    } else {
      //the previous tile is as in-los as the next tile.
      return los;
    }
  }
  //otherwise, we're tracing through a tile with unknown los.
  //trace to the next tile, which may have a known los.
  unsigned char reclos = map_trace_light(m, flags, bd, bpos, bsz);
  flags[destIndex] = MAP_SET_LOS(flags[destIndex], reclos); 
  //if the next tile is not in los...
  if(reclos == 0x01) {
    //the previous tiles must all be not in los as well.
    return 0x01; //bail and propagate
  } 
  //if the next tile is in los...
  if(reclos > 0x01)
  {
    //and the next tile is a wall...
    if(blockage == 0x03) {
      //the previous tile is not in los.
      return 0x01;
    //and the next tile is partially in los...
    } else if(blockage > 0x00 && blockage < 0x03) {
      //the previous tile is partially in los (later, use subtraction instead of a direct set)
      return 0x02;
    //and the next tile does not block los...
    } else {
      //the previous tile is as in-los as the next tile.
      return reclos;
    }
  }
  //for some reason, we still don't know whether we're in los.
  return 0x00;
}
//refactor to support an interface that includes a Light and updates the lit flags rather than the viz flags.
void map_get_visible_tiles(Map m, unsigned char *flags, Volume vol, mapVec bpos, mapVec bsz) {
  //int times = 0;
  mapVec position = volume_position(vol);
  mapVec size = m->sz, cur;
  unsigned int index, destIndex;
  unsigned char litFlags, newFlags, los;
  unsigned short mapItem;
  
  int zstart = CLIP(bpos.z, 0, size.z);
  int zend = CLIP(bpos.z+bsz.z, 0, size.z);
  int ystart = CLIP(bpos.y, 0, size.y);
  int yend = CLIP(bpos.y+bsz.y, 0, size.y);
  int xstart = CLIP(bpos.x, 0, size.x);
  int xend = CLIP(bpos.x+bsz.x, 0, size.x);
  
  for(int z = zstart; z < zend; z++) {
    for(int y = ystart; y < yend; y++) {
      for(int x = xstart; x < xend; x++) {
        index = map_tile_index(m, x, y, z);
        destIndex = tile_index(x, y, z, size, bpos, bsz);

        mapItem   = m->tilemap[index];
        litFlags  = MAP_LIT(mapItem);    
        newFlags  = flags[destIndex];
        newFlags  = MAP_SET_LIT(newFlags, litFlags);
                
        cur = (mapVec){x, y, z};
        //vol is 0 0 if unsure, 1 1 if known vol, 1 0 if edge of vol, 0 1 if known out-of-vol.
        if(MAP_VOL(newFlags) == 0x00) {
          //if it's within the volume
          if(volume_contains_point(vol, cur, 0.0)) {
            newFlags = MAP_SET_VOL(newFlags, 0x03);
          } else {
            newFlags = MAP_SET_VOL(newFlags, 0x01);
          }
          flags[destIndex] = newFlags;
        }
        if(MAP_LOS(newFlags) == 0x00) {
          TCOD_bresenham3_data_t bd;
          TCOD_line3_init_mt(cur.x, cur.y, cur.z, position.x, position.y, position.z, &bd);
          //this is a recursive fn that is also destructive to flags.  keep that in mind!
          los = map_trace_light(m, flags, &bd, bpos, bsz);
          newFlags = MAP_SET_LOS(newFlags, los);
          flags[destIndex] = newFlags;
        }
      }
    }
  }
  //what about the exits?
}

void map_get_visible_objects(Map m, TCOD_list_t objs, unsigned char *visflags, mapVec bpt, mapVec bsz) {
  unsigned char flags;
  int index;
  mapVec pt;
  Object o;
  for(int i = 0; i < map_object_count(m); i++) {
    o = map_get_object(m, i);
    pt = object_position(o);
    index = tile_index(pt.x, pt.y, pt.z, map_size(m), bpt, bsz);
    flags = visflags[index];
    if(map_item_visible(flags)) {
      TCOD_list_push(objs, o);
    }
  }
}

mapVec map_size(Map m) {
  return m->sz;
}

unsigned char map_item_index(unsigned short mapItem) {
  return MAP_IND(mapItem);
}
unsigned char map_item_flags(unsigned short mapItem) {
  return MAP_FLAGS(mapItem);
}
int map_item_lit(unsigned short flags) {
  return MAP_LIT(flags) > 1;
}
int map_item_in_volume(unsigned short flags) {
  return MAP_VOL(flags) > 1;
}
int map_item_los(unsigned short flags) {
  return MAP_LOS(flags) > 1;
}
int map_item_visible(unsigned short flags) {
  return (MAP_LIT(flags) > 1) && (MAP_VOL(flags) > 1) && (MAP_LOS(flags) > 1);
}

//this is the interface for lighting up a room and changing its lighting properties.  there may also be an API for going through all lights and relighting from scratch.
//for a new light, something like "map_get_visible_tiles" can give the unobstructed tiles in the volume (in the vis flags), which can then be used to fill in the lighting data.
//when a light is removed, this should be done to unlight any areas lit by this light.
//moving or turning a light may be a two-step process for now, since the obstruction calculations will have to be reperformed. this seems inefficient.  Fortunately, since lights are global, all objects can share the same lighting state.

//another option is a separate method for returning lit status of a tile, or at least _separate storage_ of lit tiles per light, so that the oversaturation problem doesn't become a large bug (several full-brightness lights -- if naively done, removing one light might act as if all are removed)
void map_note_light_added(Map m, unsigned char attenuation, int intensity, Volume vol) {
  
}
void map_note_light_removed(Map m, unsigned char attenuation, int intensity, Volume vol) {
  
}
void map_note_light_volume_changed(Map m, unsigned char attenuation, int intensity, Volume old, Volume new) {
  
}
