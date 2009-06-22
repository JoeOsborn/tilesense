#include "tile.h"
#include <stdlib.h>

Tile tile_new() {
  return malloc(sizeof(struct _tile));
}
Tile tile_init(
  Tile t, 
  unsigned char mq, 
  unsigned char mc, 
  unsigned short mt,
  
  unsigned char ft,
  unsigned char faceOctants,
  unsigned int fd,
  
  unsigned char solid,
  unsigned char opacity,
  unsigned flags
) {
  t->materialQuality = mq;
  t->materialCategory = mc;
  t->materialType = mt;
  
  t->featureType = ft;
  t->featureFacing = faceOctants;
  t->featureData = fd;
  
  t->solid = solid;
  t->opacity = opacity;
  t->flags = flags;
  
  return t;
}
void tile_free(Tile t) {
  free(t);
}
int tile_opacity(Tile t) {
  return t->opacity;
}