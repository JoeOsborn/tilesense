#ifndef _MAP_H
#define _MAP_H

#include <libtcod.h>
#include <list.h>

#include "geom.h"
#include "exit.h"
#include "tile.h"
#include "volume.h"

struct _map {
  char *id;
  mapVec sz;
  unsigned short *tilemap;
  TileSet tileSet;
  char ambientLight;
  TCOD_list_t exits;
  TCOD_list_t objects;
  // Exit *exits;
  // int exitCount, exitMax;
  };
typedef struct _map * Map;

#include "object.h" //ugly hack to avoid recursive type bleh

Map map_new();
Map map_init(
  Map m, 
  char *room, 
  mapVec sz, 
  unsigned short *tilemap,
  char ambientLight
);
void map_free(Map m);
mapVec map_size(Map m);
void map_add_exit(Map m, Exit ex);
void map_remove_exit(Map m, Exit ex);
void map_get_region(Map m, unsigned short *buf, mapVec start, mapVec end);
void map_add_tile(Map m, Tile t);

void map_add_object(Map m, Object o);
void map_remove_object(Map m, Object o);
void map_remove_object_at(Map m, int i);
void map_remove_object_named(Map m, char *id);
Object map_get_object(Map m, int i);
Object map_get_object_named(Map m, char *id);
int map_object_count(Map m);

int map_tile_index(Map m, int x, int y, int z);
unsigned char map_tile_at_index(Map m, int i);

void map_move_object(Map m, char *id, mapVec delta);
void map_turn_object(Map m, char *id, int amt);

//these fov ints are in terms of 8ths of a circle. 0 means "directly ahead, no spread", 8 means "everything".
//as for the buf -- it's also used as a scratchpad.  It's laid out like this:
  //tileID:8 lit:2 flgs:4 vis:2
  //vis is 0 0 if unsure, 1 1 if known viz, 1 0 if edge of viz, 0 1 if known inviz.
  //the vis flags on the tilemap itself will be ignored, though the lighting flags will be used.
//buf should be no smaller than the entire room.
//when the values in buf are used, they should be masked against MAP_VIS_TILE_PART
void map_get_visible_tiles(Map m, unsigned char *flags, Volume vol);
void map_get_visible_objects(Map m, TCOD_list_t objs, unsigned char *flags);

unsigned char map_item_index(unsigned short flags);
unsigned char map_item_flags(unsigned short mapItem);
int map_item_lit(unsigned short flags);
int map_item_in_volume(unsigned short flags);
int map_item_los(unsigned short flags);
int map_item_visible(unsigned short mapItem);

//is this API ok?  won't there be a fair bit of redundant work for objects with multiple lights?
void map_note_light_added(Map m, unsigned char attenuation, int intensity, Volume vol);
void map_note_light_removed(Map m, unsigned char attenuation, int intensity, Volume vol);
void map_note_light_volume_changed(Map m, unsigned char attenuation, int intensity, Volume old, Volume new);

#endif