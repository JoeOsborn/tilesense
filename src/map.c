#include "map.h"

#include <stdlib.h>
#include <string.h>

//use chars for tile indices...
//then have an int array for all the tile data.  
//would use flagsets, but it's expensive to return a new flagset for stimulus purposes.
//then again, if each stimulus stores a flagset, it's no issue -- we just do copies and sets.
//still, the best approach may be to use a bitfield...


//This way, the light tracer can check hitting the top, hitting the underside, and going through the surface.
//Coming in from x or y can safely use just surface -- but the z stuff has to refer to 'underside' and 'top'.
//If your bottom lets light out, hit the next top; if that top lets light in, go through to the surface; if that bottom lets light out, stop there; otherwise, keep going to the next top, and so on.  Similar in reverse, but switch out/in.
//Do we need more opacity data on tiles for this to work?  Yes, we need entry and exit data for top and bottom, not just exit data.  This is important because light can't leave or enter the bottom of a regular floor, but light can enter the bottom of a one-way glass floor (though it can't leave it); light can't leave or enter a regular tight ceiling, but it can enter a one-way glass ceiling; and even if the ceiling lets light in, the floor might not let light out.
//the extra opacity data is in, now we have to rewrite the light tracer.

Map map_new() {
  #warning new should use calloc, init should free all ptrs just in case
  //this is so that an object can be init-ed multiple times in a row to avoid some allocations
  return malloc(sizeof(struct _map));
}
Map map_init(
  Map m, 
  char *id, 
  mapVec sz, 
  unsigned char *tilemap,
  unsigned char ambientLight,
  void *ctx,
  void *baseTileCtx
) {
  m->id = strdup(id);
  m->sz = sz;
  m->tilemap = malloc(sz.x*sz.y*sz.z*sizeof(unsigned char));
  memcpy(m->tilemap, tilemap, sz.x*sz.y*sz.z*sizeof(unsigned char));
  m->perceptmap = calloc(sz.x*sz.y*sz.z, sizeof(perception));
  for(int i = 0; i < sz.x*sz.y*sz.z; i++) {
    #warning not really sure about this ambient light stuff, etc.
    m->tilemap[i] = tilemap[i];
    m->perceptmap[i].underlit=ambientLight;
    m->perceptmap[i].surflit=ambientLight;
    m->perceptmap[i].toplit=ambientLight;
  }
  m->tileset = TCOD_list_new();
  map_add_tile(m, tile_init(tile_new(), 
    tile_opacity_flagset_set(tile_opacity_flagset_make(), 
      0,0,
      0,0,
      0,0,
      0,0
    ), 
    baseTileCtx
  ));
  m->ambientLight = ambientLight;
  m->exits = TCOD_list_new();
  m->objects = TCOD_list_new();
  m->context = ctx;
  return m;
}
void map_free(Map m) {
  free(m->id);
  free(m->tilemap);
  free(m->perceptmap);
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

void map_get_region(Map m, perception *percept, mapVec start, mapVec end, mapVec bpt, mapVec bsz) {
  int index, destIndex;
  mapVec size = m->sz;
  for(int z = start.z; z <= end.z; z++) {
    for(int y = start.y; y <= end.y; y++) {
      for(int x = start.x; x <= end.x; x++) {
        index = map_tile_index(m, x, y, z);
        destIndex = tile_index(x, y, z, size, bpt, bsz);
        percept[destIndex] = m->perceptmap[index];
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
  return m->tilemap[i];
}

Tile map_get_tiledef(Map m, int i) {
  return TCOD_list_get(m->tileset, i);
}

//returns the los of the previous tile (x,y,z) based on the blockage of the next tile in the path.
unsigned char map_trace_light(Map m, perception *percept, TCOD_bresenham3_data_t *bd, mapVec bpos, mapVec bsz, int pX, int pY, int pZ) {
  
  
  
  //this could probably use a rewrite to properly populate surface/under/top.
    
  //try to trace to position through tiles that allow light to pass
  int x, y, z;
  //if the next tile is the position...
  bool done = TCOD_line3_step_mt(&x, &y, &z, bd);
  if(done) {
    x = bd->destx;
    y = bd->desty;
    z = bd->destz;
  }
  unsigned int index      = map_tile_index(m, x, y, z);
  unsigned char tileIndex = m->tilemap[index];
  Tile tileDef            = map_get_tiledef(m, tileIndex);
  #warning wrong, we need to do this in two steps for all z-transitions
  //and use z blockage info from the new tile and the old tile.
  Direction direction     = direction_light_between(x,y,z,pX,pY,pZ,z);
  // if(z != pZ) {
  //   
  // }
  
  //light leaving out of the next tile towards the previous tile
  unsigned char blockage  = tile_opacity_direction(tileDef, direction);

  if(done) {
    //the previous tile is in los if the next tile does not block it.
    if(blockage >= 10) { //refactor this stuff! this is really not quite right -- should use amount of light transfer.
      //the previous tile is not in los.
      return 0x01;
    //and the next tile is partially in los...
    } else if(blockage >= 0x05 && blockage < 0x10) {
      //the previous tile is partially in los (later, use subtraction instead of a direct set, use higher-bitdepth los (see comment on previous if))
      return 0x02;
    //and the next tile does not block los...
    } else {
      //the previous tile is certainly in los
      return 0x03;
    }
  }
  
  unsigned int destIndex  = tile_index(x, y, z, map_size(m), bpos, bsz);
  perception flg          = percept[destIndex];
  unsigned char los       = flg.surflos;
  
  //if the next tile is not in los...
  if(los == 0x01) {
    //the previous tiles must all be not in los as well.
    return 0x01; //bail and propagate
  } 
  //if the next tile is in los...
  if(los > 0x01)
  {
    //and the next tile won't permit light towards us...
    if(blockage >= 10) { //this is really not quite right -- should use amount of light transfer.
      //the previous tile is not in los.
      return 0x01;
    //and the next tile is partially in los...
    } else if(blockage >= 0x05 && blockage < 0x10) {
      //the previous tile is partially in los (later, use subtraction instead of a direct set, use higher-bitdepth los (see comment on previous if))
      return 0x02;
    //and the next tile does not block los...
    } else {
      //the previous tile is as in-los as the next tile.
      return los;
    }
  }
  //otherwise, we're tracing through a tile with unknown los.
  //trace to the next tile, which may have a known los.
  unsigned char reclos = map_trace_light(m, percept, bd, bpos, bsz, x, y, z);
  percept[destIndex].toplos = reclos;
  percept[destIndex].surflos = reclos;
  //if the next tile is not in los...
  if(reclos == 0x01) {
    //the previous tiles must all be not in los as well.
    return 0x01; //bail and propagate
  } 
  //if the next tile is in los...
  if(reclos > 0x01)
  {
    //and the next tile blocks light in this direction...
    if(blockage >= 10) {
      //the previous tile is not in los.
      return 0x01;
    //and the next tile is partially in los...
    } else if(blockage >= 5 && blockage < 10) {
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
void map_get_visible_tiles(Map m, perception *percept, Volume vol, mapVec bpos, mapVec bsz) {
  //int times = 0;
  mapVec position = volume_position(vol);
  mapVec size = m->sz, cur;
  unsigned int index, destIndex;
  perception newFlags;
  unsigned char los, litFlags;
  perception mapItem;
  
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

        mapItem   = m->perceptmap[index];
        litFlags  = mapItem.surflit;
        newFlags  = percept[destIndex];
        newFlags.toplit = litFlags;
        newFlags.surflit = litFlags;
                
        cur = (mapVec){x, y, z};
        //vol is 0 0 if unsure, 1 1 if known vol, 1 0 if edge of vol, 0 1 if known out-of-vol.
        if(newFlags.surfvol == 0x00) { //not right anymore, three vol flags?
          //if it's within the volume
          if(volume_contains_point(vol, cur, 0.0)) {
            newFlags.topvol = 0x03;
            newFlags.surfvol = 0x03;
            newFlags.undervol = 0x03;
          } else {
            newFlags.topvol = 0x01;
            newFlags.surfvol = 0x01;
            newFlags.undervol = 0x01;
          }
          percept[destIndex] = newFlags;
        }
        if(newFlags.surflos == 0x00) { //not right anymore, three vol flags?
          TCOD_bresenham3_data_t bd;
          TCOD_line3_init_mt(cur.x, cur.y, cur.z, position.x, position.y, position.z, &bd);
          //this is a recursive fn that is also destructive to flags.  keep that in mind!
          //trace from this tile to the sensor
          los = map_trace_light(m, percept, &bd, bpos, bsz, x, y, z);
          newFlags.toplos = los;
          newFlags.surflos = los;
          percept[destIndex] = newFlags;
        }
      }
    }
  }
  //what about the exits?
}

void map_get_visible_objects(Map m, TCOD_list_t objs, perception *visflags, mapVec bpt, mapVec bsz) {
  perception flags;
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

//convenience methods --all mean "top or surface", do not include underside
bool map_item_lit(perception pcpt) {
  return pcpt.toplit || pcpt.surflit;
}
bool map_item_in_volume(perception pcpt) {
  return pcpt.topvol || pcpt.surfvol;
}
bool map_item_los(perception pcpt) {
  return pcpt.toplos || pcpt.surflos;
}
//lit and in volume and in los
bool map_item_visible(perception pcpt) {
  return map_item_lit(pcpt) && map_item_in_volume(pcpt) && map_item_los(pcpt);
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
