#ifndef _FLAGSET_H
#define _FLAGSET_H

//flagset will not store values larger than one byte.
//the values taken together may be larger, but no individual value will be larger.
//this is to simplify the external interface.  this is to avoid byte-order issues.
//for larger values, clients must deal with byte order issues themselves externally.

typedef void * Flagset;

// struct _flag_schema {
//   
// };
// 
// typedef struct _flag_schema * FlagSchema;

//Flagset flagset_new(FlagSchema fsc);
Flagset flagset_new_raw(int bits);
//Flagset flagset_init(Flagset fs, FlagSchema fsc);
Flagset flagset_init_raw(Flagset fs, int bits);
void flagset_free(Flagset fs);
//unsigned char flagset_get_path(Flagset fs, FlagSchema fsc, char *key);
//unsigned char flagset_get_index(Flagset fs, FlagSchema fsc, int index);
//big-endian multi-byte values
unsigned int flagset_get_raw_large(Flagset fs, unsigned long leftOffset, int bits);
unsigned char flagset_get_raw(Flagset fs, unsigned long leftOffset, int bits);
//void flagset_set_path(Flagset fs, FlagSchema fsc, char *key, unsigned char value);
//void flagset_set_index(Flagset fs, FlagSchema fsc, int index, unsigned char value);
void flagset_set_raw_large(Flagset fs, unsigned long leftOffset, int bits, unsigned int value);
void flagset_set_raw(Flagset fs, unsigned long leftOffset, int bits, unsigned char value);

#endif
