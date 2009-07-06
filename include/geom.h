#ifndef _GEOM_H
#define _GEOM_H

#define PI 3.14159
#define ONE16TH ((2*PI)/16.0)

#define CLIP(b, a, c) (b < a ? a : (b > c ? c : b))

typedef enum {
  DirNone   =  0,
  DirXMinus =  1,
  DirXPlus  =  2,
  DirX      =  3,
  DirYMinus =  4,
  DirYPlus  =  8,
  DirY      = 12,
  DirZMinusIn  = 16,
  DirZMinusOut = 32,
  DirZMinus    = 48,
  DirZPlusIn   = 64,
  DirZPlusOut  = 128,
  DirZPlus     = 192,
  DirZ         = 240
} Direction;

Direction direction_between(int pX, int pY, int pZ, int x, int y, int z, int surfaceZ);

typedef enum {
  NegativeHalfSpace=-1,
  OnPlane=0,
  PositiveHalfSpace=1
  } halfSpace;

typedef struct {
  float x, y, z;
  } mapVec;

extern mapVec mapvec_zero;
int mapvec_equal(mapVec a, mapVec b);
float mapvec_magnitude(mapVec v);
mapVec mapvec_normalize(mapVec v);
mapVec mapvec_add(mapVec v1, mapVec v2);
mapVec mapvec_add_scalar(mapVec v1, double d);
mapVec mapvec_subtract(mapVec v1, mapVec v2);
mapVec mapvec_subtract_scalar(mapVec v1, double d);
mapVec mapvec_multiply_scalar(mapVec v, double scalar);
mapVec mapvec_divide_scalar(mapVec v, double scalar);
float mapvec_distance(mapVec p1, mapVec p2);
float mapvec_facing_to_radians(mapVec f, int aboutZ);
mapVec mapvec_rotate(mapVec p, mapVec center, float rads, int aboutZ);
mapVec mapvec_turn_facing(mapVec facing, int amt);

int tile_index(int x, int y, int z, mapVec sz, mapVec borig, mapVec bsz);

typedef struct {
  float a,b,c,d;
  } Plane;

Plane plane_make(float a, float b, float c, float d);
Plane plane_make_points(mapVec p1, mapVec p2, mapVec p3);
Plane plane_normalize(Plane p);
float plane_distance_to_point(Plane p, mapVec pt);
halfSpace plane_classify_point(Plane plane, mapVec pt, float radius);

//does this belong here?
typedef struct _perception {
  unsigned underlit     : 4;
  unsigned surflit      : 4;
  unsigned toplit       : 4; //12bit
  
  unsigned underlos     : 2;
  unsigned surflos      : 2;
  unsigned toplos       : 2; //6bit
  
  unsigned undervol     : 2;
  unsigned surfvol      : 2;
  unsigned topvol       : 2; //6bit
  //24bit
} perception;

extern perception percept_none;

#endif