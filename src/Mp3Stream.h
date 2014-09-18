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

#ifndef _MP3STREAM_H
#define _MP3STREAM_H

#include "AudioStream.h"
#include <mpg123.h>

class Mp3Stream: public AudioStream
{
    public:
        Mp3Stream();
        ~Mp3Stream();

        bool open(const MessageAudio &);
        bool open(string);
        long read(float* buffer, int bytes);
        bool close();

        long getRate();
        long getChannels();

    private:
        mpg123_handle *mh;

        int mError;
        int mEncoding;
        int mChannels;
        long mRate;
        bool isOpen;
        size_t mFrameSize;
        float scaleNegative;
        float scalePositive;

        MessageAudio currentAudio;
};

#endif
