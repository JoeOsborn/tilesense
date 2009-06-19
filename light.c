#include "light.h"
#include "stdlib.h"
#include "string.h"

Light light_new() {
  return malloc(sizeof(struct _light));
}
Light light_init(Light l, char *id, Volume volume, unsigned char attenuation, char intensity) {
  l->id = malloc(strlen(id));
  strcpy(l->id, id);
  l->attenuation = attenuation;
  l->intensity = intensity;
  l->volume = volume;
  l->oldVolume = NULL;
  return l;
}
void light_free(Light l) {
  if(l->map) {
    map_note_light_removed(l->map, l->attenuation, l->intensity, l->volume);
  }
  volume_free(l->volume);
  if(l->oldVolume) {
    volume_free(l->oldVolume);
  }
  free(l->id);
  free(l);
}
Volume light_volume(Light l) {
  return l->volume;
}
unsigned char light_attenuation(Light l) {
  return l->attenuation;
}
char light_intensity(Light l) {
  return l->intensity;
}
Map light_map(Light l) {
  return l->map;
}
void light_set_map(Light l, Map m) {
  l->map = m;
  map_note_light_added(l->map, l->attenuation, l->intensity, l->volume);
}
Object light_owner(Light l) {
  return l->owner;
}
void light_set_owner(Light l, Object o) {
  l->owner = o;
}
mapVec light_position(Light l) {
  return volume_position(l->volume);
}
void light_set_position(Light l, mapVec pos) {
  if(l->oldVolume) {
    free(l->oldVolume);
  }
  l->oldVolume = volume_copy(l->volume);
  volume_set_position(l->volume, pos);
  if(l->map) {
    map_note_light_volume_changed(l->map, l->attenuation, l->intensity, l->oldVolume, l->volume);
  }
}
mapVec light_facing(Light l) {
  return volume_facing(l->volume);
}
void light_set_facing(Light l, mapVec facing) {
  if(l->oldVolume) {
    free(l->oldVolume);
  }
  l->oldVolume = volume_copy(l->volume);
  volume_set_facing(l->volume, facing);
  if(l->map) {
    map_note_light_volume_changed(l->map, l->attenuation, l->intensity, l->oldVolume, l->volume);
  }
}
void light_move(Light l, mapVec delta) {
  light_set_position(l, mapvec_add(light_position(l), delta));
}
void light_turn(Light l, int amt) {
  light_set_facing(l, mapvec_turn_facing(light_facing(l), amt));
}