#ifndef _FLAGSET_H
#define _FLAGSET_H

#include <libtcod.h>

//describes how a number of size 1-32 bits is laid out within a bitfield of arbitrary size.
//the whole tree describes an entire such bitfield and set of such numbers.
//a Collision schema might describe a set of collision flags and parameters;
//a Light schema might describe a light's behavior.
//The goals of doing it with schema + bitfield rather than with a bitfield alone or with
//struct syntax are:
//first, it makes transmission over the network easier and less system-dependent.
//it also makes it trivial to push these fields into and out of Erlang and other environments.
//second, it allows developers and content creators to be a little explicit in their mask definitions,
//while still providing flexibility for certain entities to have more or fewer bits in their schemae.
//for example, if there is only one tile that is passable when the entity is slippery, it is unnecessary
//to have slippery flags on every tile definition.  (in practice, it may be fastest to just include the rare
//flag everywhere and check without using a schema search, or share the schema among all tiles.  collision
//occurs outside of libtilesense, so it's up to the application to decide.)
//third, it removes a lot of duplicate code for masking, accessing, etc, and provides 
//minimal processing overhead and one pointer of memory overhead in the case that does not use schemae.

typedef TCOD_list_t FlagSchema;

FlagSchema flagschema_new();
//need a nice initializer to create a flagschema linked list from a string specification
FlagSchema flagschema_init(FlagSchema fs);

void flagschema_free(FlagSchema fs);

unsigned int flagschema_net_size(FlagSchema fs);
//later, make sure this stuff is stored alphabetically?  or not...
void flagschema_insert(FlagSchema schema, char *path, unsigned int bitsize);

TCOD_list_t flagschema_get_labels(FlagSchema schema);

FlagSchema flagschema_path_get_subschema(FlagSchema fs, char *path);
FlagSchema flagschema_index_get_subschema(FlagSchema fs, int index);

bool flagschema_path_get_offset_size(FlagSchema fs, char *path, unsigned int *offset, unsigned int *bits);
bool flagschema_index_get_offset_size(FlagSchema fs, int index, unsigned int *offset, unsigned int *bits);


typedef unsigned char * Flagset;

Flagset flagset_new(FlagSchema fsc);
Flagset flagset_new_raw(int bits);
Flagset flagset_init(Flagset fs, FlagSchema fsc);
Flagset flagset_init_raw(Flagset fs, int bits);
void flagset_free(Flagset fs);

//goes over each top-level entry in fsc. 
//Returns true if any are equal between the two flagsets.
bool flagset_any_match(Flagset f1, Flagset f2, FlagSchema fsc);

//big-endian multi-byte values
unsigned int flagset_get_path(Flagset fs, FlagSchema fsc, char *key);
unsigned int flagset_get_index(Flagset fs, FlagSchema fsc, int index);
unsigned int flagset_get_raw_large(Flagset fs, unsigned long leftOffset, int bits);
unsigned char flagset_get_raw(Flagset fs, unsigned long leftOffset, int bits);
void flagset_set_path(Flagset fs, FlagSchema fsc, char *key, unsigned int value);
void flagset_set_index(Flagset fs, FlagSchema fsc, int index, unsigned int value);
void flagset_set_raw_large(Flagset fs, unsigned long leftOffset, int bits, unsigned int value);
void flagset_set_raw(Flagset fs, unsigned long leftOffset, int bits, unsigned char value);

#endif
