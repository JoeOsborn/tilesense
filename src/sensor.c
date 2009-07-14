#include "sensor.h"
#include <stdlib.h>
#include <string.h>
#include "tslist.h"

Sensor sensor_new() {
  return malloc(sizeof(struct _sensor));
}
Sensor sensor_init(Sensor s, char *id, Volume volume, void *context) {
  s->id = strdup(id);
  s->volume = volume;
  volume_swept_bounds(s->volume, &(s->borig), &(s->bsz));
  
  s->vistiles = calloc(s->bsz.x*s->bsz.y*s->bsz.z, sizeof(perception));
      
  s->visObjects = TCOD_list_new();
  s->oldVisObjects = TCOD_list_new();

  s->stimuli = TCOD_list_new();
  
  s->context = context;
  
  return s;
}
void sensor_free(Sensor s) {
  free(s->id);
  volume_free(s->volume);
  if(s->vistiles) {
    free(s->vistiles);
  }
  TCOD_list_delete(s->visObjects);
  TCOD_list_delete(s->oldVisObjects);

  TS_LIST_CLEAR_AND_DELETE(s->stimuli, stimulus);
  free(s);
}
Volume sensor_volume(Sensor s) {
  return s->volume;
}
mapVec sensor_position(Sensor s) {
  return volume_position(s->volume);
}
void sensor_set_position(Sensor s, mapVec p) {
  volume_set_position(s->volume, p);
  volume_swept_bounds(s->volume, &(s->borig), NULL);
}
mapVec sensor_facing(Sensor s) {
  return volume_facing(s->volume);
}
void sensor_set_facing(Sensor s, mapVec f) {
  volume_set_facing(s->volume, f);
}
void sensor_set_owner(Sensor s, Object o) {
  s->owner = o;
}
void sensor_set_map(Sensor s, Map m) {
  s->map = m;
}
void *sensor_context(Sensor s) {
  return s->context;
}

void sensor_move(Sensor s, mapVec delta) {
  sensor_set_position(s, mapvec_add(sensor_position(s), delta));
}
void sensor_turn(Sensor s, int amt) {
  sensor_set_facing(s, mapvec_turn_facing(sensor_facing(s), amt));
}

void sensor_sense(Sensor s) {
  Map m = s->map;
  
  mapVec pos=s->borig, sz=s->bsz;
  memset(s->vistiles, 0, sz.x*sz.y*sz.z*sizeof(perception));
  map_get_visible_tiles(m, s->vistiles, s->volume, pos, sz);
  Stimulus vistiles = stimulus_init_tile_vis_change(stimulus_new(), s->vistiles, pos, sz);
  // TCOD_console_print_left(NULL, 0, 20, "p {%f, %f, %f}, s {%f, %f, %f}", pos.x, pos.y, pos.z, sz.x, sz.y, sz.z);
  
  TCOD_list_push(s->stimuli, vistiles);
  
  TCOD_list_t oldObjList = s->visObjects; //one ago
  s->visObjects = s->oldVisObjects; //two ago
  TCOD_list_clear(s->visObjects);
  s->oldVisObjects = oldObjList;
  
  map_get_visible_objects(m, s->visObjects, s->vistiles, pos, sz);
  //remove the old objects
  Object o;
  mapVec pt;
  int index;
  Stimulus visobj;
  for(int i = 0; i < TCOD_list_size(s->oldVisObjects); i++) {
    o = TCOD_list_get(s->oldVisObjects, i);
    if(!TCOD_list_contains(s->visObjects, o)) { //not visible anymore
      //this isn't quite right, really, using percept_none here
      visobj = stimulus_init_obj_vis_change(stimulus_new(), o, percept_none, object_context(o));
      TCOD_list_push(s->stimuli, visobj);
    }
  }
  //add the new objects
  for(int i = 0; i < TCOD_list_size(s->visObjects); i++) {
    o = TCOD_list_get(s->visObjects, i);
    if(!TCOD_list_contains(s->oldVisObjects, o)) { //not visible before
      pt = object_position(o);
      index = tile_index(pt.x, pt.y, pt.z, map_size(m), pos, sz);
      visobj = stimulus_init_obj_vis_change(stimulus_new(), o, s->vistiles[index], object_context(o));
      TCOD_list_push(s->stimuli, visobj);
    }
  }
}

TCOD_list_t sensor_consume_stimuli(Sensor s) {
  TCOD_list_t ret = s->stimuli;
  s->stimuli = TCOD_list_new();
  return ret;
}
Object sensor_visobjs_member(TCOD_list_t l, Object o) {
  for(int i = 0; i < TCOD_list_size(l); i++) {
    Object o2 = TCOD_list_get(l, i);
    if(strcmp(object_id(o), object_id(o2)) == 0) {
      return o2;
    }
  }
  return NULL;
}
int sensor_visobjs_contains(TCOD_list_t l, Object o) {
  return sensor_visobjs_member(l, o) != NULL;
}
void sensor_visobjs_push(TCOD_list_t l, Object o) {
  //push if not present
  Object o2 = sensor_visobjs_member(l, o);
  if(!o2) {
    TCOD_list_push(l, o);
  }
}
void sensor_visobjs_remove(TCOD_list_t l, Object o) {
  //remove if present
  Object o2 = sensor_visobjs_member(l, o);
  if(o2) {
    TCOD_list_remove(l, o2);
  }
}
void sensor_push_stimulus(Sensor s, Stimulus stim) {
  mapVec pt, sz;
  perception *newVis;
  perception *snewVis = s->vistiles;
  perception newPerception;
  bool litAndVisible;
  Object o;
  Map m = s->map;
  mapVec borig=s->borig, bsz=s->bsz;
  switch(stimulus_type(stim)) {
    case StimTileLitChange:
    case StimTileVisChange:
      newVis = stimulus_tile_sight_change_get_new_perceptmap(stim);
      pt = stimulus_tile_sight_change_get_position(stim);
      sz = stimulus_tile_sight_change_get_size(stim);
      for(int z = pt.z; z < pt.z+sz.z; z++) {
        for(int y = pt.y; y < pt.y+sz.y; y++) {
          for(int x = pt.x; x < pt.x+sz.x; x++) {
            int stimIndex = tile_index(x, y, z, map_size(m), pt, sz);
            int visIndex = tile_index(x, y, z, map_size(m), borig, bsz);
            snewVis[visIndex] = newVis[stimIndex];
          }
        }
      }
      break;
    case StimObjLitChange:
    case StimObjVisChange:
    case StimObjMoved:
      //is the object in newVis? if so, put it into oldVis; if not, make sure it isn't in oldVis.
      //do this search by _ID_, not by identity
      //are the new flags good? if so, be sure the object is in vis. otherwise, be sure it's not in vis.
      if(sensor_visobjs_contains(s->visObjects, o)) {
        sensor_visobjs_push(s->oldVisObjects, o);
      } else {
        sensor_visobjs_remove(s->oldVisObjects, o);
      }
      newPerception = stimulus_obj_sight_change_get_new_perception(stim);
      litAndVisible = map_item_visible(newPerception);
      
      if(litAndVisible) {
        if(!sensor_visobjs_contains(s->visObjects, o)) {
          sensor_visobjs_push(s->visObjects, o);
        }
      } else {
        if(sensor_visobjs_contains(s->visObjects, o)) {
          sensor_visobjs_remove(s->visObjects, o);
        }
      }
      break;
    case StimGeneric:
    default:
      break;
  }
  TCOD_list_push(s->stimuli, stim);
}

perception *sensor_get_perceptmap(Sensor s) {
  return s->vistiles;
}

TCOD_list_t sensor_get_visible_objects(Sensor s) {
  return s->visObjects;
}

void sensor_swept_bounds(Sensor s, mapVec *borig, mapVec *bsz) {
  if(borig) {
    *borig = s->borig;
  }
  if(bsz) {
    *bsz = s->bsz;
  }
}