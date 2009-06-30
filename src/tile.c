#include "tile.h"
#include <stdlib.h>

Tile tile_new() {
  return malloc(sizeof(struct _tile));
}
Tile tile_init(
  Tile t, 
  unsigned char opacity,
  FlagSchema fsc
) {
  t->opacity = opacity;
  t->flags = flagset_init(flagset_new(fsc), fsc);
  return t;
}
void tile_free(Tile t) {
  flagset_free(t->flags);
  free(t);
}
Flagset tile_flags(Tile t) {
  return t->flags;
}
int tile_opacity(Tile t) {
  return t->opacity;
}