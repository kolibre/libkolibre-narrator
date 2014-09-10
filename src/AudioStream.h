/*
Copyright (C) 2012 Kolibre

This file is part of kolibre-narrator.

Kolibre-narrator is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Kolibre-narrator is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with kolibre-narrator. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _AUDIOSTREAM_H
#define _AUDIOSTREAM_H

#include "Message.h"

class AudioStream
{
    public:
        virtual ~AudioStream() {}

        virtual bool open(const MessageAudio &) = 0;
        virtual bool open(string) = 0;
        virtual long read(float* buffer, int bytes) = 0;
        virtual bool close() = 0;

        virtual long getRate() = 0;
        virtual long getChannels() = 0;
};

#endif
