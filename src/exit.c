#include "exit.h"
#include <stdlib.h>
#include <string.h>

Exit exit_new() {
  return malloc(sizeof(struct _exit));
}
Exit exit_init_map(Exit ex, mapVec pos, const char *m, mapVec end, void *context) {
  ex->position=pos;
  ex->destMap = strdup(m);
  ex->endPos = end;
  ex->context = context;
  return ex;
}

void exit_free(Exit ex) {
  free(ex->destMap);
  free(ex);
}
char *exit_dest_map(Exit ex) {
  return ex->destMap;
}
mapVec exit_pos(Exit ex) {
  return ex->position;
}
mapVec exit_end(Exit ex) {
  return ex->endPos;
}
void *exit_context(Exit ex) {
  return ex->context;
}