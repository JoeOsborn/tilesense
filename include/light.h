#ifndef _LIGHT_H
#define _LIGHT_H

#include "geom.h"
#include "volume.h"

struct _light {
  char *id;
  Volume volume, oldVolume;
  
  //replace these with Tile-style implicit base Flagset?
  //add on an extra "flags" Flagset for user purposes?
  unsigned char attenuation;
  char intensity;
  
  void * owner; //ugly type hack, recursive blah
  void * map; //ugly type hack, recursive blah
  
  void *context;
};

typedef struct _light * Light;

#include "object.h"
#include "map.h"

Light light_new();
Light light_init(Light l, char *id, Volume volume, unsigned char attenuation, char intensity, void *context);
void light_free(Light l);

Frustum light_frustum(Light l);
unsigned char light_attenuation(Light l);
char light_intensity(Light l);
Map light_map(Light l);
void light_set_map(Light l, Map m);
Object light_owner(Light l);
void light_set_owner(Light l, Object o);
mapVec light_position(Light l);
void light_set_position(Light l, mapVec pos);
mapVec light_facing(Light l);
void light_set_facing(Light l, mapVec facing);
void *light_context(Light l);

void light_move(Light l, mapVec delta);
void light_turn(Light l, int amt);


#endif