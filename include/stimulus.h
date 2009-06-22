#ifndef _STIMULUS_H
#define _STIMULUS_H

#include <libtcod.h>
#include <list.h>
#include <sys/time.h>

#include "geom.h"

typedef enum {
  StimGeneric = 0,
  StimTileLitChange,
  StimTileVisChange,
  StimObjLitChange,
  StimObjVisChange,
  StimObjMoved
} stimtype;

struct _stimulus {
  union {
    struct _stim_generic {
      
      } generic;
    struct _stim_tile_sight_change { //tilelit, tilevis
      unsigned char *newTiles;
      mapVec position;
      mapVec size;
      } tile_sight_change;
    struct _stim_tile_obj_sight_change { //objlit, objvis
      mapVec position;
      mapVec facing;
      char *id;
      unsigned char newFlags; 
      } obj_sight_change; 
    struct _stim_tile_obj_moved { //objmoved
      mapVec position;
      mapVec facing;
      char *id;
      unsigned char newFlags;
      mapVec dir;
      } obj_moved; 
  } stim;
  stimtype type;
  struct timeval tv;
};
typedef struct _stimulus * Stimulus;

#include "object.h" //ugly hack, avoid recursive bleh

Stimulus stimulus_new();
Stimulus stimulus_init(Stimulus s);
Stimulus stimulus_init_generic(Stimulus s);
Stimulus stimulus_init_tile_vis_change(Stimulus s, unsigned char *newTiles, mapVec position, mapVec size);
Stimulus stimulus_init_tile_lit_change(Stimulus s, unsigned char *newTiles, mapVec position, mapVec size);
Stimulus stimulus_init_obj_vis_change(Stimulus s, Object obj, unsigned char newFlags);
Stimulus stimulus_init_obj_lit_change(Stimulus s, Object obj, unsigned char newFlags);
Stimulus stimulus_init_obj_moved(Stimulus s, Object obj, mapVec dir, unsigned char newFlags);
void stimulus_free(Stimulus s);

stimtype stimulus_type(Stimulus s);
struct timeval stimulus_time(Stimulus s);

unsigned char *stimulus_tile_sight_change_get_new_tiles(Stimulus s);
mapVec stimulus_tile_sight_change_get_position(Stimulus s);
mapVec stimulus_tile_sight_change_get_size(Stimulus s);

mapVec stimulus_obj_sight_change_get_position(Stimulus s);
mapVec stimulus_obj_sight_change_get_facing(Stimulus s);
char * stimulus_obj_sight_change_get_id(Stimulus s);
unsigned char stimulus_obj_sight_change_get_new_flags(Stimulus s);

mapVec stimulus_obj_moved_get_dir(Stimulus s);
#endif