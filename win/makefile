SRCDIR=./src
INCDIR=./include
CC=mingw32-gcc
CPP=mingw32-g++
FLAGS=-ggdb
CFLAGS=$(FLAGS) -std=gnu9x -Wall -I$(INCDIR) $(SDL_FLAGS) $(ZLIB_FLAGS) $(PNG_FLAGS) $(TCOD_FLAGS) -fno-strict-aliasing
.SUFFIXES: .dll .a .o .h .c .hpp .cpp
TMPDIR=/tmp/tilesense

C_OBJS=\
  $(TMPDIR)/exit.o \
  $(TMPDIR)/flagset.o \
  $(TMPDIR)/geom.o \
  $(TMPDIR)/light.o \
  $(TMPDIR)/map.o \
  $(TMPDIR)/object.o \
  $(TMPDIR)/objectmap.o \
  $(TMPDIR)/sensor.o \
  $(TMPDIR)/stimulus.o \
  $(TMPDIR)/tile.o \
  $(TMPDIR)/volume.o

HEADERS=\
  $(INCDIR)/exit.h \
  $(INCDIR)/flagset.h \
  $(INCDIR)/geom.h \
  $(INCDIR)/light.h \
  $(INCDIR)/map.h \
  $(INCDIR)/object.h \
  $(INCDIR)/objectmap.h \
  $(INCDIR)/sensor.h \
  $(INCDIR)/stimulus.h \
  $(INCDIR)/tile.h \
  $(INCDIR)/tslist.h \
  $(INCDIR)/volume.h

TEST_OBJS=\
  $(TMPDIR)/test/test.o

$(TMPDIR) : 
	mkdir -p $@

$(TMPDIR)/test : 
	mkdir -p $@

$(TMPDIR)/%.o : $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -std=gnu9x -g -o $@ -c $<

$(TMPDIR)/test/%.o : $(SRCDIR)/../%.c
	$(CC) $(CFLAGS) -std=gnu9x -g -o $@ -c $<

#dependencies
# SDL
SDL_DIR=libtcod/dependencies/SDL-1.2.12
SDL_FLAGS=-I $(SDL_DIR)/include
SDL_LIBS=-L $(SDL_DIR)/lib/mingw -lSDL.dll

# ZLIB
ZLIB_DIR=libtcod/dependencies/zlib-1.2.3
ZLIB_FLAGS=-I $(ZLIB_DIR)/include
ZLIB_LIBS=$(ZLIB_DIR)/lib/mingw/libz.a

# PNG
PNG_DIR=libtcod/dependencies/libpng-1.2.31
PNG_FLAGS=-I $(PNG_DIR)/include
PNG_LIBS=$(PNG_DIR)/lib/mingw/libpng.a

# OPENGL
OPENGL_LIB=-L./libtcod -lopengl32

# TCOD
TCOD_DIR=libtcod
TCOD_FLAGS=-I $(TCOD_DIR)/include -I $(TCOD_DIR)/include/gui
TCOD_LIBS=-L./libtcod/lib -ltcod-mingw-debug

$(C_OBJS) : $(HEADERS)

libtilesense.a : $(TMPDIR) $(C_OBJS)
	$(CPP) -mwindows -o $@ -shared -Wl,--out-implib,libtilesense.a $(C_OBJS) $(ZLIB_LIBS) $(PNG_LIBS) $(SDL_LIBS) $(OPENGL_LIB) $(TCOD_LIBS)

$(TEST_OBJS) : $(HEADERS)

test : libtilesense.a $(TMPDIR)/test $(TEST_OBJS)
	$(CPP) $(CFLAGS) -o $@ $(TEST_OBJS) $(ZLIB_LIBS) $(PNG_LIBS) $(SDL_LIBS) $(OPENGL_LIB) $(TCOD_LIBS) libtilesense.a

clean :
	rm -f $(C_OBJS) $(TEST_OBJS) test libtilesense.a
	rm -rf test.dSYM
