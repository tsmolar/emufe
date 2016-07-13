#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.50)
AC_INIT(emufe, 3.1.4, asmolar@sf.net)
AC_CONFIG_SRCDIR([src/emufe.c])
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER([config.h])

# Checks for programs.
#AC_PROG_LIBTOOL
AC_PROG_CC
AC_PROG_RANLIB
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

AC_ARG_ENABLE(zaurus,
[  --enable-zaurus  Enable Optimizations for Zaurus and other small devices
],[AC_DEFINE([ZAURUS],[1],[Define if using Zaurus])
   use_zaurus=yes])

# Why this mess?   If ww is in the PATH, then it won't be able to
# locate the datadir, so INSTPREFIX will be where to look
if test "x$use_zaurus" = "xyes" ; then
  AC_DEFINE_UNQUOTED([INSTPREFIX],["/home/QtPalmtop"],[Help find data dir])
elif test "x$prefix" = "xNONE" ; then
    AC_DEFINE_UNQUOTED([INSTPREFIX],["/usr/local"],[Help find data dir])
else
    AC_DEFINE_UNQUOTED([INSTPREFIX],"${prefix}",[Help find data dir])
fi

# Checks for libraries.
LIBCURL_CHECK_CONFIG([no], [7.13.2], [haveCurl=yes], [haveCurl=no])
if test "x$haveCurl" = "xyes"
then
  AM_CONDITIONAL(HSNET,true)
  LIBS="$LIBS $LIBCURL"
  rem LIBS="$LIBS -L/opt/libcurl-7.16.3/lib -lcurl"
else
  AM_CONDITIONAL(HSNET,false)
fi

# Freetype
AC_CHECK_FT2([2.1.0], [haveFT=yes], [haveFT=no])
if test "x$haveFT" = "xyes"
then
  AM_CONDITIONAL(FREETYPE,true)
  AC_DEFINE([USE_FREETYPE],[1],[Define if using FreeType])
  CFLAGS="$CFLAGS $FT2_CFLAGS"
  LIBS="$LIBS $FT2_LIBS"
else
  AM_CONDITIONAL(FREETYPE,false)
fi


dnl Want Allegro or SDL, but not both!  Allegro takes presidence
dnl SDLAllegro may eventually function as a drop in replacement, meaning
dnl we will only need to check for allegro (and sdlallegro will integrate sdl)
AM_PATH_ALLEGRO(4.1.0)
if test "$allegro_CFLAGS"
then
AM_CONDITIONAL(USESDL,false)
AM_CONDITIONAL(USEALLEGRO,true)
else
AM_CONDITIONAL(USEALLEGRO,false)
dnl Check for SDL
SDL_VERSION=1.2.5
AM_PATH_SDL($SDL_VERSION,
            AM_CONDITIONAL(USESDL,true),
	    AM_CONDITIONAL(USESDL,false)
)
CFLAGS="$CFLAGS $SDL_CFLAGS"
LIBS="$LIBS $SDL_LIBS"
AC_CHECK_LIB(SDL_image,IMG_Load)
AC_CHECK_LIB(SDL_mixer,Mix_Linked_Version)
fi
# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CONFIG_FILES([Makefile
                 src/Makefile
		 widget/Makefile
		 sdlallegro/Makefile])
AC_OUTPUT
