#ifndef _TILE_H
#define _TILE_H

#include "geom.h"

//replace all these with two Flagsets: base (for stuff like opacity, sound transmission, 
//and other elements used by tilesense) and custom.
//Tiles must be initialized with both schema (for thread safety; theoretically, base
//could be a global if it's created atomically.)
struct _tile {
  unsigned materialQuality  :  2;
  unsigned materialCategory :  4;
  unsigned materialType     : 10;
  
  unsigned featureType      :  8;
  unsigned featureFacing    :  3; //number of octants from 0
  unsigned featureData      : 21;
    
  unsigned opacity          :  2;
  unsigned flags            :  6;
};
typedef struct _tile * Tile;

Tile tile_new();
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
);
void tile_free(Tile t);
int tile_opacity(Tile t);

#endif