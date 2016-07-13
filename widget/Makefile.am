lib_LIBRARIES     = libfont.a libwidget.a libdialog.a
libfont_a_SOURCES = font.c font_legacy.c 
libdialog_a_SOURCES = dialog.c dialog-str.c
libwidget_a_SOURCES = button.c widget.c window.c keyboard.c input.c style.c
if FREETYPE
libfont_a_SOURCES+=font-ttf.c
endif
if USESDL
AM_CFLAGS=-DUSESDL -DCOMPFONT -DGFXLIB_ALLEGRO
INCLUDES=-I../sdlallegro
endif
if USEALLEGRO
AM_CFLAGS=-DCOMPFONT -DGFXLIB_ALLEGRO
endif
