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

AUTOMAKE_OPTIONS = foreign

bin_SCRIPTS = narrator-utils

pkgpyexec_PYTHON = Audio.py Message.py Parameter.py Translation.py build_message_db.py csvparser.py messagehelper.py

specialpromptdir = $(pkgpyexecdir)/special
specialprompt_DATA = special/wait.wav \
					 special/wait.ogg \
					 special/shortpause.wav \
					 special/shortpause.ogg \
					 special/longpause.wav \
					 special/longpause.ogg

EXTRA_DIST = $(specialprompt_DATA)
