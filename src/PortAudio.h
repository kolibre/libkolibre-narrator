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

#ifndef _PORTAUDIO_H
#define _PORTAUDIO_H

#include <portaudio.h>
#include "RingBuffer.h"

class PortAudio {
    public:
        PortAudio();
        ~PortAudio();

        // Opens a new output stream, reuses the old one in case rate and channels match
        bool open(long rate, int channels);
        long stop();
        long abort();
        bool close();

        long getRate();
        int getChannels();

        // Checks how many samples we can write
        unsigned int getWriteAvailable();

        // Return remaining audio in internal buffers (ms)
        long getRemainingms();

        // Writes no more than getWriteAvailable samples
        bool write(float *buffer, unsigned int samples);

    private:
        bool isInitialized;
        bool isOpen;
        bool isStarted;

        PaStreamParameters mOutputParameters;
        PaStream *pStream;
        PaError mError;

        long mRate;
        int mChannels;
        long mLatency;

        RingBuffer ringbuf;

        friend int pa_stream_callback(
                const void *input,
                void *output,
                unsigned long frameCount,
                const PaStreamCallbackTimeInfo* timeInfo,
                PaStreamCallbackFlags statusFlags,
                void *userData );

        friend void pa_stream_finished_callback( void *userData );

};

#endif
