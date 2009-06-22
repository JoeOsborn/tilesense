#include "volume.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <libtcod.h>

Volume _volume_new() {
  return malloc(sizeof(struct _volume));
}
Volume _volume_raw_copy(Volume v) {
  Volume cp = malloc(sizeof(struct _volume));
  memcpy(cp, v, sizeof(struct _volume));
  return cp;
}
Volume _volume_init(Volume v, VolumeType typ, mapVec position) {
  v->type = typ;
  v->position = position;
  return v;
}
void _volume_free(Volume v) {
  free(v);
}
Volume volume_copy(Volume v) {
  switch (v->type) {
    case VolumeTypeFrustum:
      return frustum_copy(v);
    case VolumeTypeSphere:
      return sphere_copy(v);
    case VolumeTypeBox:
      return box_copy(v);
    case VolumeTypeAABox:
      return aabox_copy(v);
    default:
      return NULL;
  }
}
void volume_free(Volume v) {
  switch(v->type) {
    case VolumeTypeFrustum:
      frustum_free(v);
      break;
    case VolumeTypeSphere:
      sphere_free(v);
      break;
    case VolumeTypeBox:
      box_free(v);
      break;
    case VolumeTypeAABox:
      aabox_free(v);
      break;
    default:
      _volume_free(v);
      break;
  }
}

int volume_contains_point(Volume v, mapVec pt, float radius) {
  switch(v->type) {
    case VolumeTypeFrustum:
      return frustum_contains_point(v, pt, radius);
    case VolumeTypeSphere:
      return sphere_contains_point(v, pt, radius);
    case VolumeTypeBox:
      return box_contains_point(v, pt, radius);
    case VolumeTypeAABox:
      return aabox_contains_point(v, pt, radius);
    default:
      return 0;
  }
}
mapVec volume_position(Volume v) {
  switch(v->type) {
    case VolumeTypeFrustum:
      return frustum_position(v);
    case VolumeTypeSphere:
      return sphere_position(v);
    case VolumeTypeBox:
      return box_position(v);
    case VolumeTypeAABox:
      return aabox_position(v);
    default:
      return mapvec_zero;
  }
}
void volume_set_position(Volume v, mapVec p) {
  switch(v->type) {
    case VolumeTypeFrustum:
      frustum_set_position(v, p);
      break;
    case VolumeTypeSphere:
      sphere_set_position(v, p);
      break;
    case VolumeTypeBox:
      box_set_position(v, p);
      break;
    case VolumeTypeAABox:
      aabox_set_position(v, p);
      break;
    default:
      break;
  }
}
mapVec volume_facing(Volume v) {
    switch(v->type) {
      case VolumeTypeFrustum:
        return frustum_facing(v);
      case VolumeTypeSphere:
        return mapvec_zero;
      case VolumeTypeBox:
        return box_facing(v);
      case VolumeTypeAABox:
        return mapvec_zero;
      default:
        return mapvec_zero;
    }
}
void volume_set_facing(Volume v, mapVec face)
{
  switch(v->type) {
    case VolumeTypeFrustum:
      frustum_set_facing(v, face);
      break;
    case VolumeTypeSphere:
//      sphere_set_facing(v, face);
      break;
    case VolumeTypeBox:
      box_set_facing(v, face);
      break;
    case VolumeTypeAABox:
//      aabox_set_facing(v, face);
      break;
    default:
      break;
  }
}
void volume_swept_bounds(Volume v, mapVec *pos, mapVec *sz)
{
  switch(v->type) {
    case VolumeTypeFrustum:
      frustum_swept_bounds(v, pos, sz);
      break;
    case VolumeTypeSphere:
      sphere_swept_bounds(v, pos, sz);
      break;
    case VolumeTypeBox:
      box_swept_bounds(v, pos, sz);
      break;
    case VolumeTypeAABox:
      aabox_swept_bounds(v, pos, sz);
      break;
    default:
      break;
  }
}

void _frustum_remake_planes(Frustum f) {
  mapVec pos = f->position;
  mapVec facing = f->vol.frustum.facing;
  float fovx = f->vol.frustum.fovx;
  float fovz = f->vol.frustum.fovz;
  float near = f->vol.frustum.neary;
  if(near == 0) { near = 0.01; }
  float far = f->vol.frustum.fary;
  
  float faceAngle = mapvec_facing_to_radians(facing, 1);
  mapVec center = pos;

  float xang = (ONE16TH*fovx);
  
//  mvprintw(8, 0, "facing: %f, fovx: %f, xang: %f", faceAngle, fovx, xang*180.0/3.14159);
  float txang = tan(xang);
  float dxn = near;
  float dyn = txang*near;
  float dxf = far;
  float dyf = txang*far;
  
  float zang = (ONE16TH*fovz);
  float dzf = (tan(zang)*far);
  float dzn = (tan(zang)*near);
  
  #warning still doesn't quite work in Z
  
  mapVec lp1 = (mapVec){dxn, -dyn, dzn};
  mapVec lp2 = (mapVec){dxf, -dyf, dzf};
  mapVec lp3 = (mapVec){dxf, -dyf, -dzf};
  
  mapVec rp1 = (mapVec){dxn, dyn, -dzn};
  mapVec rp2 = (mapVec){dxf, dyf, -dzf};
  mapVec rp3 = (mapVec){dxf, dyf, dzf};

  mapVec up1 = (mapVec){dxn, dyn, dzn};
  mapVec up2 = (mapVec){dxf, dyf, dzf};
  mapVec up3 = (mapVec){dxf, -dyf, dzf};
  
  mapVec dp1 = (mapVec){dxn, dyn, -dzn};
  mapVec dp2 = (mapVec){dxf, -dyf, -dzf};
  mapVec dp3 = (mapVec){dxf, dyf, -dzf};

  mapVec np1 = (mapVec){dxn, dyn, dzn};
  mapVec np2 = (mapVec){dxn, -dyn, dzn};
  mapVec np3 = (mapVec){dxn, -dyn, -dzn};

  mapVec fp1 = (mapVec){dxf, dyf, -dzf};
  mapVec fp2 = (mapVec){dxf, -dyf, -dzf};
  mapVec fp3 = (mapVec){dxf, dyf, dzf};

//  mvprintw(12, 0, "np1dir: %f, np2dir: %f, np3dir: %f", mapvec_facing_to_radians(np1, 1), mapvec_facing_to_radians(np2, 1), mapvec_facing_to_radians(np3, 1));
//  mvprintw(13, 0, 
//    "n1: %f, %f, %f;\nn2: %f, %f, %f;\nn3: %f, %f, %f", 
//    np1.x,np1.y,np1.z, np2.x,np2.y,np2.z, np3.x,np3.y,np3.z
//  );

  lp1 = mapvec_rotate(lp1, mapvec_zero, faceAngle, 1);
  lp2 = mapvec_rotate(lp2, mapvec_zero, faceAngle, 1);
  lp3 = mapvec_rotate(lp3, mapvec_zero, faceAngle, 1);

  rp1 = mapvec_rotate(rp1, mapvec_zero, faceAngle, 1);
  rp2 = mapvec_rotate(rp2, mapvec_zero, faceAngle, 1);
  rp3 = mapvec_rotate(rp3, mapvec_zero, faceAngle, 1);
  
  up1 = mapvec_rotate(up1, mapvec_zero, faceAngle, 1);
  up2 = mapvec_rotate(up2, mapvec_zero, faceAngle, 1);
  up3 = mapvec_rotate(up3, mapvec_zero, faceAngle, 1);

  dp1 = mapvec_rotate(dp1, mapvec_zero, faceAngle, 1);
  dp2 = mapvec_rotate(dp2, mapvec_zero, faceAngle, 1);
  dp3 = mapvec_rotate(dp3, mapvec_zero, faceAngle, 1);
  
  np1 = mapvec_rotate(np1, mapvec_zero, faceAngle, 1);
  np2 = mapvec_rotate(np2, mapvec_zero, faceAngle, 1);
  np3 = mapvec_rotate(np3, mapvec_zero, faceAngle, 1);
  
  fp1 = mapvec_rotate(fp1, mapvec_zero, faceAngle, 1);
  fp2 = mapvec_rotate(fp2, mapvec_zero, faceAngle, 1);
  fp3 = mapvec_rotate(fp3, mapvec_zero, faceAngle, 1);
  
//  mvprintw(16, 0, "np1dir: %f, np2dir: %f, np3dir: %f", mapvec_facing_to_radians(np1, 1), mapvec_facing_to_radians(np2, 1), mapvec_facing_to_radians(np3, 1));
//  mvprintw(17, 0, 
//    "n1: %f, %f, %f;\nn2: %f, %f, %f;\nn3: %f, %f, %f", 
//    np1.x,np1.y,np1.z, np2.x,np2.y,np2.z, np3.x,np3.y,np3.z
//  );
  
  lp1 = mapvec_add(lp1, center);
  lp2 = mapvec_add(lp2, center);
  lp3 = mapvec_add(lp3, center);

  rp1 = mapvec_add(rp1, center);
  rp2 = mapvec_add(rp2, center);
  rp3 = mapvec_add(rp3, center);

  up1 = mapvec_add(up1, center);
  up2 = mapvec_add(up2, center);
  up3 = mapvec_add(up3, center);

  dp1 = mapvec_add(dp1, center);
  dp2 = mapvec_add(dp2, center);
  dp3 = mapvec_add(dp3, center);

  np1 = mapvec_add(np1, center);
  np2 = mapvec_add(np2, center);
  np3 = mapvec_add(np3, center);

  fp1 = mapvec_add(fp1, center);
  fp2 = mapvec_add(fp2, center);
  fp3 = mapvec_add(fp3, center);
  
//  mvprintw(20, 0, 
//    "n1: %f, %f, %f;\nn2: %f, %f, %f;\nn3: %f, %f, %f", 
//    np1.x,np1.y,np1.z, np2.x,np2.y,np2.z, np3.x,np3.y,np3.z
//  );

  f->vol.frustum.left=plane_make_points(lp1, lp2, lp3);
  f->vol.frustum.right=plane_make_points(rp1, rp2, rp3);
  f->vol.frustum.up=plane_make_points(up1, up2, up3);
  f->vol.frustum.down=plane_make_points(dp1, dp2, dp3);
  f->vol.frustum.near=plane_make_points(np1, np2, np3);
  f->vol.frustum.far=plane_make_points(fp1, fp2, fp3);
}

Frustum frustum_new() {
  return _volume_new();
}
Frustum frustum_init(Frustum f, mapVec pos, mapVec facing, int fovx, int fovz, int near, int far) {
  f = _volume_init(f, VolumeTypeFrustum, pos);
  f->vol.frustum.facing = facing;
  f->vol.frustum.fovx = fovx;
  f->vol.frustum.fovz = fovz;
  f->vol.frustum.neary = near;
  f->vol.frustum.fary = far;
  _frustum_remake_planes(f);
  return f;
}
Frustum frustum_copy(Frustum f) {
  return _volume_raw_copy(f);
}
void frustum_free(Frustum f) {
  _volume_free(f);
}

mapVec frustum_position(Frustum f) {
  return f->position;
}
void frustum_set_position(Frustum f, mapVec p) {
  f->position = p;
  _frustum_remake_planes(f);
}
mapVec frustum_facing(Frustum f) {
  return f->vol.frustum.facing;
}
void frustum_set_facing(Frustum f, mapVec face) {
  f->vol.frustum.facing = face;
  _frustum_remake_planes(f);
}
int frustum_contains_point(Frustum f, mapVec pt, float rad) {
  if(plane_classify_point(f->vol.frustum.near,  pt, rad) == NegativeHalfSpace) { return 0; }
  if(plane_classify_point(f->vol.frustum.far,   pt, rad) == NegativeHalfSpace) { return 0; }
  if(plane_classify_point(f->vol.frustum.left,  pt, rad) == NegativeHalfSpace) { return 0; }
  if(plane_classify_point(f->vol.frustum.right, pt, rad) == NegativeHalfSpace) { return 0; }
  if(plane_classify_point(f->vol.frustum.up,    pt, rad) == NegativeHalfSpace) { return 0; }
  if(plane_classify_point(f->vol.frustum.down,  pt, rad) == NegativeHalfSpace) { return 0; }
  return 1;
}

void frustum_swept_bounds(Frustum f, mapVec *p, mapVec *sz) {
  float fovx = f->vol.frustum.fovx;
  float fovz = f->vol.frustum.fovz;
  float far = f->vol.frustum.fary;
  
  float xang = (ONE16TH*fovx);
  float txang = tan(xang);
  float dxf = far;
  float dyf = txang*far;
  
  float zang = (ONE16TH*fovz);
  float dzf = (tan(zang)*far);
  
  float maxDim = MAX(dxf, dyf);
  
  if(p) {
    *p = mapvec_subtract(f->position, (mapVec){maxDim, maxDim, dzf});
  }
  if(sz) {
    *sz = (mapVec){maxDim*2, maxDim*2, dzf*2};
  }
}

Sphere sphere_new() {
  return _volume_new();
}
Sphere sphere_init(Sphere s, mapVec pos, float radius) {
  s = _volume_init(s, VolumeTypeSphere, pos);
  struct _sphere sphere;
  sphere.radius = radius;
  s->vol.sphere = sphere;
  return s;
}
Sphere sphere_copy(Sphere s) {
  return _volume_raw_copy(s);
}
void sphere_free(Sphere s) {
  _volume_free(s);
}

mapVec sphere_position(Sphere s) {
  return s->position;
}
void sphere_set_position(Sphere s, mapVec p) {
  s->position = p;
}
int sphere_contains_point(Sphere s, mapVec pt, float radius) {
  return (fabs(mapvec_distance(s->position, pt)) - radius) < s->vol.sphere.radius;
}

void sphere_swept_bounds(Sphere s, mapVec *pos, mapVec *sz) {
  float radius = s->vol.sphere.radius;
  if(pos) {
    *pos = mapvec_subtract_scalar(s->position, radius);
  }
  if(sz) {
    *sz = (mapVec){radius*2, radius*2, radius*2};
  }
}

void _box_remake_planes(Box b) {
  mapVec pos = b->position;
  mapVec facing = b->vol.box.facing;
  mapVec size = b->vol.box.size;
  mapVec center = mapvec_add(pos, (mapVec){0, 0, 0});
  float faceAngle = mapvec_facing_to_radians(facing, 1);
  //right, far, up
  mapVec crfu = mapvec_add(pos, mapvec_multiply_scalar(size,  0.5));
  //left, near, down
  mapVec clnd = mapvec_add(pos, mapvec_multiply_scalar(size, -0.5));
  //other six corners
  mapVec crfd = (mapVec){crfu.x, crfu.y, clnd.z};
  mapVec crnd = (mapVec){crfu.x, clnd.y, clnd.z};
  mapVec clfd = (mapVec){clnd.x, crfu.y, clnd.z};
  mapVec crnu = (mapVec){crfu.x, clnd.y, crfu.z};
  mapVec clfu = (mapVec){clnd.x, crfu.y, crfu.z};
  mapVec clnu = (mapVec){clnd.x, clnd.y, crfu.z};
  
  //rotate each point about the center by faceAngle
  clfd = mapvec_rotate(clfd, center, faceAngle, 1);
  crnu = mapvec_rotate(crnu, center, faceAngle, 1);
  clfu = mapvec_rotate(clfu, center, faceAngle, 1);
  clnd = mapvec_rotate(clnd, center, faceAngle, 1);
  clnu = mapvec_rotate(clnu, center, faceAngle, 1);
  crnd = mapvec_rotate(crnd, center, faceAngle, 1);
  crfu = mapvec_rotate(crfu, center, faceAngle, 1);
  crfd = mapvec_rotate(crfd, center, faceAngle, 1);
  
  // mvprintw(12, 0, "facing.x: %f .y: %f .z: %f", facing.x, facing.y, facing.z);
  // mvprintw(13, 0, "faceangle: %f", faceAngle);
  // mvprintw(14, 0, "pos.x: %f .y: %f .z: %f", pos.x, pos.y, pos.z);
  // mvprintw(15, 0, "size.x: %f .y: %f .z: %f", size.x, size.y, size.z);
  // mvprintw(16, 0, "clnd.x: %f .y: %f .z: %f", clnd.x, clnd.y, clnd.z);
  // mvprintw(17, 0, "crfu.x: %f .y: %f .z: %f", crfu.x, crfu.y, crfu.z);
  // mvprintw(18, 0, "clnu.x: %f .y: %f .z: %f", clnu.x, clnu.y, clnu.z);
  // mvprintw(19, 0, "clfd.x: %f .y: %f .z: %f", clfd.x, clfd.y, clfd.z);
  // mvprintw(20, 0, "clfu.x: %f .y: %f .z: %f", clfu.x, clfu.y, clfu.z);
  // mvprintw(21, 0, "crfd.x: %f .y: %f .z: %f", crfd.x, crfd.y, crfd.z);
  // mvprintw(22, 0, "crnu.x: %f .y: %f .z: %f", crnu.x, crnu.y, crnu.z);
  // mvprintw(23, 0, "crnd.x: %f .y: %f .z: %f", crnd.x, crnd.y, crnd.z);
  
  b->vol.box.left = plane_make_points(clfd, clfu, clnu);
  b->vol.box.right = plane_make_points(crfu, crfd, crnd);
  
  b->vol.box.up = plane_make_points(crnu, clnu, clfu);
  b->vol.box.down = plane_make_points(clnd, crnd, crfd);
  
  b->vol.box.near = plane_make_points(clnd, clnu, crnu);
  b->vol.box.far = plane_make_points(crfd, crfu, clfu);
}

Box box_new() {
  return _volume_new();
}
Box box_init(Box b, mapVec pos, mapVec facing, mapVec size) {
  b = _volume_init(b, VolumeTypeBox, pos);
  b->vol.box.facing = facing;
  b->vol.box.size = size;
  _box_remake_planes(b);
  return b;
}
Box box_copy(Box b) {
  return _volume_raw_copy(b);
}
void box_free(Box b) {
  _volume_free(b);
}

mapVec box_position(Box b) {
  return b->position;
}
void box_set_position(Box b, mapVec pt) {
  b->position = pt;
  _box_remake_planes(b);
}
mapVec box_size(Box b) {
  return b->vol.box.size;
}
mapVec box_facing(Box b) {
  return b->vol.box.facing;
}
void box_set_facing(Box b, mapVec face) {
  b->vol.box.facing = face;
  _box_remake_planes(b);
}
int box_contains_point(Box b, mapVec pt, float radius) {
  //make sure it passes all six planes, like a frustum would
  return frustum_contains_point(b, pt, radius);
}

void box_swept_bounds(Box b, mapVec *pos, mapVec *sz) {
  mapVec bsz = b->vol.box.size;
  float maxDim = MAX(bsz.x, bsz.y);
  if(pos) {
    *pos = mapvec_subtract(b->position, mapvec_multiply_scalar((mapVec){maxDim, maxDim, bsz.z}, 0.5));
  }
  if(sz) {
    *sz = (mapVec){maxDim, maxDim, bsz.z};
  }
}

AABox aabox_new() {
  return box_new();
}
AABox aabox_init(AABox b, mapVec pos, mapVec size) {
  b = box_init(b, pos, mapvec_zero, size);
  b->type = VolumeTypeAABox;
  return b;
}
AABox aabox_copy(AABox b) {
  return _volume_raw_copy(b);
}
void aabox_free(AABox b) {
  return box_free(b);
}

mapVec aabox_position(AABox b) {
  return box_position(b);
}
void aabox_set_position(AABox b, mapVec pt) {
  box_set_position(b, pt);
}
mapVec aabox_size(AABox b) {
  return box_size(b);
}
int aabox_contains_point(AABox b, mapVec pt, float radius) {
  return box_contains_point(b, pt, radius);
}

void aabox_swept_bounds(AABox b, mapVec *pos, mapVec *sz) {
  if(pos) {
    *pos = mapvec_subtract(b->position, mapvec_multiply_scalar(b->vol.box.size, 0.5));
  }
  if(sz) {
    *sz = b->vol.box.size;
  }
}
