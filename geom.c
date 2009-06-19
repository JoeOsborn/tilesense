#include "geom.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

mapVec mapvec_zero = {0, 0, 0};

int mapvec_equal(mapVec a, mapVec b) {
  const float eps = 0.00001;
  return (fabs(a.x - b.x) < eps) && (fabs(a.y - b.y) < eps) && (fabs(a.z - b.z) < eps);
}

float mapvec_magnitude(mapVec v) {
  return sqrt(v.x*v.x+v.y*v.y+v.z*v.z);
}
mapVec mapvec_normalize(mapVec v) {
  return mapvec_divide_scalar(v, mapvec_magnitude(v));
}

mapVec mapvec_add(mapVec v1, mapVec v2) {
  return (mapVec) {v1.x+v2.x, v1.y+v2.y, v1.z+v2.z};
}

mapVec mapvec_add_scalar(mapVec v, double scalar) {
  return (mapVec){v.x+scalar, v.y+scalar, v.z+scalar};
}

mapVec mapvec_subtract(mapVec v1, mapVec v2) {
  return (mapVec){v1.x-v2.x, v1.y-v2.y, v1.z-v2.z};
}

mapVec mapvec_multiply_scalar(mapVec v, double scalar) {
  return (mapVec){v.x*scalar, v.y*scalar, v.z*scalar};
}

mapVec mapvec_divide_scalar(mapVec v, double scalar) {
  return (mapVec){v.x/scalar, v.y/scalar, v.z/scalar};
}

float mapvec_distance(mapVec p1, mapVec p2) {
  float dx = p2.x-p1.x, dy = p2.y-p1.y, dz = p2.z-p1.z;
  return sqrt(dx*dx+dy*dy+dz*dz);
}

float mapvec_facing_to_radians(mapVec f, int aboutZ) {
  if(mapvec_equal(f, mapvec_zero)) { return 0; }
  if(aboutZ) {
    return atan2(f.y, f.x);    
  }
  return 0;
}

mapVec mapvec_rotate(mapVec p, mapVec center, float rads, int aboutZ) {
  //if(fabs(rads) < 0.00001) { return p; }
  mapVec opt = mapvec_subtract(p, center);
  if(aboutZ) {
    float mag = sqrt(opt.x*opt.x+opt.y*opt.y);
    float baseRad = mapvec_facing_to_radians(opt, 1);
    float finalRad = baseRad + rads;
    mapVec rot = (mapVec){cos(finalRad)*mag, sin(finalRad)*mag, opt.z};
    return mapvec_add(center, rot);
  }
  return p;
}

mapVec mapvec_turn_facing(mapVec f, int amt) {
  //amt is a 45 degree increment
  float rads = (PI/4.0) * amt;
  if(mapvec_equal(f, mapvec_zero)) { 
    f = (mapVec){1, 0, 0};
  }
  //rotate it about zero
  return mapvec_normalize(mapvec_rotate(f, mapvec_zero, rads, 1));
}

int tile_index(int x, int y, int z, mapVec sz) {
  return sz.x*sz.y*(sz.z-z-1)+sz.x*y+x;
}

Plane plane_make(float a, float b, float c, float d) {
//  return plane_normalize((Plane){a, b, c, d});
  return (Plane){a,b,c,d};
}

Plane plane_make_points(mapVec p1, mapVec p2, mapVec p3) {
  /*
  A = y1 (z2 - z3) + y2 (z3 - z1) + y3 (z1 - z2) 
  B = z1 (x2 - x3) + z2 (x3 - x1) + z3 (x1 - x2) 
  C = x1 (y2 - y3) + x2 (y3 - y1) + x3 (y1 - y2) 
  - D = x1 (y2 z3 - y3 z2) + x2 (y3 z1 - y1 z3) + x3 (y1 z2 - y2 z1)
  */
  return plane_make(
    p1.y*(p2.z-p3.z) + p2.y*(p3.z-p1.z) + p3.y*(p1.z-p2.z),
    p1.z*(p2.x-p3.x) + p2.z*(p3.x-p1.x) + p3.z*(p1.x-p2.x),
    p1.x*(p2.y-p3.y) + p2.x*(p3.y-p1.y) + p3.x*(p1.y-p2.y),
    -1*(p1.x*(p2.y*p3.z - p3.y*p2.z) + p2.x*(p3.y*p1.z - p1.y*p3.z) + p3.x*(p1.y*p2.z - p2.y*p1.z))
  );
}

float plane_distance_to_point(Plane plane, mapVec pt) {
  return plane.a*pt.x + plane.b*pt.y + plane.c*pt.z + plane.d;
}

halfSpace plane_classify_point(Plane plane, mapVec pt, float rad) {
  float d = plane_distance_to_point(plane, pt); 
  if (d < -1*rad) return NegativeHalfSpace; 
  if (d > 1*rad) return PositiveHalfSpace;
  return OnPlane;
}

Plane plane_normalize(Plane p) {
  Plane r=p;
  float mag; 
  mag = sqrt(r.a * r.a + r.b * r.b + r.c * r.c); 
  r.a = r.a / mag; 
  r.b = r.b / mag; 
  r.c = r.c / mag; 
  r.d = r.d / mag;
  return r;
}

