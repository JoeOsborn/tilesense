#ifndef _TS_LIST_H
#define _TS_LIST_H

#define TS_LIST_CLEAR_AND_DELETE(_list, _type) do { \
  for(int i = 0; i < TCOD_list_size(_list); i++) {  \
    _type ## _free(TCOD_list_get(_list, i));        \
  }                                                 \
  TCOD_list_delete(_list);                          \
  _list = NULL;                                     \
} while(0)

#define TS_LIST_FOREACH(list, op) do {                  \
  void *each = NULL;                                    \
  for(int __i = 0; __i < TCOD_list_size(list); __i++) { \
    each = TCOD_list_get(list, __i);                    \
    op;                                                 \
  }                                                     \
} while(0)                                              \

#endif