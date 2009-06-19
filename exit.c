#include "exit.h"
#include <stdlib.h>
#include <string.h>

Exit exit_new() {
  return malloc(sizeof(struct _exit));
}
Exit exit_init_room(Exit ex, mapVec pos, const char *r, mapVec end) {
  ex->type=RoomExit;
  ex->position=pos;
  ex->destRoom = malloc(strlen(r)*sizeof(char));
  strcpy(ex->destRoom, r);
  ex->destZone = NULL;
  ex->endPos = end;
  return ex;
}
Exit exit_init_zone(Exit ex, mapVec pos, const char *z, const char *r, mapVec end) {
  Exit iex = exit_init_room(ex, pos, r, end);
  iex->type=ZoneExit;
  iex->destZone = malloc(strlen(z)*sizeof(char));
  strcpy(iex->destZone, z);
  return iex;
}

void exit_free(Exit ex) {
  free(ex->destRoom);
  free(ex->destZone);
  free(ex);
}
exitType exit_type(Exit ex) {
  return ex->type;
}
char *exit_dest_room(Exit ex) {
  return ex->destRoom;
}
char *exit_dest_zone(Exit ex) {
  return ex->destZone;
}
mapVec exit_pos(Exit ex) {
  return ex->position;
}
mapVec exit_end(Exit ex) {
  return ex->endPos;
}
