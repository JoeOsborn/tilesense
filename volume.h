#ifndef _VOLUME_H
#define _VOLUME_H

#include "geom.h"

struct _frustum {
  mapVec facing;
  Plane left, right, up, down, near, far;
  int fovx, fovz;
  int neary, fary;
  };
struct _sphere {
  float radius;
};
struct _box { //looks like a frustum so that the same inclusion test can be used
  mapVec facing;
  Plane left, right, up, down, near, far;
  mapVec size;
};

typedef enum {
  VolumeTypeNull=0,
  VolumeTypeFrustum,
  VolumeTypeSphere,
  VolumeTypeBox,
  VolumeTypeAABox
} VolumeType;

struct _volume {
  VolumeType type;
  mapVec position;
  union _volume_guts {
    struct _frustum frustum;
    struct _sphere sphere;
    struct _box box;
  } vol;
};

typedef struct _volume * Volume;

typedef Volume Frustum;
typedef Volume Sphere;
typedef Volume Box;
typedef Volume AABox;

Volume volume_copy(Volume v);
void volume_free(Volume v);

int volume_contains_point(Volume v, mapVec pt, float radius);

mapVec volume_position(Volume v);
void volume_set_position(Volume v, mapVec p);
mapVec volume_facing(Volume v);
void volume_set_facing(Volume v, mapVec face);

Frustum frustum_new();
Frustum frustum_init(Frustum f, mapVec pos, mapVec facing, int fovx, int fovz, int near, int far);
Frustum frustum_copy(Frustum f);
void frustum_free(Frustum f);

mapVec frustum_position(Frustum f);
void frustum_set_position(Frustum f, mapVec pt);
mapVec frustum_facing(Frustum f);
void frustum_set_facing(Frustum f, mapVec face);
int frustum_contains_point(Frustum f, mapVec pt, float radius);

Sphere sphere_new();
Sphere sphere_init(Sphere s, mapVec pos, float radius);
Sphere sphere_copy(Sphere s);
void sphere_free(Sphere s);

mapVec sphere_position(Sphere s);
void sphere_set_position(Sphere s, mapVec pt);
int sphere_contains_point(Sphere s, mapVec pt, float radius);

Box box_new();
Box box_init(Box b, mapVec pos, mapVec facing, mapVec size);
Box box_copy(Box b);
void box_free(Box b);

mapVec box_position(Box b);
void box_set_position(Box b, mapVec pt);
mapVec box_size(Box b);
mapVec box_facing(Box b);
void box_set_facing(Box b, mapVec face);
int box_contains_point(Box b, mapVec pt, float radius);

AABox aabox_new();
AABox aabox_init(AABox b, mapVec pos, mapVec size);
AABox aabox_copy(AABox b);
void aabox_free(AABox b);

mapVec aabox_position(AABox b);
void aabox_set_position(AABox b, mapVec pt);
mapVec aabox_size(AABox b);
int aabox_contains_point(AABox b, mapVec pt, float radius);

#endif