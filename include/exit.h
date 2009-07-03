#ifndef _EXIT_H
#define _EXIT_H

#include "geom.h"

//add on an extra "flags" Flagset for user purposes?
//should exit even be a part of libtilesense?  Probably, for inter-room stuff...
//consider how to move forward on this.
//defer it for now.

struct _exit {
  mapVec position;
  char *destMap;
  mapVec endPos;
  
  void *context;
  };

typedef struct _exit * Exit;

Exit exit_new();
Exit exit_init_room(Exit ex, mapVec pos, const char *r, mapVec end, void *context);
void exit_free(Exit ex);
char *exit_dest_map(Exit ex);
mapVec exit_pos(Exit ex);
mapVec exit_end(Exit ex);
void *exit_context(Exit ex);

#endif