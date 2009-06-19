#include "tile.h"
#include <stdlib.h>
#include <stdio.h>

#include <ncurses.h>

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

TileSet tileset_new() {
  return malloc(sizeof(struct _tile_set));
}
TileSet tileset_init(TileSet ts) {
  ts->tiles = calloc(256, sizeof(Tile *));
  ts->tileCount = 0;
  Tile null_tile = tile_new();
  tile_init(null_tile, 0, 0, 0, 0, mapvec_zero, 0, 0, 0, 0, 0);
  tileset_add_tile(ts, null_tile);
  return ts;
}
void tileset_free(TileSet ts) {
  for(int i = 0; i < ts->tileCount; i++) {
    free(ts->tiles[i]);
  }
  free(ts);
}
TileSet tileset_add_tile(TileSet ts, Tile t) {
  if(ts->tileCount < 256) {
    ts->tiles[ts->tileCount] = t;
    ts->tileCount++;
    return ts;
  }
  printf("Added too many tiles to tileset\n");
  tileset_free(ts);
  endwin();
  exit(-1);
  return NULL;
}
TileSet tileset_add_tiles(TileSet ts, Tile *t, int tileCount) {
  TileSet ret;
  for(int i = 0; i < tileCount; i++) {
    ret = tileset_add_tile(ts, t[i]);
  }
  return ret;
}
int tileset_tile_count(TileSet ts) {
  return ts->tileCount;
}
Tile tileset_tile(TileSet ts, int index) {
  if(index < ts->tileCount) {
    return ts->tiles[index];
  }
  printf("Seeking tile %i beyond end of tileset\n", index);
  tileset_free(ts);
  endwin();
  exit(-1);
  return NULL;
}
