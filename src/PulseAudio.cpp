/*
 * Copyright (C) 2014 Johan Abbors
 *
 * This file is part of kolibre-narrator.
 *
 * Kolibre-narrator is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * Kolibre-narrator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with kolibre-narrator. If not, see <http://www.gnu.org/licenses/>.
 */

#include "PulseAudio.h"

#include <unistd.h>
#include <string>
#include <log4cxx/logger.h>

#define RINGBUFFERSIZE 1024*16

// create logger which will become a child to logger kolibre.narrator
log4cxx::LoggerPtr narratorPuaLog(log4cxx::Logger::getLogger("kolibre.narrator.pulseaudio"));

PulseAudio::PulseAudio()
{
    isInitialized = true;
    isOpen = false;
    isStarted = false;
    mChannels = 0;
    mRate = 0;
    mLatency = 0;
}

PulseAudio::~PulseAudio()
{
    abort();
    close();
}

bool PulseAudio::open(long rate, int channels)
{
    if (mRate != rate || mChannels != mChannels)
    {
        close();
    }

    if (!isOpen)
    {
        mSpec.format = PA_SAMPLE_FLOAT32LE;
        mSpec.rate = rate;
        mSpec.channels = channels;

        LOG4CXX_INFO(narratorPuaLog, "Connecting to PulseAudio server...");

        pSimple = pa_simple_new(NULL,   // Use the default server.
                "Narrator",             // Our application's name.
                PA_STREAM_PLAYBACK,     // Open stream for playback.
                NULL,                   // Use the default device.
                "Short messages",       // Description of our stream.
                &mSpec,                 // Our sample format.
                NULL,                   // Use default channel map
                NULL,                   // Use default buffering attributes.
                &mError                 // Use error code.
                );

        if (!pSimple)
        {
            //std::string errMsg(pa_strerror(mError));
            std::string errMsg = "error";
            LOG4CXX_ERROR(narratorPuaLog, "Connection failed: " << errMsg);
            return false;
        }

        LOG4CXX_INFO(narratorPuaLog, "Connection established");

        mRate = rate;
        mChannels = channels;
        isOpen = true;
        isStarted = false;
    }

    return true;
}

long PulseAudio::stop()
{
    return abort();
}

long PulseAudio::abort()
{
    if (isStarted)
    {
        pa_simple_flush(pSimple, &mError);
        isStarted = false;
    }
    return mLatency;
}

bool PulseAudio::close()
{
    if (isStarted)
    {
        if (pa_simple_drain(pSimple, &mError) < 0)
        {
            //std::string errMsg(pa_strerror(mError));
            std::string errMsg = "error";
            LOG4CXX_ERROR(narratorPuaLog, "pa_simple_drain() failed: " << errMsg);
        }
        isStarted = false;
    }

    if (isOpen)
    {
        pa_simple_free(pSimple);
        isOpen = false;
    }
    return false;
}

unsigned int PulseAudio::getWriteAvailable()
{
    return RINGBUFFERSIZE;
}

long PulseAudio::getRemainingms()
{
    return 0;
}

bool PulseAudio::write(float *buffer, unsigned int samples)
{
    if (!isStarted)
    {
        LOG4CXX_TRACE(narratorPuaLog, "Starting stream");
        isStarted = true;
    }

    // don't know why but multiplying samples by 4 works
    int retVal = pa_simple_write(pSimple, buffer, samples*4, &mError);
    if (retVal)
    {
        //std::string errMsg(pa_strerror(mError));
        std::string errMsg = "error";
        LOG4CXX_ERROR(narratorPuaLog, "Failed to write samples: " << errMsg);
    }

    return false;
}
