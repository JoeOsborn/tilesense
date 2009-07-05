#include "tile.h"
#include <stdlib.h>

#define TILE_OPACITY_PART_SZ 4
#define TILE_OPACITY_XM_OFF  0
#define TILE_OPACITY_XP_OFF  4
#define TILE_OPACITY_YM_OFF  8
#define TILE_OPACITY_YP_OFF 12
#define TILE_OPACITY_ZM_OFF 16
#define TILE_OPACITY_ZP_OFF 20


Flagset tile_opacity_flagset_make() {
  return flagset_init_raw(flagset_new_raw(TILE_OPACITY_PART_SZ*6), TILE_OPACITY_PART_SZ*6);
}
Flagset tile_opacity_flagset_set(Flagset opacity, 
  unsigned char xm, unsigned char xp, 
  unsigned char ym, unsigned char yp, 
  unsigned char zm, unsigned char zp
) {
  if(xm <= 15) {
    flagset_set_raw(opacity, TILE_OPACITY_XM_OFF, TILE_OPACITY_PART_SZ, xm);    
  }
  if(xp <= 15) {
    flagset_set_raw(opacity, TILE_OPACITY_XP_OFF, TILE_OPACITY_PART_SZ, xp);    
  }
  if(ym <= 15) {
    flagset_set_raw(opacity, TILE_OPACITY_YM_OFF, TILE_OPACITY_PART_SZ, ym);    
  }
  if(yp <= 15) {
    flagset_set_raw(opacity, TILE_OPACITY_YP_OFF, TILE_OPACITY_PART_SZ, yp);    
  }
  if(zm <= 15) {
    flagset_set_raw(opacity, TILE_OPACITY_ZM_OFF, TILE_OPACITY_PART_SZ, zm);    
  }
  if(zp <= 15) {
    flagset_set_raw(opacity, TILE_OPACITY_ZP_OFF, TILE_OPACITY_PART_SZ, zp);    
  }
  return opacity;
}



Tile tile_new() {
  return malloc(sizeof(struct _tile));
}
Tile tile_init(
  Tile t,
  Flagset opacity,
  void *context
) {
  t->opacity = opacity;
  t->context = context;
  return t;
}
void tile_free(Tile t) {
  flagset_free(t->opacity);
  free(t);
}
Flagset tile_opacity(Tile t) {
  return t->opacity;
}
unsigned char tile_opacity_direction(Tile t, Direction direction) {
  //if the line is moving in multiple directions, take the higher of the opacities...
  unsigned int blockage = 0;
  if(direction & DirXMinus) {
    blockage = tile_opacity_xm(t);
  }
  if(direction & DirXPlus) {
    blockage = tile_opacity_xp(t);
  }
  if(direction & DirYMinus) {
    blockage = MAX(blockage, tile_opacity_ym(t));
  }
  if(direction & DirYPlus) {
    blockage = MAX(blockage, tile_opacity_yp(t));
  }
  //unless it's moving in z, in which case we use z, not x/y
  if(direction & DirZMinus) {
    blockage = tile_opacity_zm(t);
  }
  if(direction & DirZPlus) {
    blockage = tile_opacity_zp(t);
  }
  return (unsigned char)blockage;
}

unsigned char tile_opacity_xm(Tile t) {
  return flagset_get_raw(t->opacity, TILE_OPACITY_XM_OFF, TILE_OPACITY_PART_SZ);
}
unsigned char tile_opacity_xp(Tile t) {
  return flagset_get_raw(t->opacity, TILE_OPACITY_XP_OFF, TILE_OPACITY_PART_SZ);
}
unsigned char tile_opacity_ym(Tile t) {
  return flagset_get_raw(t->opacity, TILE_OPACITY_YM_OFF, TILE_OPACITY_PART_SZ);
}
unsigned char tile_opacity_yp(Tile t) {
  return flagset_get_raw(t->opacity, TILE_OPACITY_YP_OFF, TILE_OPACITY_PART_SZ);
}
unsigned char tile_opacity_zm(Tile t) {
  return flagset_get_raw(t->opacity, TILE_OPACITY_ZM_OFF, TILE_OPACITY_PART_SZ);
}
unsigned char tile_opacity_zp(Tile t) {
  return flagset_get_raw(t->opacity, TILE_OPACITY_ZP_OFF, TILE_OPACITY_PART_SZ);
}
void *tile_context(Tile t) {
  return t->context;
}