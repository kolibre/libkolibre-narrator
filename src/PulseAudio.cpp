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

PulseAudio::PulseAudio():
    ringbuf(RINGBUFFERSIZE)
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
    if (isOpen)
    {
        pa_simple_free(pSimple);
        isOpen = false;
    }
    return false;
}

unsigned int PulseAudio::getWriteAvailable()
{
    size_t writeAvailable = 0;
    int waitCount = 0;

    while (writeAvailable == 0)
    {
        writeAvailable = ringbuf.getWriteAvailable();

        if (writeAvailable == 0)
        {
            usleep(200000); //200 ms
        }

        if (waitCount++ > 10)
        {
            // abort and reopen stream
            break;
        }
    }

    return writeAvailable;
}

long PulseAudio::getRemainingms()
{
    size_t bufferedData = ringbuf.getReadAvailable();

    long bufferms = 0;

    if (mChannels != 0 && mRate != 0)
        bufferms = (long) (1000.0 * bufferedData) / mChannels / mRate;

    return bufferms + mLatency;
}

bool PulseAudio::write(float *buffer, unsigned int samples)
{
    size_t elemWritten = ringbuf.writeElements(buffer, samples*mChannels);

    // Try starting the stream
    if (!isStarted && ringbuf.getWriteAvailable() <= (RINGBUFFERSIZE/2))
    {
        LOG4CXX_TRACE(narratorPuaLog, "Starting stream");
        int retVal = pa_simple_write(pSimple, buffer, elemWritten, &mError);
        if (retVal)
        {
            //std::string errMsg(pa_strerror(mError));
            std::string errMsg = "error";
            LOG4CXX_ERROR(narratorPuaLog, "Failed to start stream: " << errMsg);
        }

        pa_usec_t latency;
        latency = pa_simple_get_latency(pSimple, &mError);
        if (latency == (pa_usec_t) -1)
        {
            //std::string errMsg(pa_strerror(mError));
            std::string errMsg = "error";
            LOG4CXX_ERROR(narratorPuaLog, "Failed to get latency: " << errMsg);
        }
        mLatency = (long)latency;

        isStarted = true;
    }
    else if (!isStarted)
        LOG4CXX_TRACE(narratorPuaLog, "Buffering: " << ((RINGBUFFERSIZE - ringbuf.getWriteAvailable()) * 100) / (RINGBUFFERSIZE) << "%");

    if (elemWritten < samples)
        return false;

    return false;
}
