#ifndef _STIMULUS_H
#define _STIMULUS_H

#include <libtcod.h>
#include <sys/time.h>

#include "geom.h"

typedef enum {
  StimGeneric = 0,
  StimTileLitChange,
  StimTileVisChange,
  StimObjLitChange,
  StimObjVisChange,
  StimObjMoved,
  StimUser1,
  StimUser2,
  StimUser3,
  StimUser4,
  StimUser5,
  StimUser6,
  StimUser7,
  StimUser8,
  StimUser9,
  StimUser10
} stimtype;

struct _stimulus {
  union {
    struct _stim_generic {
      void *context;
      } generic;
    struct _stim_tile_sight_change { //tilelit, tilevis
      perception *newMap;
      mapVec position;
      mapVec size;
      } tile_sight_change;
    struct _stim_tile_obj_sight_change { //objlit, objvis
      mapVec position;
      mapVec facing;
      char *id;
      perception newPercept; 
      void *context;
      } obj_sight_change; 
    struct _stim_tile_obj_moved { //objmoved
      mapVec position;
      mapVec facing;
      char *id;
      perception newPercept; 
      void *context;
      mapVec dir;
      } obj_moved; 
  } stim;
  stimtype type;
  uint32 stimTime;

  void *stimulusContext;
};
typedef struct _stimulus * Stimulus;

#include "object.h" //ugly hack, avoid recursive bleh

Stimulus stimulus_new();
Stimulus stimulus_init(Stimulus s);
Stimulus stimulus_init_generic(Stimulus s, void *context);
Stimulus stimulus_init_user(Stimulus s, stimtype type, void *context);
Stimulus stimulus_init_tile_vis_change(Stimulus s, perception *newMap, mapVec position, mapVec size);
Stimulus stimulus_init_tile_lit_change(Stimulus s, perception *newMap, mapVec position, mapVec size);
Stimulus stimulus_init_obj_vis_change(Stimulus s, Object obj, perception newFlags, void *context);
Stimulus stimulus_init_obj_lit_change(Stimulus s, Object obj, perception newFlags, void *context);
Stimulus stimulus_init_obj_moved(Stimulus s, Object obj, mapVec dir, perception newFlags, void *context);
void stimulus_free(Stimulus s);

stimtype stimulus_type(Stimulus s);
uint32 stimulus_time(Stimulus s);

void *stimulus_generic_get_context(Stimulus s);

perception *stimulus_tile_sight_change_get_new_perceptmap(Stimulus s);
mapVec stimulus_tile_sight_change_get_position(Stimulus s);
mapVec stimulus_tile_sight_change_get_size(Stimulus s);

void * stimulus_obj_sight_change_get_context(Stimulus s);
mapVec stimulus_obj_sight_change_get_position(Stimulus s);
mapVec stimulus_obj_sight_change_get_facing(Stimulus s);
char * stimulus_obj_sight_change_get_id(Stimulus s);
perception stimulus_obj_sight_change_get_new_perception(Stimulus s);

mapVec stimulus_obj_moved_get_dir(Stimulus s);
void * stimulus_obj_moved_get_context(Stimulus s);

#endif