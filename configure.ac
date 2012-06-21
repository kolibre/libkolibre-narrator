## Copyright (C) 2012  The Kolibre Foundation
#
# This file is part of kolibre-narrator.
#
# Kolibre-narrator is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 2.1 of the License, or
# (at your option) any later version.
#
# Kolibre-narrator is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with kolibre-narrator. If not, see <http://www.gnu.org/licenses/>.

# Process this file with autoconf to produce a configure script.

dnl -----------------------------------------------
dnl  Release instructions
dnl -----------------------------------------------
dnl
dnl  When you are about to release the software
dnl  either as a stable release or an unstable release
dnl  you should update the *_VERSION variables below
dnl  according to the following rules:
dnl
dnl  1. If any interfaces has been removed or changed,
dnl     increment MAJOR_VERSION and set others to 0
dnl  2. If any interface has been added,
dnl     increment MINOR_VERSION and set PATCH_VERSION to 0
dnl  3. If only the source code has changed,
dnl     increment PATCH_VERSION
dnl
dnl  If you are releasing a stable version,
dnl  set package version number in AC_INIT to
dnl  [$MAJOR_VERSION.$MINOR_VERSION.$PATCH_VERSION]
dnl
dnl  If you are releasing an unstable version,
dnl  add a string (i.e -alpha or -beta or -rc1) to the
dnl  package version in AC_INIT after $PATCH_VERSION
dnl  e.g. [$MAJOR_VERSION.$MINOR_VERSION.$PATCH_VERSION-rc1]

AC_PREREQ([2.67])
AC_INIT([kolibre-narrator],[0.0.1],[BUG-REPORT-ADDRESS])
AC_CONFIG_SRCDIR([src/Narrator.h])
AM_INIT_AUTOMAKE([tar-pax])
AC_CONFIG_HEADERS([config.h])

# Setup version
MAJOR_VERSION=0
MINOR_VERSION=0
PATCH_VERSION=1

VERSION_INFO=`expr $MAJOR_VERSION + $MINOR_VERSION`:$PATCH_VERSION:$MINOR_VERSION
AC_SUBST(VERSION_INFO)

# Checks for programs.
LT_INIT
AC_PROG_CXX
AC_LANG([C++])
AX_BOOST_BASE
AC_CHECK_DECL([HAVE_BOOST], [], [AC_MSG_ERROR([**autoconf-archive is needed to build libkolibre-narrator])])
AX_BOOST_SIGNALS
AC_PROG_CC
ACX_PTHREAD

DX_DOXYGEN_FEATURE(ON)
DX_HTML_FEATURE(ON)
DX_PDF_FEATURE(ON)
DX_INIT_DOXYGEN([libkolibre-narrator], doxygen.cfg, [doxygen-doc])

# Checks for libraries.

AC_CHECK_LIB(gst, sqlite3)

dnl -----------------------------------------------
dnl check for log4cxx
dnl -----------------------------------------------

PKG_CHECK_MODULES(LOG4CXX, liblog4cxx >= 0.10.0, HAVE_LOG4CXX=yes, HAVE_LOG4CXX=no)

if (test "x$HAVE_LOG4CXX" = "xno") then
    AC_MSG_ERROR([**liblog4cxx >= 0.10.0 is needed to build libkolibre-narrator **])
fi

AC_SUBST(LOG4CXX_CFLAGS)
AC_SUBST(LOG4CXX_LIBS)

dnl -----------------------------------------------
dnl check for vorbisfile
dnl -----------------------------------------------

PKG_CHECK_MODULES(VORBISFILE, vorbisfile >= 1.2.0, HAVE_VORBISFILE=yes, HAVE_VORBISFILE=no)

if (test "x$HAVE_VORBISFILE" = "xno") then
    AC_MSG_ERROR([**libvorbisfile >= 1.2.0 is needed to build libkolibre-narrator **])
fi

AC_SUBST(VORBISFILE_CFLAGS)
AC_SUBST(VORBISFILE_LIBS)

dnl -----------------------------------------------
dnl Check for portaudio
dnl -----------------------------------------------

PKG_CHECK_MODULES(PORTAUDIO, portaudio-2.0 >= 19, HAVE_PORTAUDIO=yes, HAVE_PORTAUDIO=no)

if (test "x$HAVE_PORTAUDIO" = "xno") then
    AC_MSG_ERROR([**libportaudio >= 19 is needed to build libkolibre-narrator **])
fi

AC_SUBST(PORTAUDIO_LIBS)
AC_SUBST(PORTAUDIO_CFLAGS)

dnl -----------------------------------------------
dnl Check for sqlite3
dnl -----------------------------------------------

PKG_CHECK_MODULES(SQLITE3, sqlite3 >= 3.5, HAVE_SQLITE3=yes, HAVE_SQLITE3=no)

if (test "x$HAVE_SQLITE3" = "xno") then
   AC_MSG_ERROR([**libsqlite3 >= 3.5 is needed to build libkolibre-narrator **])
fi

AC_SUBST(SQLITE3_LIBS)
AC_SUBST(SQLITE3_CFLAGS)

dnl -----------------------------------------------
dnl Check for SoundTouch
dnl -----------------------------------------------

AC_ARG_ENABLE(integer-samples,
              [AS_HELP_STRING([--enable-integer-samples],[use integer samples instead of floats
[default=yes]])],,
              [enable_integer_samples=no])


if test "x$enable_integer_samples" = "xyes"; then
        echo "****** Integer sample type enabled ******"
        AC_DEFINE(INTEGER_SAMPLES,1,[Use Integer as Sample type])
        PKG_CHECK_MODULES(SOUNDTOUCH, soundtouchi-1.4 >= 1.3.1, HAVE_SOUNDTOUCH=yes, HAVE_SOUNDTOUCH=no)
        PKG_CHECK_MODULES(SOUNDTOUCH, soundtouchi-1.0 >= 1.3.1, HAVE_SOUNDTOUCH=yes, HAVE_SOUNDTOUCH=no)
        if (test "x$HAVE_SOUNDTOUCH" = "xno") then
           AC_MSG_ERROR([**libsoundtouchi >= 1.3.1 is needed to build libkolibre-narrator **])
        fi
else
        echo "****** Float sample type enabled ******"
        AC_DEFINE(FLOAT_SAMPLES,1,[Use Float as Sample type])
        PKG_CHECK_MODULES(SOUNDTOUCH, soundtouch-1.0, HAVE_SOUNDTOUCH=yes, HAVE_SOUNDTOUCH=no)
        if (test "x$HAVE_SOUNDTOUCH" = "xno") then
           AC_MSG_ERROR([**libsoundtouch >= 1.3.1 is needed to build libkolibre-narrator **])
        fi
fi

AC_SUBST(SOUNDTOUCH_LIBS)
AC_SUBST(SOUNDTOUCH_CFLAGS)

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h malloc.h memory.h stdlib.h termios.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([floor memset])

dnl -----------------------------------------------
dnl Call configure recursively
dnl -----------------------------------------------

AC_ARG_WITH([utils], AS_HELP_STRING([--without-utils], [dont build and install narrator utils]))
AS_IF([test "x$with_utils" != "xno"],
[AC_CONFIG_SUBDIRS([utils])])

AC_CONFIG_FILES([Makefile
                 doxygen.cfg
                 libkolibre-narrator.pc
                 src/Makefile
                 tests/Makefile])
AC_OUTPUT

