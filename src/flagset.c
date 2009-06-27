#include "flagset.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

//Flagset flagset_new(FlagSchema fsc);
Flagset flagset_new_raw(int bits) {
  return calloc(ceilf(bits/8.0), sizeof(unsigned char));
}
//Flagset flagset_init(Flagset fs, FlagSchema fsc);
Flagset flagset_init_raw(Flagset fs, int bits) {
  memset(fs, 0, ceilf(bits/8.0));
  return fs;
}
void flagset_free(Flagset fs) {
  free(fs);
}
//int flagset_get_path(Flagset fs, FlagSchema fsc, char *key, void *out);
//int flagset_get_index(Flagset fs, FlagSchema fsc, int index, void *out);
unsigned int flagset_get_raw_large(Flagset fs, unsigned long leftOffset, int bits) {
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

//void flagset_set_path(Flagset fs, FlagSchema fsc, char *key, void *value);
//void flagset_set_index(Flagset fs, FlagSchema fsc, int index, void *value);
void flagset_set_raw_large(Flagset fs, unsigned long leftOffset, int bits, unsigned int value) {
  if(bits <= 8) {
    flagset_set_raw(fs, leftOffset, bits, (unsigned char)value);
  }
  //the current byte of value
  unsigned char curValue = 0;
  //where in value we start reading (from the left)
  int valOff = (sizeof(unsigned int)*4) - bits;
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
