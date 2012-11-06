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

#ifndef _FILTER_H
#define _FILTER_H

#include <SoundTouch.h>

// SoundTouch.h defines malloc to rpl_malloc?
#ifdef malloc
#undef malloc
#endif

// extend the soundtouch class to get a simpler interface
class Filter : public soundtouch::SoundTouch {
    public:
        Filter();
        ~Filter();

        bool open(long rate, int channels);
        bool write(float *buffer, unsigned int bytes);
        unsigned int read(float *buffer, unsigned int bytes);
        unsigned int availableSamples();

        void setGain(double gain) { mGain = gain; };

        void fadeout(float *buffer, unsigned int bytes);

    private:
        double mTempo;
        double mPitch;
        double mGain;

        long mRate;
        int mChannels;

        void applyGain(float *buffer, unsigned int samples);
};

#endif
