#ifndef _TILE_H
#define _TILE_H

#include "geom.h"
#include "flagset.h"

Flagset tile_opacity_flagset_make();
Flagset tile_opacity_flagset_set(Flagset fs, 
  unsigned char xm, unsigned char xp,  //light leaving to the left, light leaving to the right
  unsigned char ym, unsigned char yp,  //etc...
  unsigned char floorIn, unsigned char floorOut,
  unsigned char ceilIn, unsigned char ceilOut
);

struct _tile {
  Flagset opacity;
  void *context;
};
typedef struct _tile * Tile;

Tile tile_new();
Tile tile_init(
  Tile t, 
  Flagset opacity,
  void *ctx
);
void tile_free(Tile t);
Flagset tile_opacity(Tile t);
unsigned char tile_opacity_direction(Tile t, Direction direction);
void *tile_context(Tile t);
#endif