#ifndef _TILE_H
#define _TILE_H

#include "geom.h"
#include "flagset.h"

//replace all these with two Flagsets: base (for stuff like opacity, sound transmission, 
//and other elements used by tilesense) and custom.
//Tiles must be initialized with both schema (for thread safety; theoretically, base
//could be a global if it's created atomically; in any event, tile_init_base_schema()
//should exist.)

//On the other hand, if schema are never likely to differ between tiles (as in base), it
//may be more efficient to use the struct syntax.
//But then again, it's easier to pass flagsets across the erlang-c border.
//HOWEVER, it may not be necessary to pass opacity, etc across this border frequently, if at all;
//in fact, all tiles -really- need is a way to uniquely identify them for the sake of the client.
//still, I like the idea of initializing them with a flagschema best of all, and they can even
//have a pointer to the flagschema since there won't be that many tiles.... but no, that's unpleasant.
//initializing them with a flagschema lets the map do the legwork of mapping tile indices onto tile defs.


struct _tile {
  unsigned opacity          :  2;
  unsigned reserved         : 30;
  Flagset flags;
};
typedef struct _tile * Tile;

Tile tile_new();
Tile tile_init(
  Tile t, 
  unsigned char opacity,
  FlagSchema fsc
);
void tile_free(Tile t);
Flagset tile_flags(Tile t);
int tile_opacity(Tile t);

#endif