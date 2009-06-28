#include "flagset.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

FlagSchema flagschema_new() {
  return malloc(sizeof(struct _flag_schema));
}
FlagSchema flagschema_init(FlagSchema fs, char *label, unsigned int bitsize) {
  fs->label = malloc(strlen(label) * sizeof(char) + 1);
  strcpy(fs->label, label);
  fs->bitsize = bitsize;
  fs->offset = 0;
  fs->next = NULL;
  return fs;
}
void flagschema_free(FlagSchema fs) {
  if(fs->next) {
    flagschema_free(fs->next);
  }
  free(fs->label);
  free(fs);
}

FlagSchema flagschema_get_last(FlagSchema first) {
  if(first->next) {
    return flagschema_get_last(first->next);
  }
  return first;
}
void flagschema_recalculate_offsets(FlagSchema first) {
  FlagSchema next = first->next;
  if(next) {
    next->offset = first->offset + first->bitsize;
    flagschema_recalculate_offsets(next);
  }
}
void flagschema_append(FlagSchema first, FlagSchema last) {
  flagschema_insert(flagschema_get_last(first), last);
}
void flagschema_insert(FlagSchema first, FlagSchema next) {
  FlagSchema temp = first->next;
  first->next = next;
  if(temp) {
    next->next = temp;
  }
  flagschema_recalculate_offsets(first);
}

void flagschema_label_get_offset_size(FlagSchema fs, char *key, unsigned int *offset, unsigned int *bits) {
  if(strcmp(key, fs->label) != 0) {
    if(fs->next) {
      flagschema_label_get_offset_size(fs->next, key, offset, bits);
    } else {
      if(offset) {
        *offset = -1;
      }
      if(bits) {
        *bits = -1;
      }
    }
    return;
  }
  if(offset) {
    *offset = fs->offset;
  }
  if(bits) {
    *bits = fs->bitsize;
  }
}
void flagschema_index_get_offset_size(FlagSchema fs, int index, unsigned int *offset, unsigned int *bits) {
  if(index > 0) {
    if(fs->next) {
      flagschema_index_get_offset_size(fs->next, index-1, offset, bits);
    } else {
      if(offset) {
        *offset = -1;
      }
      if(bits) {
        *bits = -1;
      }
    }
    return;
  }
  if(offset) {
    *offset = fs->offset;
  }
  if(bits) {
    *bits = fs->bitsize;
  }
}

unsigned int flagschema_net_size(FlagSchema fs) {
  FlagSchema last = flagschema_get_last(fs);
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
unsigned int flagset_get_label(Flagset fs, FlagSchema fsc, char *key) {
  unsigned int offset=0, size=0;
  flagschema_label_get_offset_size(fsc, key, &offset, &size);
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

void flagset_set_label(Flagset fs, FlagSchema fsc, char *key, unsigned int value) {
  unsigned int offset=0, size=0;
  flagschema_label_get_offset_size(fsc, key, &offset, &size);
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