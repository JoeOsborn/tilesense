#ifndef _OBJECT_H
#define _OBJECT_H

#include <libtcod.h>
#include "geom.h"

//add on an extra "flags" Flagset for user purposes?

struct _object {
  char *id;
  mapVec position;
  mapVec facing;
  TCOD_list_t sensors;
  TCOD_list_t lights;
    
  void * map; //ugly hack to avoid recursive struct def'n, should be Map
  
  void *context;
};
typedef struct _object * Object;

#include "map.h" //ugly hack to avoid recursive bleh...
#include "sensor.h"
#include "light.h"

Object object_new();
Object object_init(Object o, char *id, mapVec pos, mapVec face, Map m, void *context);
void object_free(Object o);

mapVec object_position(Object o);
mapVec object_facing(Object o);
char *object_id(Object o);
Map object_map(Object o);

void *object_context(Object o);
void object_set_context(Object o, void *ctx);

void object_sense(Object o);

int object_sensor_count(Object o);
TCOD_list_t object_sensors(Object o);
Sensor object_get_sensor(Object o, int i);
Sensor object_get_sensor_named(Object o, char *n);
void object_add_sensor(Object o, Sensor s);
void object_remove_sensor(Object o, Sensor s);

int object_light_count(Object o);
Light object_get_light(Object o, int i);
void object_add_light(Object o, Light l);
void object_remove_light(Object o, Light l);

void object_move(Object o, mapVec delta);
void object_turn(Object o, int amt);

void object_note_object_moved(Object o, Object o2, mapVec delta);
void object_note_object_turned(Object o, Object o2, int amt);

void object_set_map(Object o, Map m);

#endif