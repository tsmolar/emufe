dnl @synopsis CHECK_ZDLL()
dnl
dnl This macro searches for an installed zdll (dll version) library. If nothing was
dnl specified when calling configure, it searches first in /usr/local
dnl and then in /usr. If the --with-zdll=DIR is specified, it will try
dnl to find it in DIR/include/zlib.h and DIR/lib/libz.a. If
dnl --without-zdll is specified, the library is not searched at all.
dnl
dnl If either the header file (zlib.h) or the library (libz) is not
dnl found, the configuration exits on error, asking for a valid zdll
dnl installation directory or --without-zdll.
dnl
dnl The macro defines the symbol HAVE_LIBZ if the library is found. You
dnl should use autoheader to include a definition for this symbol in a
dnl config.h file. Sample usage in a C/C++ source is as follows:
dnl
dnl   #ifdef HAVE_LIBZ
dnl   #include <zlib.h>
dnl   #endif /* HAVE_LIBZ */
dnl
dnl @category InstalledPackages
dnl @author Loic Dachary <loic@senga.org>
dnl @version 2004-09-20
dnl @license GPLWithACException

AC_DEFUN([CHECK_ZDLL],
#
# Handle user hints
#
[AC_MSG_CHECKING(if zdll is wanted)
AC_ARG_WITH(zdll,
[  --with-zdll=DIR root directory path of zdll installation
  --without-zdll to disable zdll usage completely],
[if test "$withval" != no ; then
  AC_MSG_RESULT(yes)
  if test -d "$withval"
  then
    ZDLL_HOME="$withval"
  else
    AC_MSG_WARN([Sorry, $withval does not exist, checking usual places])
  fi
else
  AC_MSG_RESULT(no)
fi], [
ZDLL_HOME=/usr/local
if test ! -f "${ZDLL_HOME}/include/zlib.h"
then
        ZDLL_HOME=/usr
fi
])
#
# Locate zdll, if wanted
#
if test -n "${ZDLL_HOME}"
then
        ZDLL_OLD_LDFLAGS=$LDFLAGS
#        ZDLL_OLD_CPPFLAGS=$LDFLAGS
        ZDLL_OLD_CPPFLAGS=$CPPFLAGS
        ZDLL_LDFLAGS="-L${ZDLL_HOME}/lib"
        ZDLL_CPPFLAGS="-I${ZDLL_HOME}/include"
	LDFLAGS="$LDFLAGS $ZDLL_LDFLAGS"
	CPPFLAGS="$CPPFLAGS $ZDLL_CPPFLAGS"
        AC_LANG_SAVE
        AC_LANG_C
        AC_CHECK_LIB(zdll, inflateEnd, [zdll_cv_libz=yes], [zdll_cv_libz=no])
        AC_CHECK_HEADER(zlib.h, [zdll_cv_zdll_h=yes], [zdll_cv_zdll_h=no])
        AC_LANG_RESTORE
        if test "$zdll_cv_libz" = "yes" -a "$zdll_cv_zdll_h" = "yes"
        then
                #
                # If both library and header were found, use them
                #
                AC_CHECK_LIB(zdll, inflateEnd)
                AC_MSG_CHECKING(zdll in ${ZDLL_HOME})
		have_zdll=yes
                AC_MSG_RESULT(ok)
        else
                #
                # If either header or library was not found, revert and bomb
                #
                AC_MSG_CHECKING(zdll in ${ZDLL_HOME})
                ZDLL_LDFLAGS=""
                ZDLL_CPPFLAGS=""
                AC_MSG_RESULT(failed)
                #AC_MSG_ERROR(either specify a valid zdll installation with --with-zdll=DIR or disable zdll usage with --without-zdll)
        fi
	LDFLAGS="$ZDLL_OLD_LDFLAGS"
	CPPFLAGS="$ZDLL_OLD_CPPFLAGS"
fi

])
