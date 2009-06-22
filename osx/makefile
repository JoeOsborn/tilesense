SRCDIR=./src
INCDIR=./include
CC=gcc
FLAGS=-ggdb -std=c99
.SUFFIXES: .o .h .c

C_OBJS=\
  $(SRCDIR)/bresenham3_c.o \
  $(SRCDIR)/exit.o \
  $(SRCDIR)/geom.o \
  $(SRCDIR)/light.o \
  $(SRCDIR)/map.o \
  $(SRCDIR)/object.o \
  $(SRCDIR)/sensor.o \
  $(SRCDIR)/stimulus.o \
  $(SRCDIR)/tile.o \
  $(SRCDIR)/volume.o

HEADERS=\
  $(INCDIR)/bresenham3.h \
  $(INCDIR)/exit.h \
  $(INCDIR)/geom.h \
  $(INCDIR)/light.h \
  $(INCDIR)/map.h \
  $(INCDIR)/object.h \
  $(INCDIR)/sensor.h \
  $(INCDIR)/stimulus.h \
  $(INCDIR)/tile.h \
  $(INCDIR)/volume.h

TEST_OBJS=\
  test.o

#dependencies
# SDL
SDL_DIR=libtcod/dependencies/SDL-1.2.12
SDL_FLAGS=-D_GNU_SOURCE=1 -D_REENTRANT -I $(SDL_DIR)/include
SDL_LIBS=$(SDL_DIR)/lib/osx/libSDL.a $(SDL_DIR)/lib/osx/libSDLmain.a -framework Carbon -framework Quartz -framework OpenGL -framework AppKit -framework IOKit -framework AudioUnit -framework Quicktime -framework Cocoa

# ZLIB
ZLIB_DIR=libtcod/dependencies/zlib-1.2.3
ZLIB_FLAGS=-I $(ZLIB_DIR)/include
ZLIB_LIBS=$(ZLIB_DIR)/lib/osx/libz.a

# PNG
PNG_DIR=libtcod/dependencies/libpng-1.2.31
PNG_FLAGS=-I $(PNG_DIR)/include
PNG_LIBS=$(PNG_DIR)/lib/osx/libpng.a

# TCOD
TCOD_DIR=libtcod
TCOD_FLAGS=-I $(TCOD_DIR)/include -I $(TCOD_DIR)/include/gui
TCOD_LIBS=$(TCOD_DIR)/libtcod.a

CFLAGS=$(FLAGS) -Wall -I$(INCDIR) $(SDL_FLAGS) $(ZLIB_FLAGS) $(PNG_FLAGS) $(TCOD_FLAGS) -fno-strict-aliasing

$(C_OBJS) : $(HEADERS)

libtilesense.a : $(C_OBJS)
	libtool -static -o $@ $(C_OBJS) $(ZLIB_LIBS) $(PNG_LIBS) $(SDL_LIBS) $(TCOD_LIBS)

$(TEST_OBJS) : $(HEADERS)

test : libtilesense.a test.o
	$(CC) $(CFLAGS) -o $@ $(TEST_OBJS) $(ZLIB_LIBS) $(PNG_LIBS) $(SDL_LIBS) $(TCOD_LIBS) libtilesense.a

clean :
	rm -f $(C_OBJS) $(TEST_OBJS) test libtilesense.a
	rm -rf test.dSYM
