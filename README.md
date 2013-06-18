What is Kolibre?
---------------------------------
Kolibre is a Finnish non-profit association whose purpose is to promote
information systems that aid people with reading disabilities. The software
which Kolibre develops is published under open source and made available to all
stakeholders at github.com/kolibre.

Kolibre is committed to broad cooperation between organizations, businesses and
individuals around the innovative development of custom information systems for
people with different needs. More information about Kolibres activities, association 
embership and contact information can be found at http://www.kolibre.org/


What is libkolibre-narrator?
---------------------------------
Libkolibre-narrator is a library for playing audio prompts. It provides necessary
interface methods to play numbers, dates, time, durations and strings stored in
a message database. The package includes a set of utilities that can be used to
generate the messages database.


Documentation
---------------------------------
Kolibre client developer documentation is available at 
https://github.com/kolibre/libkolibre-builder/wiki

This library is documented using doxygen.

Type ./configure && make doxygen-doc to generate documentation.


Platforms
---------------------------------
Libkolibre-narrator has been tested with Linux Debian Squeeze and can be built
using dev packages from apt repositories.


Dependencies
---------------------------------
Major dependencies for libkolibre-narrator:

* libvorbisfile
* libportaudio19
* sqlite3
* libsoundtouch
* libogg
* libjack
* libpthread
* liblog4cxx


Building from source
---------------------------------
If building from GIT sources, first do a:

    $ autoreconf -fi

If building from a release tarball you can skip the above step.

    $ ./configure
    $ make
    $ make install

see INSTALL for detailed instructions.


Licensing
---------------------------------
Copyright (C) 2012 Kolibre

This file is part of libkolibre-narrator.

Libkolibre-narrator is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Libkolibre-narrator is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libkolibre-narrator. If not, see <http://www.gnu.org/licenses/>.

[![githalytics.com alpha](https://cruel-carlota.pagodabox.com/86183bc334a5abaa4505507a97c0931a "githalytics.com")](http://githalytics.com/kolibre/libkolibre-narrator)
