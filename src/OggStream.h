/*
Copyright (C) 2012  The Kolibre Foundation

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

#ifndef _OGGSTREAM_H
#define _OGGSTREAM_H

#include "Message.h"
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

class OggStream
{
    public:
        OggStream();
        ~OggStream();

        bool open(const MessageAudio &);
        bool open(string);
        long read(float* buffer, int bytes);
        bool close();

        long getRate() { return mRate; };
        long getChannels() { return mChannels; };

    private:
        OggVorbis_File mStream;
        string mStreamInfo;
        int mSection;
        ov_callbacks mCallbacks;
        int mChannels;
        long mRate;
        MessageAudio currentAudio;
        bool isOpen;
};

#endif
