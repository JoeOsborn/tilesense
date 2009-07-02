#ifndef _SENSOR_H
#define _SENSOR_H

#include "geom.h"
#include <libtcod.h>
#include <list.h>
#include "volume.h"

//add on an extra "flags" Flagset for user purposes?

struct _sensor {
  char *id;
  Volume volume;  

  unsigned char *vistiles;
  unsigned char *oldVistiles;
  
  mapVec borig, bsz;

  TCOD_list_t visObjects;
  TCOD_list_t oldVisObjects;
  
  TCOD_list_t stimuli; //a list of Stimulus
  
  void * owner; //ugly hack for recursive bleh, should be Object
  void * map; //ugly hack for recursive bleh, should be Map
  
  void *context;
};
typedef struct _sensor * Sensor;

#include "map.h" //ugly hack for recursive bleh
#include "object.h"
#include "stimulus.h"

Sensor sensor_new();
Sensor sensor_init(Sensor s, char *id, Volume volume, void *context);
void sensor_free(Sensor s);
Volume sensor_volume(Sensor s);
void *sensor_context(Sensor s);
mapVec sensor_position(Sensor s);
void sensor_set_position(Sensor s, mapVec p);
mapVec sensor_facing(Sensor s);
void sensor_set_facing(Sensor s, mapVec f);
void sensor_set_owner(Sensor s, Object o);
void sensor_set_map(Sensor s, Map m);

void sensor_move(Sensor s, mapVec delta);
void sensor_turn(Sensor s, int amt);

void sensor_sense(Sensor s);
TCOD_list_t sensor_consume_stimuli(Sensor s); //the user _must_ free the returned list
void sensor_push_stimulus(Sensor s, Stimulus stim); //this dispatches and updates the vistiles/visobjects/whatever

unsigned char *sensor_get_visible_tiles(Sensor s);
TCOD_list_t sensor_get_visible_objects(Sensor s);

void sensor_swept_bounds(Sensor s, mapVec *borig, mapVec *bsz);

#endif