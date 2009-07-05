#ifndef _TILE_H
#define _TILE_H

#include "geom.h"
#include "flagset.h"

Flagset tile_opacity_flagset_make();
Flagset tile_opacity_flagset_set(Flagset fs, 
  unsigned char xm, unsigned char xp, 
  unsigned char ym, unsigned char yp, 
  unsigned char zm, unsigned char zp
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
unsigned char tile_opacity_xm(Tile t);
unsigned char tile_opacity_xp(Tile t);
unsigned char tile_opacity_ym(Tile t);
unsigned char tile_opacity_yp(Tile t);
unsigned char tile_opacity_zm(Tile t);
unsigned char tile_opacity_zp(Tile t);
void *tile_context(Tile t);
#endif