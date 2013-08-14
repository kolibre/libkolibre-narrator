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

#include "Filter.h"

Filter::Filter()
{
    setSetting(SETTING_USE_AA_FILTER, true);
    setSetting(SETTING_AA_FILTER_LENGTH, 64);
    setSetting(SETTING_USE_QUICKSEEK, false);

    setSetting(SETTING_SEQUENCE_MS, 41);
    setSetting(SETTING_SEEKWINDOW_MS, 28);
    setSetting(SETTING_OVERLAP_MS, 8);

    mTempo = 1.0;
    mPitch = 1.0;
    mGain = 1.0;
    mRate = 0;
    mChannels = 0;

    setTempo(mTempo);
    setPitch(mPitch);

    // Set sane defaults
    open(44100, 2);
}

Filter::~Filter()
{
    clear();
}

bool Filter::open(long rate, int channels)
{
    if(mRate != rate || mChannels != channels) {
        mRate = rate;
        setSampleRate(mRate);
        mChannels = channels;
        setChannels(mChannels);
    }
    return true;
}

bool Filter::write(float *buffer, unsigned int samples)
{
    putSamples(buffer, samples); // One sample contains data from all channels
    return true;
}

unsigned int Filter::read(float *buffer, unsigned int bytes)
{
    bytes = receiveSamples(buffer, bytes);
    applyGain(buffer, bytes);
    return bytes;
}

void Filter::applyGain(float *buffer, unsigned int samples)
{
    // Change the gain on the buffer
    static unsigned int i;

    if(mGain == 1.0) return;

    for(i = 0; i < samples; i++) {
        buffer[i] = buffer[i] * mGain;
    }
}

void Filter::fadeout(float *buffer, unsigned int bytes)
{
    // Linear fadeout on the samples in the buffer
    static unsigned int i;
    float val = 1;
    for(i = 0; i < bytes; i++) {
        val = (bytes - i) / bytes;
        buffer[i] = buffer[i] * val;
    }
}
