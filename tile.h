#ifndef _TILE_H
#define _TILE_H

#include "geom.h"

struct _tile { //later, use struct alignment weird syntax
  unsigned char materialQuality;      //2 bits
  unsigned char materialCategory;     //4 bits
  unsigned short materialType;        //10 bits
  
  unsigned char featureType;          //8 bits
  mapVec featureOrientation; //2 bits each x,y,z
  unsigned int featureData;           //18 bits
  
  unsigned char lightingBrightness;  //2 bits
  unsigned char lightingAttenuation; //2 bits
  unsigned char lightingArc;         //4 bits
  
  unsigned char flags;               //8 bits
};
typedef struct _tile * Tile;

Tile tile_new();
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
);
void tile_free(Tile t);
int tile_light_blockage(Tile t);

#endif