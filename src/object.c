#include "object.h"
#include <stdlib.h>
#include <string.h>
#include "stimulus.h"
#include "sensor.h"

Object object_new() {
  return malloc(sizeof(struct _object));
}

Object object_init(Object o, char *id, mapVec pos, mapVec face, Map m) {
  o->id = malloc(strlen(id)*sizeof(char));
  strcpy(o->id, id);
  o->position = pos;
  o->facing = face;
  o->map = m;
  o->sensors = TCOD_list_new();
  o->lights = TCOD_list_new();
  return o;
}

void object_free(Object o) {
  free(o->id);
  for(int i = 0; i < object_sensor_count(o); i++) {
    object_remove_sensor(o, object_get_sensor(o,i));
  }
  TCOD_list_delete(o->sensors);
  for(int i = 0; i < object_light_count(o); i++) {
    object_remove_light(o, object_get_light(o,i));
  }
  TCOD_list_delete(o->lights);
  free(o);
}

mapVec object_position(Object o) {
  return o->position;
}
mapVec object_facing(Object o) {
  return o->facing;
}
char *object_id(Object o) {
  return o->id;
}
Map object_map(Object o) {
  return o->map;
}

int object_sensor_count(Object o) {
  return TCOD_list_size(o->sensors);
}
Sensor object_get_sensor(Object o, int i) {
  return TCOD_list_get(o->sensors, i);
}
void object_add_sensor(Object o, Sensor s) {
  TCOD_list_push(o->sensors, s);
  sensor_set_owner(s, o);
  sensor_set_map(s, o->map);
  sensor_set_position(s, mapvec_add(o->position, sensor_position(s)));
  float senseRot = mapvec_facing_to_radians(sensor_facing(s), 1);
  sensor_set_facing(s, mapvec_normalize(mapvec_rotate(o->facing, mapvec_zero, senseRot, 1)));
}
void object_remove_sensor(Object o, Sensor s) {
  TCOD_list_remove(o->sensors, s);
  sensor_free(s);
}

int object_light_count(Object o) {
  return TCOD_list_size(o->lights);
}
Light object_get_light(Object o, int i) {
  return TCOD_list_get(o->lights, i);
}
void object_add_light(Object o, Light l) {
  TCOD_list_push(o->lights, l);
  light_set_owner(l, o);
  light_set_position(l, mapvec_add(o->position, light_position(l)));
  float lightRot = mapvec_facing_to_radians(light_facing(l), 1);
  light_set_facing(l, mapvec_normalize(mapvec_rotate(o->facing, mapvec_zero, lightRot, 1)));
  light_set_map(l, o->map);
  if(o->map) {
    object_sense(o);
  }
}
void object_remove_light(Object o, Light l) {
  TCOD_list_remove(o->lights, l);
  if(o->map) {
    object_sense(o);
  }
  light_free(l);
}

void object_sense(Object o) {
  for(int i = 0; i < object_sensor_count(o); i++) {
    sensor_sense(object_get_sensor(o, i));
  }
}

void object_move(Object o, mapVec delta) {
  o->position = (mapVec){o->position.x+delta.x, o->position.y+delta.y, o->position.z+delta.z};
  for(int i = 0; i < object_sensor_count(o); i++) {
    sensor_move(object_get_sensor(o, i), delta);
  }
  for(int i = 0; i < object_light_count(o); i++) {
    Light l = object_get_light(o, i);
    light_move(l, delta);
  }
  object_sense(o);
}
void object_turn(Object o, int amt) {
  o->facing = mapvec_turn_facing(o->facing, amt);
  for(int i = 0; i < object_sensor_count(o); i++) {
    sensor_turn(object_get_sensor(o, i), amt);
  }
  for(int i = 0; i < object_light_count(o); i++) {
    Light l = object_get_light(o, i);
    light_turn(l, amt);
  }
  object_sense(o);
}

void object_note_object_moved(Object o, Object o2, mapVec delta) {
  Map m = object_map(o);
  mapVec newPos = object_position(o2);
  mapVec oldPos = (mapVec){newPos.x-delta.x, newPos.y-delta.y, newPos.z-delta.z};
  int oldIndex, newIndex;
  unsigned short oldMapItem, newMapItem;
  unsigned char newFlags, oldVis, newVis;
  unsigned char *vistiles;
  
  mapVec bpt, bsz;  
  Stimulus movestim;
  Sensor s;
  for(int i = 0; i < object_sensor_count(o); i++) {
    s = object_get_sensor(o,i);
    sensor_swept_bounds(s, &bpt, &bsz);
    vistiles = sensor_get_visible_tiles(s);
    oldIndex = tile_index(oldPos.x, oldPos.y, oldPos.z, map_size(m), bpt, bsz);
    newIndex = tile_index(newPos.x, newPos.y, newPos.z, map_size(m), bpt, bsz);
    oldMapItem = vistiles[oldIndex];
    newMapItem = vistiles[newIndex];
    oldVis = map_item_visible(oldMapItem);
    newVis = map_item_visible(newMapItem);
    if(oldVis || newVis) {
      newFlags = map_item_flags(newMapItem);
      movestim = stimulus_init_obj_moved(stimulus_new(), o2, delta, newFlags);
      sensor_push_stimulus(s, movestim);
    }
  }
}