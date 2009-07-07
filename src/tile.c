#include "tile.h"
#include <stdlib.h>

#define TILE_OPACITY_PART_SZ     4
#define TILE_OPACITY_XM_OFF      0
#define TILE_OPACITY_XP_OFF      4
#define TILE_OPACITY_YM_OFF      8
#define TILE_OPACITY_YP_OFF     12
#define TILE_OPACITY_ZM_OUT_OFF 16
#define TILE_OPACITY_ZM_IN_OFF  20
#define TILE_OPACITY_ZP_OUT_OFF 24
#define TILE_OPACITY_ZP_IN_OFF  28


Flagset tile_opacity_flagset_make() {
  return flagset_init_raw(flagset_new_raw(TILE_OPACITY_PART_SZ*8), TILE_OPACITY_PART_SZ*8);
}
Flagset tile_opacity_flagset_set(Flagset opacity, 
  unsigned char xm, unsigned char xp, 
  unsigned char ym, unsigned char yp, 
  unsigned char zmOut, unsigned char zmIn,
  unsigned char zpOut, unsigned char zpIn
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
  if(zmOut <= 15) {
    flagset_set_raw(opacity, TILE_OPACITY_ZM_OUT_OFF, TILE_OPACITY_PART_SZ, zmOut);    
  }
  if(zmIn <= 15) {
    flagset_set_raw(opacity, TILE_OPACITY_ZM_IN_OFF, TILE_OPACITY_PART_SZ, zmIn);    
  }
  if(zpOut <= 15) {
    flagset_set_raw(opacity, TILE_OPACITY_ZP_OUT_OFF, TILE_OPACITY_PART_SZ, zpOut);    
  }
  if(zpIn <= 15) {
    flagset_set_raw(opacity, TILE_OPACITY_ZP_IN_OFF, TILE_OPACITY_PART_SZ, zpIn);    
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

#define GET_PART(Part) (flagset_get_raw(t->opacity, Part, TILE_OPACITY_PART_SZ))
unsigned char tile_opacity_direction(Tile t, Direction direction) {
  //if the line is moving in multiple directions, take the higher of the opacities...
  unsigned char blockage = 0;
  if(direction & DirXMinus) {
    blockage = MAX(blockage, GET_PART(TILE_OPACITY_XM_OFF));
  }
  if(direction & DirXPlus) {
    blockage = MAX(blockage, GET_PART(TILE_OPACITY_XP_OFF));
  }
  if(direction & DirYMinus) {
    blockage = MAX(blockage, GET_PART(TILE_OPACITY_YM_OFF));
  }
  if(direction & DirYPlus) {
    blockage = MAX(blockage, GET_PART(TILE_OPACITY_YP_OFF));
  }
  if(direction & DirZMinusIn) { //entering from the ceiling
    blockage = MAX(blockage, GET_PART(TILE_OPACITY_ZM_IN_OFF));
  }
  if(direction & DirZMinusOut) { //exiting from the floor
    blockage = MAX(blockage, GET_PART(TILE_OPACITY_ZM_OUT_OFF));
  }
  if(direction & DirZPlusIn) { //entering from the floor
    blockage = MAX(blockage, GET_PART(TILE_OPACITY_ZP_IN_OFF));
  }
  if(direction & DirZPlusOut) { //exiting from the ceiling
    blockage = MAX(blockage, GET_PART(TILE_OPACITY_ZP_OUT_OFF));
  }
  return blockage;
}
#undef GET_PART

void *tile_context(Tile t) {
  return t->context;
}