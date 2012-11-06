## Copyright (C) 2012 Kolibre
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

AC_PREREQ([2.67])
AC_INIT([libkolibre-narrator-utils],[0.0.1],[BUG-REPORT-ADDRESS])
AC_CONFIG_SRCDIR([.])
AM_INIT_AUTOMAKE
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AM_PATH_PYTHON

dnl -----------------------------------------------
dnl check for oggenc, ogginfo
dnl -----------------------------------------------

AC_CHECK_PROG(HAVE_VORBISTOOLS, oggenc, yes, no)

if (test "x$HAVE_VORBISTOOLS" = "xno") then
  AC_MSG_ERROR([**vorbis-tools is needed to run libkolibre-narrator-utils **])
fi

dnl -----------------------------------------------
dnl check for espeak
dnl -----------------------------------------------

AC_CHECK_PROG(HAVE_ESPEAK, espeak, yes, no)

if (test "x$HAVE_ESPEAK" = "xno") then
  AC_MSG_ERROR([**espeak is needed to run libkolibre-narrator-utils **])
fi

dnl -----------------------------------------------
dnl check for sox
dnl -----------------------------------------------

AC_CHECK_PROG(HAVE_SOX, sox, yes, no)

if (test "x$HAVE_SOX" = "xno") then
  AC_MSG_ERROR([**sox is needed to run libkolibre-narrator-utils **])
fi

AC_CONFIG_FILES([Makefile narrator-utils])

AC_OUTPUT
