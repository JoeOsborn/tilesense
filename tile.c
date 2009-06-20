#include "tile.h"

Tile tile_new() {
  return malloc(sizeof(struct _tile));
}
Tile tile_init(
  Tile t, 
  unsigned char mq, 
  unsigned char mc, 
  unsigned char mt,
  
  unsigned char ft,
  mapVec fo,
  unsigned int fd,
  
  unsigned char lb,
  unsigned char latt,
  unsigned char larc,
  
  unsigned char flg
) {
  t->materialQuality = mq;
  t->materialCategory = mc;
  t->materialType = mt;
  
  t->featureType = ft;
  t->featureOrientation = fo;
  t->featureData = fd;
  
  t->lightingBrightness = lb;
  t->lightingAttenuation = latt;
  t->lightingArc = larc;
  
  t->flags = flg;
  
  return t;
}
void tile_free(Tile t) {
  free(t);
}
int tile_light_blockage(Tile t) {
  return (t->flags & 0x60) >> 5;
}