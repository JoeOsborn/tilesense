#include "objectmap.h"
#include <limits.h>

ObjectMap objectmap_new() {
  return calloc(1, sizeof(struct _object_map));
}
ObjectMap objectmap_init(ObjectMap om, mapVec sz) {
  om->size = sz;
  om->indexSize = sz.x*sz.y*sz.z;
  om->entryCount = 50;
  om->activeEntries = 0;
  om->entries = calloc(om->entryCount, sizeof(_omentry));
  for(int i = 0; i < om->entryCount; i++) {
    om->entries[i].index=INT_MAX;
    om->entries[i].objects = TCOD_list_new();
  }
  return om;
}
void objectmap_free(ObjectMap om) {
  for(int i = 0; i < om->entryCount; i++) {
    TCOD_list_delete(om->entries[i].objects);
  }
  free(om->entries);
  free(om);
}

int objectmap_index(ObjectMap om, mapVec pos) {
  return tile_index(pos.x, pos.y, pos.z, om->size, mapvec_zero, om->size);
}

int objectmap_find_entry_between(ObjectMap om, int start, int end, int index) {
  if(om->activeEntries == 0) { return 0; }
  if(start == end) {
    //if start >= idx, return start
    //else, return start+1
    return (om->entries[start].index >= index) ? start : start+1;
  }
  if(start <= (end-1)) {
    if (om->entries[start].index >= index) { return start; }
    if (om->entries[end].index >= index) { return end; }
    return end + 1;
  }
  //divide and conquer
  //find the entry, or the index at which the entry should be inserted
  int mid = (end - start) / 2;
  //is the index at mid more or less than index?
  if(om->entries[mid].index > index) {
    return objectmap_find_entry_between(om, start, mid, index);
  } else if(om->entries[mid].index < index) {
    return objectmap_find_entry_between(om, mid, end, index);
  }
  return mid;
}

//if existingOnly is true, will return the index of the entry, or -1 if not found
//if existingOnly is false, will return the index where the entry should be inserted.
int objectmap_find_entry(ObjectMap om, int index, bool existingOnly) {
  int idx = objectmap_find_entry_between(om, 0, om->activeEntries, index);
  if(idx >= om->activeEntries) {
    return existingOnly ? -1 : idx;
  }
  if(om->entries[idx].index == index) {
    return idx;
  } else {
    return existingOnly ? -1 : idx;
  }
}

TCOD_list_t objectmap_get(ObjectMap om, mapVec pos) {
  int idx = objectmap_find_entry(om, objectmap_index(om, pos), true);
  if(idx == -1) { return NULL; }
  return om->entries[idx].objects;
}

void objectmap_remove_entry(ObjectMap om, int idx) {
  TCOD_list_clear(om->entries[idx].objects);
  om->entries[idx].index = INT_MAX;
  //now, shuffle around entries to move srcIdx..activeEntries back one space
  TCOD_list_t old = om->entries[idx].objects;
  for(int i = idx; i < om->activeEntries; i++) {
    om->entries[i] = om->entries[i+1];
  }
  //both the first and last+1th list here are guaranteed to be empty.
  om->entries[om->activeEntries].objects = old; 
  om->activeEntries--;
}

void objectmap_resize_if_necessary(ObjectMap om) {
  if(om->activeEntries == om->entryCount) {
    om->entries = realloc(om->entries, om->entryCount*2*sizeof(_omentry));
    om->entryCount *= 2;
    for(int i = om->activeEntries; i < om->entryCount; i++) {
      om->entries[i].objects = TCOD_list_new();
      om->entries[i].index = INT_MAX;
    }
  }
}

void objectmap_insert_entry(ObjectMap om, int idx, int objIndex) {
  //move a spare entry from the end of the list to the right index, and update idx. increment om->activeEntries.
  //a. shuffle forward everything from idx..activeEntries
  
  //avoid leaking or aliasing any object lists (this last one is guaranteed to be empty, so we're safe in that respect)
  TCOD_list_t old = om->entries[om->activeEntries].objects;
  for(int i = om->activeEntries; i > idx; i--) {
    om->entries[i] = om->entries[i-1];
  }
  om->entries[idx].objects = old;

  om->activeEntries++;
  om->entries[idx].index = objIndex;

  //if there aren't any spare entries, realloc and extend the entries list. increment om->entryCount accordingly.
  objectmap_resize_if_necessary(om);
}

void objectmap_insert_at(ObjectMap om, Object o, mapVec pos) {
  int objIndex = objectmap_index(om, pos);
  int idx = objectmap_find_entry(om, objIndex, false);
  if((idx >= om->activeEntries) || (om->entries[idx].index != objIndex)) {
    objectmap_insert_entry(om, idx, objIndex);
  }
  TCOD_list_push(om->entries[idx].objects, o);
}

void objectmap_remove_at(ObjectMap om, Object o, mapVec pos) {
  int objIndex = objectmap_index(om, object_position(o));
  int idx = objectmap_find_entry(om, objIndex, true);
  if(idx == -1) { return; }
  if(TCOD_list_size(om->entries[idx].objects) == 1) {
    //if the list is empty, remove this entry and shunt it to the reserve space at the end of the array
    objectmap_remove_entry(om, idx);
  } else {
    TCOD_list_remove(om->entries[idx].objects, o);
  }
}

void objectmap_insert(ObjectMap om, Object o) {
  objectmap_insert_at(om, o, object_position(o));
}

void objectmap_remove(ObjectMap om, Object o) {
  objectmap_remove_at(om, o, object_position(o));
}

void objectmap_move(ObjectMap om, Object o, mapVec delta) {
  #warning later optimization
  //if the object is alone in the old idx and the new idx does not exist, just shuffle around the array.
  objectmap_remove(om, o);
  objectmap_insert_at(om, o, mapvec_add(object_position(o), delta));
}
