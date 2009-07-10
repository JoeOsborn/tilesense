#include "flagset.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

struct _flag_schema_entry {
  char *label;
  unsigned int offset;
  unsigned int bitsize;
  FlagSchema children;
}; 

FlagSchema flagschema_new() {
  return TCOD_list_new();
}
FlagSchema flagschema_init(FlagSchema fs) {
  return fs;
}
void flagschema_free(FlagSchema fs) {
  for(int i = 0; i < TCOD_list_size(fs); i++) {
    struct _flag_schema_entry *entry = TCOD_list_get(fs, i);
    flagschema_free(entry->children);
    free(entry->label);
    free(entry);
  }
  TCOD_list_delete(fs);
}
struct _flag_schema_entry *flagschema_path_find_entry(FlagSchema fs, char *key) {
  char *next = strchr(key, '.');
  if(next) { next++; }
  for(int i = 0; i < TCOD_list_size(fs); i++) {
    struct _flag_schema_entry *entry = TCOD_list_get(fs, i);
    char *check = entry->label;
    if(strncmp(key, check, next-key-1) == 0) {
      if(next) {
        return flagschema_path_find_entry(entry->children, next);
      } else {
        return entry;
      }
    }
  }
  return NULL;
}
void flagschema_recalculate_bitsize(FlagSchema fs, int index) {
  for(int i = index; i < TCOD_list_size(fs); i++) {
    struct _flag_schema_entry *entry = TCOD_list_get(fs, i);
    
    if(TCOD_list_size(entry->children) > 0) {
      entry->bitsize = 0;
      for(int j = 0; j < TCOD_list_size(entry->children); j++) {
        struct _flag_schema_entry *child = TCOD_list_get(entry->children, j);
        flagschema_recalculate_bitsize(child->children, 0);
        entry->bitsize += child->bitsize;
      }
    }
    
  }
}
void flagschema_recalculate_offsets_inner(FlagSchema fs, int index, int startOffset) {  
  for(int i = index; i < TCOD_list_size(fs); i++) {
    struct _flag_schema_entry *entry = TCOD_list_get(fs, i);
    if(i == index) {
      entry->offset = startOffset;
    }
    int childSoFar = 0;
    for(int j = 0; j < TCOD_list_size(entry->children); j++) {
      struct _flag_schema_entry *child = TCOD_list_get(entry->children, j);
      child->offset = entry->offset + childSoFar;
      flagschema_recalculate_offsets_inner(child->children, index, child->offset);
      childSoFar += child->bitsize;
    }
    if(i < (TCOD_list_size(fs)-1)) {
      struct _flag_schema_entry *next = TCOD_list_get(fs, i+1);
      next->offset = entry->offset + entry->bitsize;
    }
  }
}
void flagschema_recalculate_offsets(FlagSchema fs, int index) {
  //recalculate sizes, then offsets.
  flagschema_recalculate_bitsize(fs, index);
  int start = 0;
  if(index > 0) {
    struct _flag_schema_entry * entry = TCOD_list_get(fs, index);
    start = entry->offset;
  }
  flagschema_recalculate_offsets_inner(fs, index, start);
}
void flagschema_insert(FlagSchema fs, char *path, unsigned int bitsize) {
  //later, insert in the alphabetical middle, then update find_entry to do a binary search.
  char *next = strchr(path, '.');
  if(next) { next++; }
  if(!next) {
    struct _flag_schema_entry *entry = calloc(1, sizeof(struct _flag_schema_entry));
    entry->label = strdup(path);
    entry->bitsize = bitsize;
    entry->offset = flagschema_net_size(fs);
    entry->children = TCOD_list_new();
    TCOD_list_push(fs, entry);
  } else { 
    //there's another level from path--next; find the right one or make it if necessary
    char *firstPart = calloc(next-path, sizeof(char));
    strncpy(firstPart, path, next-path-1);
    struct _flag_schema_entry *treeTop = flagschema_path_find_entry(fs, firstPart);
    if(!treeTop) {
      treeTop = calloc(1, sizeof(struct _flag_schema_entry));
      treeTop->label = strdup(firstPart);
      treeTop->bitsize = 0;
      treeTop->offset = 0;
      treeTop->children = TCOD_list_new();
      //later, insert alphabetically
      TCOD_list_push(fs, treeTop);
    }
    free(firstPart);
    flagschema_insert(treeTop->children, next, bitsize);
  }
  int index = 0;
  flagschema_recalculate_offsets(fs, index);
}

FlagSchema flagschema_path_get_subschema(FlagSchema fs, char *path) {
  struct _flag_schema_entry *entry = flagschema_path_find_entry(fs, path);
  return entry ? entry->children : NULL;
}
FlagSchema flagschema_index_get_subschema(FlagSchema fs, int index) {
  struct _flag_schema_entry *entry = TCOD_list_get(fs, index);
  return entry ? entry->children : NULL;
}

bool flagschema_path_get_offset_size(FlagSchema fs, char *key, unsigned int *offset, unsigned int *bits) {
  struct _flag_schema_entry *entry = flagschema_path_find_entry(fs, key);
  if(entry) {
    if(offset) {
      *offset = entry->offset;
    }
    if(bits) {
      *bits = entry->bitsize;
    }
    return true;
  }
  return false;
}
bool flagschema_index_get_offset_size(FlagSchema fs, int index, unsigned int *offset, unsigned int *bits) {
  struct _flag_schema_entry *entry = TCOD_list_get(fs, index);
  if(entry) {
    if(offset) {
      *offset = entry->offset;
    }
    if(bits) {
      *bits = entry->bitsize;
    }
    return true;
  }
  return false;
}

unsigned int flagschema_net_size(FlagSchema fs) {
  if(TCOD_list_size(fs) == 0) { return 0; }
  struct _flag_schema_entry *last = TCOD_list_get(fs, TCOD_list_size(fs)-1);
  return last->offset + last->bitsize;
}

Flagset flagset_new(FlagSchema fsc) {
  return flagset_new_raw(flagschema_net_size(fsc));
}
Flagset flagset_new_raw(int bits) {
  unsigned int bytes = bits / 8;
  if(bits % 8 != 0) {
    bytes++;
  }
  return calloc(bytes,sizeof(unsigned char));
}
Flagset flagset_init(Flagset fs, FlagSchema fsc) {
  return flagset_init_raw(fs, flagschema_net_size(fsc));
}
Flagset flagset_init_raw(Flagset fs, int bits) {
  unsigned int bytes = bits / 8;
  if(bits % 8 != 0) {
    bytes++;
  }
  memset(fs, 0, bytes*sizeof(unsigned char));
  return fs;
}
void flagset_free(Flagset fs) {
  free(fs);
}
unsigned int flagset_get_path(Flagset fs, FlagSchema fsc, char *key) {
  unsigned int offset=0, size=0;
  flagschema_path_get_offset_size(fsc, key, &offset, &size);
  if(offset == -1 || size == -1) {
    return -1;
  }
  return flagset_get_raw_large(fs, offset, size);
}
unsigned int flagset_get_index(Flagset fs, FlagSchema fsc, int index) {
  unsigned int offset=0, size=0;
  flagschema_index_get_offset_size(fsc, index, &offset, &size);
  if(offset == -1 || size == -1) {
    return -1;
  }
  return flagset_get_raw_large(fs, offset, size);
}
unsigned int flagset_get_raw_large(Flagset fs, unsigned long leftOffset, int bits) {
  if(bits <= 8) {
    return flagset_get_raw(fs, leftOffset, bits);
  }
  unsigned int value = 0;
  unsigned long byteOff = leftOffset / 8;
  unsigned char bitOff = leftOffset - (byteOff * 8);
  unsigned long bitEnd = leftOffset+bits;

  unsigned char leftBits = 8 - bitOff;
  unsigned char rightBits = (bitOff+bits) % 8;
  //0011 0011 00110011 00110011 0011 0011 -- 24 bits from the middle -- left = 4, bits = 24
  //left 4 -- at 4 (leftOff), read 4 (lb), shift left by 20 (24-left-8*0)
  //next 8 -- at 4+4 (leftOff+lb), read 8, shift left by 12 (24-left-8*1)
  //next 8 -- at 4+4+8, read 8, shift left by 4  (24-left-8*2)
  //last 4 -- at 4+4+8+8, read 4 (rb), do not shift
  unsigned char bytes = 0;
  unsigned long offset = leftOffset;
  int curBits = leftBits;
  do {
    value += (unsigned int)flagset_get_raw(fs, offset, curBits) << ((bits-leftBits)-8*bytes);
    offset += curBits;
    curBits = 8;
    bytes++;
  } while(offset < (bitEnd-rightBits));
  value += (unsigned int)flagset_get_raw(fs, bitEnd-rightBits, rightBits);
  return value;
}

unsigned char flagset_get_raw(Flagset fs, unsigned long leftOffset, int bits) {
  if(bits == 0) { return 0; }
  unsigned long byteOff;
  unsigned char bitOff;
  if(leftOffset > 8) {
    byteOff = (leftOffset / 8);
    bitOff = leftOffset - (byteOff * 8);  
  } else {
    byteOff = 0;
    bitOff = leftOffset;
  }
  unsigned char *start = (unsigned char *)fs + byteOff;
  unsigned char readEnd = (bitOff + bits);
  if(readEnd <= 8) {
    unsigned char mask = (unsigned char)(0xFF << bitOff) >> (unsigned char)(bitOff + (8 - readEnd)) << (unsigned char)(8 - readEnd);
    return (unsigned char)((*start) & mask) >> (unsigned char)(8 - readEnd);
  } else {
    unsigned char rightBits = readEnd - 8;
    unsigned char leftMask = (unsigned char)(0xFF << bitOff) >> bitOff;
    unsigned char rightMask = (unsigned char)(0xFF >> (unsigned char)(8-rightBits) << (unsigned char)(8-rightBits));
    return (unsigned char)(((*start) & leftMask) << rightBits) + (unsigned char)((*((start+1)) & rightMask) >> (unsigned char)(8-rightBits));
  }
}

void flagset_set_raw_large(Flagset fs, unsigned long leftOffset, int bits, unsigned int value) {
  if(bits <= 8) {
    flagset_set_raw(fs, leftOffset, bits, (unsigned char)value);
  }
  int writeOff = leftOffset;

  int fullBytes = bits / 8;
  int leftBits = bits % 8;

  //write the first few bits
  flagset_set_raw(fs, writeOff, leftBits, (unsigned char)(value >> (bits-leftBits)));
  writeOff += leftBits;
  //then write 8 bits at a time.
  for(int i = 1; i <= fullBytes; i++) {
    flagset_set_raw(fs, writeOff, 8, (unsigned char)((value >> (bits-leftBits-i*8)) & 0xFF));
    writeOff += 8;
  }
}
void flagset_set_raw(Flagset fs, unsigned long leftOffset, int bits, unsigned char value) {
  unsigned long byteOff;
  unsigned char bitOff;
  if(leftOffset > 8) {
    byteOff = (leftOffset / 8);
    bitOff = leftOffset - (byteOff * 8);  
  } else {
    byteOff = 0;
    bitOff = leftOffset;
  }
  unsigned char *start = (unsigned char *)fs + byteOff;
  unsigned char writeEnd = (bitOff + bits);
  if(writeEnd <= 8) {
    unsigned char mask = ((unsigned char)(0xFF << bitOff) >> (unsigned char)(bitOff + (8 - writeEnd))) << (unsigned char)(8 - writeEnd);
    start[0] &= ~mask;
    start[0] |= value << (8-writeEnd);
  } else {
    unsigned char rightBits = writeEnd - 8;
    unsigned char leftMask = (unsigned char)(0xFF << bitOff) >> bitOff;
    unsigned char rightMask = ((unsigned char)(0xFF >> (unsigned char)(8-rightBits)) << (unsigned char)(8-rightBits));
    start[0] &= ~leftMask;
    start[0] |= (value >> rightBits);
    start[1] &= ~rightMask;
    start[1] |= (value << (8-rightBits));
  }
}

void flagset_set_path(Flagset fs, FlagSchema fsc, char *key, unsigned int value) {
  unsigned int offset=0, size=0;
  flagschema_path_get_offset_size(fsc, key, &offset, &size);
  if(offset == -1 || size == -1) {
    return;
  }
  return flagset_set_raw_large(fs, offset, size, value);
}
void flagset_set_index(Flagset fs, FlagSchema fsc, int index, unsigned int value) {
  unsigned int offset=0, size=0;
  flagschema_index_get_offset_size(fsc, index, &offset, &size);
  if(offset == -1 || size == -1) {
    return;
  }
  return flagset_set_raw_large(fs, offset, size, value);
}