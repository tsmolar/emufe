lib_LIBRARIES     = libsdlallegro.a
libsdlallegro_a_SOURCES = sdl_allegro.c sa_prims.c sa_sdlimage.c sa_timer.c sa_joystick.c sa_pcx.c
noinst_HEADERS=sdl_allegro.h

if USESDL
  AM_CFLAGS=-DSDL1
endif

if USESDL2
  AM_CFLAGS=-DSDL2
endif
