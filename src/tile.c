#include "tile.h"
#include <stdlib.h>
#include "tslist.h"

Tile tile_new() {
  return malloc(sizeof(struct _tile));
}
Tile tile_init(
  Tile t,
  unsigned char wallTransparent,
  unsigned char floorTransparent,
  unsigned char ceilTransparent,
  void *context
) {
  t->transparency.wall = wallTransparent;
  t->transparency.floor = floorTransparent;
  t->transparency.ceiling = ceilTransparent;
  t->context = context;
  return t;
}
void tile_free(Tile t) {
  free(t);
}
bool tile_wall_transparent(Tile t) {
  return t->transparency.wall;
}
bool tile_floor_transparent(Tile t) {
  return t->transparency.floor;
}
bool tile_ceiling_transparent(Tile t) {
  return t->transparency.ceiling;
}

void *tile_context(Tile t) {
  return t->context;
}