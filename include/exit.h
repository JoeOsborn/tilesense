#ifndef _EXIT_H
#define _EXIT_H

#include "geom.h"

//add on an extra "flags" Flagset for user purposes?
//should exit even be a part of libtilesense?  Probably, for inter-room stuff...
//consider how to move forward on this.
//defer it for now.

typedef enum {
  RoomExit,
  ZoneExit
  } exitType;

struct _exit {
  exitType type;
  mapVec position;
  char *destZone;
  char *destRoom;
  mapVec endPos;
  };

typedef struct _exit * Exit;

Exit exit_new();
Exit exit_init_room(Exit ex, mapVec pos, const char *r, mapVec end);
Exit exit_init_zone(Exit ex, mapVec pos, const char *z, const char *r, mapVec end);
void exit_free(Exit ex);
exitType exit_type(Exit ex);
char *exit_dest_room(Exit ex);
char *exit_dest_zone(Exit ex);
mapVec exit_pos(Exit ex);
mapVec exit_end(Exit ex);

#endif