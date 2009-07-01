#include "tile.h"
#include <stdlib.h>

Tile tile_new() {
  return malloc(sizeof(struct _tile));
}
Tile tile_init(
  Tile t, 
  unsigned char opacity
) {
  t->opacity = opacity;
  return t;
}
void tile_free(Tile t) {
  free(t);
}
int tile_opacity(Tile t) {
  return t->opacity;
}