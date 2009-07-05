#include "tile.h"
#include <stdlib.h>

#define TILE_OPACITY_PART_SZ 4
#define TILE_OPACITY_XM_OFF  0
#define TILE_OPACITY_XP_OFF  4
#define TILE_OPACITY_YM_OFF  8
#define TILE_OPACITY_YP_OFF 12
#define TILE_OPACITY_FIN_OFF 16
#define TILE_OPACITY_FOUT_OFF 20
#define TILE_OPACITY_CIN_OFF 24
#define TILE_OPACITY_COUT_OFF 28


Flagset tile_opacity_flagset_make() {
  return flagset_init_raw(flagset_new_raw(TILE_OPACITY_PART_SZ*8), TILE_OPACITY_PART_SZ*8);
}
Flagset tile_opacity_flagset_set(Flagset opacity, 
  unsigned char xm, unsigned char xp, 
  unsigned char ym, unsigned char yp, 
  unsigned char fin, unsigned char fout,
  unsigned char cin, unsigned char cout
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
  if(fin <= 15) {
    flagset_set_raw(opacity, TILE_OPACITY_FIN_OFF, TILE_OPACITY_PART_SZ, fin);    
  }
  if(fout <= 15) {
    flagset_set_raw(opacity, TILE_OPACITY_FOUT_OFF, TILE_OPACITY_PART_SZ, fout);    
  }
  if(cin <= 15) {
    flagset_set_raw(opacity, TILE_OPACITY_CIN_OFF, TILE_OPACITY_PART_SZ, cin);    
  }
  if(cout <= 15) {
    flagset_set_raw(opacity, TILE_OPACITY_COUT_OFF, TILE_OPACITY_PART_SZ, cout);    
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
  unsigned int zBlock = 0;
  //Z overrides regular blockages if present.  is this correct this way?
  if(direction & DirZMinusIn) { //entering from the ceiling
    zBlock = tile_opacity_cin(t);
    blockage = zBlock ? zBlock : blockage;
  }
  if(direction & DirZMinusOut) { //exiting from the floor
    zBlock = tile_opacity_fout(t);
    blockage = zBlock ? zBlock : blockage;
  }
  if(direction & DirZPlusIn) { //entering from the floor
    zBlock = tile_opacity_fin(t);
    blockage = zBlock ? zBlock : blockage;
  }
  if(direction & DirZPlusOut) { //exiting from the ceiling
    zBlock = tile_opacity_cout(t);
    blockage = zBlock ? zBlock : blockage;
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
unsigned char tile_opacity_fin(Tile t) {
  return flagset_get_raw(t->opacity, TILE_OPACITY_FIN_OFF, TILE_OPACITY_PART_SZ);
}
unsigned char tile_opacity_fout(Tile t) {
  return flagset_get_raw(t->opacity, TILE_OPACITY_FOUT_OFF, TILE_OPACITY_PART_SZ);
}
unsigned char tile_opacity_cin(Tile t) {
  return flagset_get_raw(t->opacity, TILE_OPACITY_CIN_OFF, TILE_OPACITY_PART_SZ);
}
unsigned char tile_opacity_cout(Tile t) {
  return flagset_get_raw(t->opacity, TILE_OPACITY_COUT_OFF, TILE_OPACITY_PART_SZ);
}
void *tile_context(Tile t) {
  return t->context;
}