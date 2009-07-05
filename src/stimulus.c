#include "stimulus.h"
#include <stdlib.h>
#include <string.h>

Stimulus stimulus_new() {
  return malloc(sizeof(struct _stimulus));
}

Stimulus stimulus_init(Stimulus s) {
  return s;
}

void stimulus_free(Stimulus s) {
  switch(s->type) {
    default:
    case StimGeneric:
      break;
    case StimTileLitChange:
    case StimTileVisChange:
      free(s->stim.tile_sight_change.newMap);
      break;
    case StimObjLitChange:
    case StimObjVisChange:
      free(s->stim.obj_sight_change.id);
      break;
    case StimObjMoved:
      free(s->stim.obj_moved.id);
      break;
  }
  free(s);
}

Stimulus stimulus_init_type(Stimulus s, stimtype type) {
  s->type = type;
  gettimeofday(&(s->tv), NULL);
  return s;
}

Stimulus stimulus_init_generic(Stimulus s, void *context) {
  s = stimulus_init_type(s, StimGeneric);
  s->stim.generic.context = context;
  return s;
}

Stimulus stimulus_init_tile_sight_change(Stimulus s, perception *newMap, mapVec position, mapVec size, stimtype type) {
  int stimSz = size.x*size.y*size.z*sizeof(perception);
  s->stim.tile_sight_change.newMap = malloc(stimSz);
  memcpy(s->stim.tile_sight_change.newMap, newMap, stimSz);
  s->stim.tile_sight_change.position = position;
  s->stim.tile_sight_change.size = size;
  return stimulus_init_type(s, type);
}

Stimulus stimulus_init_tile_vis_change(Stimulus s, perception *newTiles, mapVec position, mapVec size) {
  return stimulus_init_tile_sight_change(s, newTiles, position, size, StimTileVisChange);
}

Stimulus stimulus_init_tile_lit_change(Stimulus s, perception *newTiles, mapVec position, mapVec size) {
  return stimulus_init_tile_sight_change(s, newTiles, position, size, StimTileLitChange);
}

Stimulus stimulus_init_obj_sight_change(Stimulus s, Object obj, perception newFlags, stimtype type, void *ctx) {
  s->stim.obj_sight_change.position = object_position(obj);
  s->stim.obj_sight_change.facing = object_facing(obj);
  s->stim.obj_sight_change.id = strdup(object_id(obj));
  s->stim.obj_sight_change.newPercept = newFlags;
  s->stim.obj_sight_change.context = ctx;
  return stimulus_init_type(s, type);
}

Stimulus stimulus_init_obj_vis_change(Stimulus s, Object obj, perception newFlags, void *ctx) {
  return stimulus_init_obj_sight_change(s, obj, newFlags, StimObjVisChange, ctx);
}

Stimulus stimulus_init_obj_lit_change(Stimulus s, Object obj, perception newFlags, void *ctx) {
  return stimulus_init_obj_sight_change(s, obj, newFlags, StimObjLitChange, ctx);
}

Stimulus stimulus_init_obj_moved(Stimulus s, Object obj, mapVec dir, perception newFlags, void *ctx) {
  s = stimulus_init_obj_sight_change(s, obj, newFlags, StimObjMoved, ctx);
  s->stim.obj_moved.dir = dir;
  return s;
}

void *stimulus_obj_sight_change_get_context(Stimulus s) {
  return s->stim.obj_sight_change.context;
}
void * stimulus_obj_moved_get_context(Stimulus s) {
  return stimulus_obj_sight_change_get_context(s);
}


stimtype stimulus_type(Stimulus s) {
  return s->type;
}

struct timeval stimulus_time(Stimulus s) {
  return s->tv;
}

perception *stimulus_tile_sight_change_get_new_perceptmap(Stimulus s) {
  return s->stim.tile_sight_change.newMap;
}
mapVec stimulus_tile_sight_change_get_position(Stimulus s) {
  return s->stim.tile_sight_change.position;
}
mapVec stimulus_tile_sight_change_get_size(Stimulus s) {
  return s->stim.tile_sight_change.size;
}
mapVec stimulus_obj_sight_change_get_position(Stimulus s) {
  return s->stim.obj_sight_change.position;
}
mapVec stimulus_obj_sight_change_get_facing(Stimulus s) {
  return s->stim.obj_sight_change.facing;
}
char * stimulus_obj_sight_change_get_id(Stimulus s) {
  return s->stim.obj_sight_change.id;
}
perception stimulus_obj_sight_change_get_new_perception(Stimulus s) {
  return s->stim.obj_sight_change.newPercept;
}

mapVec stimulus_obj_moved_get_dir(Stimulus s) {
  return s->stim.obj_moved.dir;
}