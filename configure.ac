#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.50)
AC_INIT(emufe, 4.2.0, asmolar@sf.net)
AC_CONFIG_SRCDIR([src/emufe.c])
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER([config.h])

# Checks for programs.
#AC_PROG_LIBTOOL
AC_PROG_CC
AC_PROG_RANLIB
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

#AC_ARG_ENABLE(zaurus,
#[  --enable-zaurus  Enable Optimizations for Zaurus and other small devices
#],[AC_DEFINE([ZAURUS],[1],[Define if using Zaurus])
#   use_zaurus=yes])

#AC_ARG_ENABLE(amdgfx,
#[  --enable-amdgfx  Use slower but safer screen update routine
#],[AC_DEFINE([AMDGFX],[1],[Define if using AMD Graphics])
#   use_amdgfx=yes])

#if test "x$use_amdgfx" = "xyes" ; then
#  AM_CONDITIONAL(AMDGFX,false)
#fi

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


# Checks for pkg-config
AC_PATH_PROG([PKGCONF], [pkg-config], [], [$PATH:/usr/bin:/usr/local/bin:/opt/bin:/opt/homebrew/bin])

# If pkg-config is available, use it to find needed libraries
if test -n "${PKGCONF}"; then
    # Freetype via pkg-config
    AC_MSG_CHECKING([whether pkg-config could find freetype2])
    if "${PKGCONF}" --exists freetype2; then
        AC_MSG_RESULT([yes])
        AC_DEFINE([USE_FREETYPE],[1],[Define if using FreeType])
        FT2_CFLAGS=`"${PKGCONF}" --cflags freetype2`
        FT2_LIBS=`"${PKGCONF}" --libs freetype2`
        AM_CONDITIONAL(FREETYPE,true)
	CFLAGS="$CFLAGS $FT2_CFLAGS"
	LIBS="$LIBS $FT2_LIBS"
    else
        AC_MSG_RESULT([no])
        AM_CONDITIONAL(FREETYPE,false)
    fi
fi

## Freetype
#AC_CHECK_FT2([2.1.0], [haveFT=yes], [haveFT=no])
#if test "$haveFT" = "yes"
#then
#  AM_CONDITIONAL(FREETYPE,true)
#  AC_DEFINE([USE_FREETYPE],[1],[Define if using FreeType])
#  CFLAGS="$CFLAGS $FT2_CFLAGS"
#  LIBS="$LIBS $FT2_LIBS"
#else
#  AM_CONDITIONAL(FREETYPE,false)
#fi

gfx_lib=null
dnl Need one of SDL2, SDL or allegro, in order of presidence
dnl Check for SDL2
#AM_PATH_SDL2(2.0.0,AM_CONDITIONAL(USESDL2,true),AM_CONDITIONAL(USESDL2,false))
AM_CONDITIONAL(USESDL2,false)
AM_CONDITIONAL(USESDL,false)
AM_CONDITIONAL(USEALLEGRO,false)


AC_MSG_NOTICE([SDL2 check])
AM_PATH_SDL2(2.0.0,gfx_lib=sdl2,:)
if [[ "$gfx_lib" = sdl2 ]]; then
  AC_MSG_NOTICE([Choosing SDL2 Library])
  AM_CONDITIONAL(USESDL2,true)
  CFLAGS="$CFLAGS $SDL_CFLAGS"
  LIBS="$LIBS $SDL_LIBS"
  AC_CHECK_LIB(SDL2_image,IMG_Load)
  AC_CHECK_LIB(SDL2_mixer,Mix_Linked_Version)
  AC_CHECK_LIB(SDL2_ttf,TTF_Init)
else
  AC_MSG_NOTICE([SDL1 check])
  AM_PATH_SDL(1.2.5,gfx_lib=sdl1,:)
  if [[ "$gfx_lib" = sdl1 ]]; then
    AC_MSG_NOTICE([Choosing SDL1 Library])
    AM_CONDITIONAL(USESDL,true)
    CFLAGS="$CFLAGS $SDL_CFLAGS"
    LIBS="$LIBS $SDL_LIBS"
    AC_CHECK_LIB(SDL_image,IMG_Load)
    AC_CHECK_LIB(SDL_mixer,Mix_Linked_Version)
  else
    AM_PATH_ALLEGRO(4.1.0,gfx_lib=allegro,:)
    if [[ "$gfx_lib" = allegro ]]; then
      AC_MSG_NOTICE([Choosing Allegro Library])
      AM_CONDITIONAL(USEALLEGRO,true)
    else
      if [[ "$gfx_lib" = null ]]; then
        AC_MSG_ERROR([* * * No compatible graphics library found!])
      fi
    fi
  fi
fi

#dnl Need one of SDL2, SDL or allegro, in order of presidence
#dnl Check for SDL2
#AM_PATH_SDL2(2.0.0,AM_CONDITIONAL(USESDL2,true),AM_CONDITIONAL(USESDL2,false))
#if test "$SDL_CFLAGS"
#then
#  AM_CONDITIONAL(USESDL,false)
#  AM_CONDITIONAL(USEALLEGRO,false)
#else
#  dnl Check for SDL1
#  AM_PATH_SDL(1.2.5,AM_CONDITIONAL(USESDL,true),AM_CONDITIONAL(USESDL,false))
#  if test "$SDL_CFLAGS"
#  then
#    AM_CONDITIONAL(USEALLEGRO,false)
#    CFLAGS="$CFLAGS $SDL_CFLAGS"
#    LIBS="$LIBS $SDL_LIBS"
#    AC_CHECK_LIB(SDL_image,IMG_Load)
#    AC_CHECK_LIB(SDL_mixer,Mix_Linked_Version)
#  else
#    dnl Check for Allegro
#    AM_PATH_ALLEGRO(4.1.0)
#    if test "$allegro_CFLAGS"
#    then
#      AM_CONDITIONAL(USEALLEGRO,true)
#    else
#      AM_CONDITIONAL(USEALLEGRO,false)
#      AC_MSG_ERROR([* * * No compatible graphics library found!])
#    endif
#  endif
#endif

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CONFIG_FILES([Makefile
                 src/Makefile
		 widget/Makefile
		 sdlallegro/Makefile])
AC_OUTPUT
