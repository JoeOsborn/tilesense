#ifndef _OBJECT_MAP_H
#define _OBJECT_MAP_H

#include <libtcod.h>
#include "geom.h"

typedef struct {
  int index;
  TCOD_list_t objects;
} _omentry;

struct _object_map {
  mapVec size;
  int indexSize;
  int entryCount;
  int activeEntries;
  _omentry *entries;
};

typedef struct _object_map * ObjectMap;

#include "object.h"

ObjectMap objectmap_new();
ObjectMap objectmap_init(ObjectMap om, mapVec sz);
void objectmap_free(ObjectMap om);

TCOD_list_t objectmap_get(ObjectMap om, mapVec position);
void objectmap_insert(ObjectMap om, Object o);
void objectmap_remove(ObjectMap om, Object o);
void objectmap_move(ObjectMap om, Object o, mapVec delta);

#endif