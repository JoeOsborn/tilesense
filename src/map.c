#include "map.h"

#include <stdlib.h>
#include <string.h>

#include <stdio.h>

#include "tslist.h"

#include <assert.h>

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
  m->fovmap = TCOD_map3_new(sz.x,sz.y,sz.z);
  m->fovmapDirty = true;
  for(int i = 0; i < sz.x*sz.y*sz.z; i++) {
    #warning not really sure about this ambient light stuff, etc.
    m->tilemap[i] = tilemap[i];
    m->perceptmap[i].underlit=ambientLight;
    m->perceptmap[i].surflit=ambientLight;
    m->perceptmap[i].toplit=ambientLight;
  }
  m->tileset = TCOD_list_new();
  map_add_tile(m, tile_init(tile_new(), 
    1,1,1,
    baseTileCtx
  ));
  m->ambientLight = ambientLight;
  // m->exits = TCOD_list_new();
  m->objects = TCOD_list_new();
  m->objectMap = objectmap_init(objectmap_new(), sz);
  m->context = ctx;
  return m;
}
void map_free(Map m) {
  free(m->id);
  free(m->tilemap);
  free(m->perceptmap);
  objectmap_free(m->objectMap);
  TCOD_map3_delete(m->fovmap);
//  TS_LIST_CLEAR_AND_DELETE(m->exits, exit);
  TS_LIST_CLEAR_AND_DELETE(m->tileset, tile);
  TS_LIST_CLEAR_AND_DELETE(m->objects, object);
  free(m);
}

void *map_context(Map m) {
  return m->context;
}

char *map_id(Map m) {
  return m->id;
}
// void map_add_exit(Map m, Exit ex) {
//   TCOD_list_push(m->exits, ex);
// }
// void map_remove_exit(Map m, Exit ex) {
//   TCOD_list_remove(m->exits, ex);
// }

void map_add_tile(Map m, Tile t) {
  TCOD_list_push(m->tileset, t);
  m->fovmapDirty=true;
}
Tile map_get_tile(Map m, int tileIndex) {
  return TCOD_list_get(m->tileset, tileIndex);
}

//adding/removing/moving large objects might result in fovmap dirtiness too

void map_add_object(Map m, Object o) {
  TCOD_list_push(m->objects, o);
  objectmap_insert(m->objectMap, o);
}
void map_remove_object(Map m, Object o) {
  if(o != NULL) {
    TCOD_list_remove(m->objects, o);
    objectmap_remove(m->objectMap, o);
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
TCOD_list_t map_objects_at(Map m, int x, int y, int z) {
  return map_objects_at_position(m, (mapVec){x, y, z});
}
TCOD_list_t map_objects_at_position(Map m, mapVec pos) {
  return objectmap_get(m->objectMap, pos);
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
  objectmap_move(m->objectMap, o, delta);
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
      object_note_object_turned(o2, o, amt);
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

unsigned int map_tile_index(Map m, int x, int y, int z) {
  return tile_index(x, y, z, m->sz, mapvec_zero, m->sz);
}

unsigned char map_tile_at(Map m, int x, int y, int z) {
  return map_tile_at_index(m, map_tile_index(m, x, y, z));
}

unsigned char map_tile_at_index(Map m, int i) {
  return m->tilemap[i];
}

Tile map_tiledef_at_index(Map m, int i) {
  return TCOD_list_get(m->tileset, i);
}

Tile map_tiledef_at(Map m, int x, int y, int z) {
  return map_tiledef_at_index(m, map_tile_at(m, x, y, z));
}

Tile map_tiledef_at_position(Map m, mapVec pos) {
  return map_tiledef_at(m, pos.x, pos.y, pos.z);
}

void map_remake_fovmap(Map m) {
  // TCOD_map3_set_properties(TCOD_map3_t map, int x, int y, int z, bool is_transparent, bool floor_transparent, bool ceiling_transparent, bool is_walkable, bool ramp_down, bool ramp_up);
  for(int x = 0; x < m->sz.x; x++) {
    for(int y = 0; y < m->sz.y; y++) {
      for(int z = 0; z < m->sz.z; z++) {
        Tile tile = map_tiledef_at(m, x, y, z);
        bool wallTransparent = tile_wall_transparent(tile);
        bool floorTransparent = tile_floor_transparent(tile);
        bool ceilTransparent = tile_ceiling_transparent(tile);
        //uh, I guess map is going to need to know about passability eventually?
        //maybe a callback? or maybe just make clients make their own TCOD_map3_t.
        bool wallPassable  = wallTransparent;
        bool floorPassable = floorTransparent;
        bool ceilPassable  = ceilTransparent;
        
        TCOD_map3_set_properties(m->fovmap, x, y, z, wallTransparent, floorTransparent, ceilTransparent, wallPassable, floorPassable, ceilPassable);
      }
    }
  }
  m->fovmapDirty = false;
}

//refactor to support an interface that includes a Light and updates the lit flags rather than the viz flags.
void map_get_visible_tiles(Map m, perception *percept, Volume vol, mapVec bpos, mapVec bsz) {
  //fovmap is dirty if tile defs have changed
  if(m->fovmapDirty) {
    map_remake_fovmap(m);
  }
  //int times = 0;
  mapVec position = volume_position(vol);
  // printf("\n\n\n");
  
  mapVec size = m->sz, cur;
  unsigned int index, destIndex;
  perception newFlags;
  perception mapItem;
  
  int zstart = CLIP(bpos.z, 0, size.z);
  int zend = CLIP(bpos.z+bsz.z, 0, size.z);
  int ystart = CLIP(bpos.y, 0, size.y);
  int yend = CLIP(bpos.y+bsz.y, 0, size.y);
  int xstart = CLIP(bpos.x, 0, size.x);
  int xend = CLIP(bpos.x+bsz.x, 0, size.x);
  
  TCOD_map3_compute_fov(m->fovmap,
    position.x,position.y,position.z,
    mapvec_magnitude(bsz),
    true,
    FOV3_DIAMOND
  );
  //place the artificial boundaries imposed by the volume?
  //nah, too expensive to undo the artificial boundaries.
  //just copy vol/lit/los data
  for(int z = zstart; z < zend; z++) {
    for(int y = ystart; y < yend; y++) {
      for(int x = xstart; x < xend; x++) {
        index = map_tile_index(m, x, y, z);
        destIndex = tile_index(x, y, z, size, bpos, bsz);

        mapItem   = m->perceptmap[index];
        newFlags  = percept[destIndex];
        
        newFlags.toplit = mapItem.toplit;
        newFlags.surflit = mapItem.surflit;
        newFlags.underlit = mapItem.underlit;
                
        cur = (mapVec){x, y, z};
        //vol is 0 0 if unsure, 1 1 if known vol, 1 0 if edge of vol, 0 1 if known out-of-vol.
        if(newFlags.surfvol == 0x00) {
          //if it's within the volume
          if(volume_contains_point(vol, cur, 0.0)) {
            newFlags.surfvol = 0x03;
          } else {
            newFlags.surfvol = 0x01;
            //could potentially be faster to do volume in a seperate pass over
            //all tiles to add more obstacles to the fovmap.
            TCOD_map3_set_in_fov(m->fovmap, x,y,z, 0);
          }
          percept[destIndex] = newFlags;
        }
        if(newFlags.surflos == 0x00 || newFlags.edgelos == 0x00) {
          newFlags.surflos = newFlags.edgelos = (TCOD_map3_is_in_fov(m->fovmap,x,y,z) ? 0x03 : 0x01);
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
  //would it be faster to go from bpt<>bsz and use objectmap_get?
  for(int i = 0; i < map_object_count(m); i++) {
    o = map_get_object(m, i);
    pt = object_position(o);
    if(tile_index_in_bounds(pt.x, pt.y, pt.z, map_size(m), bpt, bsz)) {
      index = tile_index(pt.x, pt.y, pt.z, map_size(m), bpt, bsz);
      flags = visflags[index];
      if(map_item_visible(flags)) {
        TCOD_list_push(objs, o);
      }
    }
  }
}

mapVec map_size(Map m) {
  return m->sz;
}

//convenience methods --all mean "top or surface", do not include underside
bool map_item_lit(perception pcpt) {
  return (pcpt.toplit > 0x01) || (pcpt.surflit > 0x01);
}
bool map_item_in_volume(perception pcpt) {
  return (pcpt.surfvol > 0x01);
}
bool map_item_los(perception pcpt) {
  return (pcpt.edgelos > 0x01) || (pcpt.surflos > 0x01);
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
