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

AUTOMAKE_OPTIONS = foreign

check_PROGRAMS = basicsay dbtest interfacetest stress_test
TESTS = basicsay dbtest.sh interfacetest.sh stress_test.sh

stress_test_CPPFLAGS = @LOG4CXX_CFLAGS@
stress_test_SOURCES = stress_test.cpp
stress_test_LDADD = -L$(top_builddir)/src/ -lkolibre-narrator @LOG4CXX_LIBS@

basicsay_CPPFLAGS = @LOG4CXX_CFLAGS@
basicsay_SOURCES = basicsay.cpp
basicsay_LDADD = -L$(top_builddir)/src/ -lkolibre-narrator @LOG4CXX_LIBS@

dbtest_CPPFLAGS = @LOG4CXX_CFLAGS@
dbtest_SOURCES = dbtest.cpp
dbtest_LDADD = -L$(top_builddir)/src/ -lkolibre-narrator @LOG4CXX_LIBS@

interfacetest_CPPFLAGS = @LOG4CXX_CFLAGS@
interfacetest_SOURCES = interfacetest.cpp
interfacetest_LDADD = -L$(top_builddir)/src/ -lkolibre-narrator @LOG4CXX_LIBS@

INCLUDES = -I$(top_srcdir)/src

EXTRA_DIST = aktuell_sida.ogg setup_logging.h dbtest.sh interfacetest.sh stress_test.sh
