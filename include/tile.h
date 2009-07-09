#ifndef _TILE_H
#define _TILE_H

#include "geom.h"
#include "flagset.h"

struct _tile {
  struct {
    unsigned wall:1;
    unsigned floor:1;
    unsigned ceiling:1;
  } transparency;
  void *context;
};
typedef struct _tile * Tile;

Tile tile_new();
Tile tile_init(
  Tile t, 
  unsigned char wallTransparent,
  unsigned char floorTransparent,
  unsigned char ceilTransparent,
  void *ctx
);
void tile_free(Tile t);
bool tile_wall_transparent(Tile t);
bool tile_floor_transparent(Tile t);
bool tile_ceiling_transparent(Tile t);
void *tile_context(Tile t);
#endif