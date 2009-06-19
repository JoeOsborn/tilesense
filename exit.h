#ifndef _EXIT_H
#define _EXIT_H

#include "geom.h"

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