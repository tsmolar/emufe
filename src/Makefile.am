#gfxcdir=$(datadir)/ballooner-lander/gfx/common
#gfxc_DATA=../share/ballooner-lander/gfx/common/*
#snddir=$(datadir)/ballooner-lander/sounds
#snd_DATA=../share/ballooner-lander/sounds/*
#hsdir=$(datadir)/ballooner-lander/hs
#hs_DATA=../share/ballooner-lander/hs/scores.dat
#icondir=$(datadir)/ballooner-lander/icons
#icon_DATA=../share/ballooner-lander/icons/*
#docdir=$(datadir)/ballooner-lander/docs
#doc_DATA=../doc/*

#if GFXPNG
#gfxdir=$(datadir)/ballooner-lander/gfx/png
#gfx_DATA=../share/ballooner-lander/gfx/png/*
#endif

#if GFXPCX
#gfxdir=$(datadir)/ballooner-lander/gfx/pcx
#gfx_DATA=../share/ballooner-lander/gfx/pcx/*
#endif

#if LENCRYPT
#lvlext=ldx
#else
#lvlext=ldt
#endif

#if LVLPNG
#gfxldir=$(datadir)/ballooner-lander/gfx/lvlng
#gfxl_DATA=../share/ballooner-lander/gfx/lvlng/*
#leveldir=$(datadir)/ballooner-lander/levels
#level_DATA=../share/ballooner-lander/levels/level*_ng.$(lvlext)
#endif

#if LVLPCX
#gfxldir=$(datadir)/ballooner-lander/gfx/lvlcx
#gfxl_DATA=../share/ballooner-lander/gfx/lvlcx/*
#leveldir=$(datadir)/ballooner-lander/levels
#level_DATA=../share/ballooner-lander/levels/level*_cx.$(lvlext)
#endif

#if LVLLR
#gfxldir=$(datadir)/ballooner-lander/gfx/lvllr
#gfxl_DATA=../share/ballooner-lander/gfx/lvllr/*
#leveldir=$(datadir)/ballooner-lander/levels
#level_DATA=../share/ballooner-lander/levels/level*_lr.$(lvlext)
#endif

bin_PROGRAMS=emufe splash
emufe_SOURCES=emufe.c rcfile.c dfilepath.c modules.c fileio.c setup.c
emufe_LDADD=../widget/libfont.a ../widget/libwidget.a
splash_SOURCES=splash.c dfilepath.c

if USESDL
#bl_SOURCES+=gfx.c
AM_CFLAGS=-DUSESDL
#AM_CFLAGS=-DUSESDL
emufe_LDADD+=../sdlallegro/libsdlallegro.a ../widget/libfont.a
splash_LDADD=../sdlallegro/libsdlallegro.a ../widget/libfont.a
INCLUDES = -I../sdlallegro -I../widget
endif

if USESDL2
AM_CFLAGS=-DUSESDL
emufe_LDADD+=../sdlallegro/libsdlallegro.a ../widget/libfont.a
splash_LDADD=../sdlallegro/libsdlallegro.a ../widget/libfont.a
INCLUDES = -I../sdlallegro -I../widget
endif

if USEALLEGRO
#bl_SOURCES+=gfx.c
AM_CFLAGS=-DUSEALLEGRO -DUSEGFX $(allegro_CFLAGS)
AM_CXXFLAGS=-DUSEALLEGRO -DUSEGFX $(allegro_CFLAGS)
emufe_LDADD+=../widget/libfont.a ../widget/libwidget.a $(allegro_LIBS)
splash_LDADD=../widget/libfont.a ../widget/libwidget.a $(allegro_LIBS)
INCLUDES = -I../widget
endif
