#ifndef _MAP_H
#define _MAP_H

#include <libtcod.h>

#include "geom.h"
#include "exit.h"
#include "tile.h"
#include "volume.h"

struct _map {
  char *id;
  mapVec sz;
  unsigned char *tilemap;
  perception *perceptmap;
  TCOD_list_t tileset;
  char ambientLight;
  TCOD_list_t exits;
  TCOD_list_t objects;
  void *context;
};
typedef struct _map * Map;

#include "object.h" //ugly hack to avoid recursive type bleh

Map map_new();
Map map_init(
  Map m, 
  char *id, 
  mapVec sz, 
  unsigned char *tilemap,
  unsigned char ambientLight,
  void *ctx,
  void *baseTileCtx
);
void map_free(Map m);
mapVec map_size(Map m);
void map_add_exit(Map m, Exit ex);
void map_remove_exit(Map m, Exit ex);
void map_get_region(Map m, perception *buf, mapVec start, mapVec end, mapVec bpos, mapVec bsz);
void map_add_tile(Map m, Tile t);

void *map_context(Map m);
char *map_id(Map m);

void map_add_object(Map m, Object o);
void map_remove_object(Map m, Object o);
void map_remove_object_at(Map m, int i);
void map_remove_object_named(Map m, char *id);
Object map_get_object(Map m, int i);
Object map_get_object_named(Map m, char *id);
int map_object_count(Map m);

int map_tile_index(Map m, int x, int y, int z);
unsigned char map_tile_at_index(Map m, int i);
unsigned char map_tile_at(Map m, int x, int y, int z);
Tile map_get_tile(Map m, int tileIndex);

void map_move_object(Map m, char *id, mapVec delta);
void map_turn_object(Map m, char *id, int amt);

//as for the buf -- it's also used as a scratchpad.  It's laid out like this:
  //lit:4 vol:2 los:2
  //vol and los are 0 0 if unsure, 1 1 if known viz, 1 0 if edge of viz, 0 1 if known inviz.
  //the vol/los flags on the tilemap itself will be ignored, though the lighting flags will be used.
//buf should be no smaller than the given bounds.  buf will contain the flags, map should be queried for tile_id
void map_get_visible_tiles(Map m, perception *percept, Volume vol, mapVec bpos, mapVec bsz);
void map_get_visible_objects(Map m, TCOD_list_t objs, perception *percept, mapVec bpt, mapVec bsz);

//convenience methods --all mean "top or surface", do not include underside
bool map_item_lit(perception pcpt);
bool map_item_in_volume(perception pcpt);
bool map_item_los(perception pcpt);
//lit and in volume and in los
bool map_item_visible(perception pcpt);

//is this API ok?  won't there be a fair bit of redundant work for objects with multiple lights?
void map_note_light_added(Map m, unsigned char attenuation, int intensity, Volume vol);
void map_note_light_removed(Map m, unsigned char attenuation, int intensity, Volume vol);
void map_note_light_volume_changed(Map m, unsigned char attenuation, int intensity, Volume old, Volume new);

#endif