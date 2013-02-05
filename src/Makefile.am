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

AUTOMAKE_OPTIONS=foreign

lib_LTLIBRARIES = libkolibre-narrator.la

# Install the headers in a versioned directory - e.g. examplelib-1.0:
library_includedir = $(includedir)/libkolibre/narrator-$(PACKAGE_VERSION)
library_include_HEADERS = Narrator.h

libkolibre_narrator_la_SOURCES = Narrator.cpp Message.cpp OggStream.cpp Filter.cpp RingBuffer.cpp PortAudio.cpp MessageHandler.cpp Db.cpp
libkolibre_narrator_la_LIBADD = @LOG4CXX_LIBS@ @VORBISFILE_LIBS@ @PORTAUDIO_LIBS@ @SQLITE3_LIBS@ @SOUNDTOUCH_LIBS@
libkolibre_narrator_la_LDFLAGS = @BOOST_LDFLAGS@ -version-info $(VERSION_INFO)
libkolibre_narrator_la_CPPFLAGS = @LOG4CXX_CFLAGS@ @VORBISFILE_CFLAGS@ @PORTAUDIO_CFLAGS@ @SOUNDTOUCH_CFLAGS@ @BOOST_CPPFLAGS@

EXTRA_DIST = OggStream.h PortAudio.h Filter.h RingBuffer.h Message.h MessageHandler.h Db.h
