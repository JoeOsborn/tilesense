#ifndef _TILE_H
#define _TILE_H

#include "geom.h"
#include "flagset.h"

//consider replacing opacity and reserved with a base Flagset using a specific, implicit schema.
//also, should tiles have ID strings?
struct _tile {
  unsigned opacity          :  2;
  unsigned reserved         : 30;
};
typedef struct _tile * Tile;

Tile tile_new();
Tile tile_init(
  Tile t, 
  unsigned char opacity
);
void tile_free(Tile t);
int tile_opacity(Tile t);

#endif