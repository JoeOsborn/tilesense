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

unsigned int map_tile_index(Map m, int x, int y, int z) {
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

// typedef enum {
//   MapTraceNone=0,
//   MapTraceLOS=1,
//   MapTraceLight=2,
//   MapTraceBoth=3
// } MapTraceMode;

perception map_trace(Map m, 
  perception *percept, 
  TCOD_bresenham3_data_t *bd, 
  mapVec bpos, mapVec bsz, 
  int pX, int pY, int pZ) 
{ 
  //doesn't work yet, got to figure out why.  build a test room that's smaller and get it working!
  
  int x,y,z;
  bool done = TCOD_line3_step_mt(&x, &y, &z, bd) || ((x == bd->destx) && (y== bd->desty) && (z==bd->destz));
  if(done) {
    x = bd->destx;
    y = bd->desty;
    z = bd->destz;
  }
  //the "out" tile is the returned tile.
  //the "in" tile is the current tile specified by x,y,z.
  Direction outDir = direction_between(pX,pY,pZ,x,y,z,pZ);
  
  unsigned int outIdx = tile_index(pX,pY,pZ,map_size(m),bpos,bsz);
  unsigned int inIdx  = tile_index( x, y, z,map_size(m),bpos,bsz);
  
  Tile outTile = map_get_tile(m, m->tilemap[map_tile_index(m, pX, pY, pZ)]);
  Tile inTile  = map_get_tile(m, m->tilemap[map_tile_index(m, x, y, z)]);
  
  perception outp = percept[outIdx];
  perception inp  = percept[inIdx];

  //is inp unknown?
  if(inp.surflos==0x00) {
    //yes: is in == dest?
    if(done) {
      //yes: in.surflos = 0x03
      //destination is always in surface los.
      inp.surflos=0x03;
      //update the index.
      percept[inIdx] = inp;
    } else {
      //no: in = recurse (map will be updated during recursion)
      inp = map_trace(m, percept, bd, bpos, bsz, x, y, z);
    }
  }
  unsigned char inBlock;
  if(inp.toplos==0x00) {
    //does the ceiling accept light?
    inBlock = tile_opacity_direction(inTile, DirZMinusIn);
    if((inp.surflos == 0x01) || inBlock > 10) {
      inp.toplos = 0x01;
    } else if(inBlock < 5) {
      inp.toplos = MAX(inp.toplos, inp.surflos);
    } else if(inBlock < 10) {
      inp.toplos = MAX(inp.surflos, 0x02);
    }
  }
  if(inp.underlos==0x00) {
    //does the floor accept light?
    inBlock = tile_opacity_direction(inTile, DirZPlusIn);
    if((inp.surflos == 0x01) || inBlock > 10) {
      inp.underlos = 0x01;
    } else if(inBlock < 5) {
      inp.underlos = MAX(inp.underlos, inp.surflos);
    } else if(inBlock < 10) {
      inp.underlos = MAX(inp.surflos, 0x02);
    }
  }
  percept[inIdx] = inp;
  
  //is inp out of los?
  if(inp.surflos == 0x01) { 
    //yes: it doesn't matter whether in's top or bottom are -- out certainly won't be in los.
    outp.toplos = outp.surflos = outp.underlos = 0x01;
    //update the index.
    percept[outIdx] = outp;
    return outp;
  }
  
  //inp is in los.  is outp?
  unsigned char outBlock;
  //are outp's top or bottom in los?
  //is the light leaving the bottom?
  if(z < pZ) {
    //yes: the underside will be in los if in.top is AND if in's ceiling accepts light
    inBlock = tile_opacity_direction(inTile, DirZMinusIn);
    if(inBlock > 10) { outp.underlos = 0x01; }
    else if(inBlock > 5) { outp.underlos = 0x02; }
    else { outp.underlos = inp.toplos; }
  //is the light leaving the top?
  } else if(z > pZ) {
    //yes: the top itself will be in los if in.under is AND if in's floor accepts light
    inBlock = tile_opacity_direction(inTile, DirZPlusIn);
    if(inBlock > 10) { outp.toplos = 0x01; }
    else if(inBlock > 5) { outp.toplos = 0x02; }
    else { outp.toplos = inp.underlos; }
  }
  
  //is outp's surface in los?
  //it only is if any of:
    //outp's top is in los and outp emits light out the top
    //outp's bottom is in los and outp emits light out the bottom
    //inp's surface is in los and outp emits light towards inp
  outp.surflos = 0x01;
  if(outp.toplos > 0x01 && (z != pZ)) {
    outBlock = tile_opacity_direction(outTile, DirZPlusOut);
    if(outBlock < 5) {
      outp.surflos = MAX(outp.surflos, outp.toplos);
    } else if(outBlock < 10) {
      outp.surflos = MAX(outp.surflos, 0x02);
    }
  }
  if(outp.underlos > 0x01 && (z != pZ)) {
    outBlock = tile_opacity_direction(outTile, DirZMinusOut);
    if(outBlock < 5) {
      outp.surflos = MAX(outp.surflos, outp.underlos);
    } else if(outBlock < 10) {
      outp.surflos = MAX(outp.surflos, 0x02);
    }
  }
  if(inp.surflos > 0x01 && (z == pZ)) {
    outBlock = tile_opacity_direction(outTile, outDir);
    if(outBlock < 5) {
      outp.surflos = MAX(outp.surflos, inp.surflos);
    } else if(outBlock < 10) {
      outp.surflos = MAX(outp.surflos, 0x02);
    }
  }
  //outp's top and bottom could be lit if outp's surflos is > 1 and out emits light those ways
  // if(outp.underlos == 0x00) {
  //   outp.underlos = 0x01;
  //   if(outp.surflos > 0x01) {
  //     //light leaves the floor
  //     outBlock = tile_opacity_direction(outTile, DirZMinusOut);
  //     if(outBlock < 5) {
  //       outp.underlos = MAX(outp.underlos, outp.surflos);
  //     } else if(outBlock < 10) {
  //       outp.underlos = MAX(outp.underlos, 0x02);
  //     }
  //   }
  // }
  // if(outp.toplos == 0x00) {
  //   outp.toplos = 0x01;
  //   if(outp.surflos > 0x01) {
  //     outBlock = tile_opacity_direction(outTile, DirZPlusOut);
  //     if(outBlock < 5) {
  //       outp.toplos = MAX(outp.toplos, outp.surflos);
  //     } else if(outBlock < 10) {
  //       outp.toplos = MAX(outp.toplos, 0x02);
  //     }
  //   }
  // }
  percept[outIdx] = outp;
  return outp; 
}
//refactor to support an interface that includes a Light and updates the lit flags rather than the viz flags.
void map_get_visible_tiles(Map m, perception *percept, Volume vol, mapVec bpos, mapVec bsz) {
  //int times = 0;
  mapVec position = volume_position(vol);
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
        if(newFlags.surfvol == 0x00 || newFlags.topvol == 0x00 || newFlags.undervol == 0x00) { //not right anymore, three vol flags?
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
        if(newFlags.surflos == 0x00 || newFlags.toplos == 0x00 || newFlags.underlos == 0x00) {
          TCOD_bresenham3_data_t bd;
          TCOD_line3_init_mt(cur.x, cur.y, cur.z, position.x, position.y, position.z, &bd);
          //this is a recursive fn that is also destructive to flags.  keep that in mind!
          //trace from this tile to the sensor
          newFlags = map_trace(m, percept, &bd, bpos, bsz, x, y, z);
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
  return (pcpt.toplit > 0x01) || (pcpt.surflit > 0x01);
}
bool map_item_in_volume(perception pcpt) {
  return (pcpt.topvol > 0x01) || (pcpt.surfvol > 0x01);
}
bool map_item_los(perception pcpt) {
  return (pcpt.toplos > 0x01) || (pcpt.surflos > 0x01);
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
