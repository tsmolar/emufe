#!/bin/sh

# regenerate configure

aclocal -I m4
autoheader
#svn log -v > ChangeLog
automake --add-missing --copy
autoconf
