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

#include "PortAudio.h"
#include "RingBuffer.h"

#include <iostream>
#include <unistd.h>
#include <cassert>
#include <cstring>
#include <log4cxx/logger.h>

#define RINGBUFFERSIZE 4096*16

// create logger which will become a child to logger kolibre.narrator
log4cxx::LoggerPtr narratorPaLog(log4cxx::Logger::getLogger("kolibre.narrator.portaudio"));

int pa_stream_callback(
        const void *input,
        void *output,
        unsigned long frameCount,
        const PaStreamCallbackTimeInfo* timeInfo,
        PaStreamCallbackFlags statusFlags,
        void *userData );

void pa_stream_finished_callback( void *userData );

PortAudio::PortAudio():
    ringbuf(RINGBUFFERSIZE)
{
    isInitialized = true;
    isOpen = false;
    isStarted = false;
    mChannels = 0;
    mRate = 0;
    mLatency = 0;
    pStream = NULL;

    LOG4CXX_INFO(narratorPaLog, "Initializing portaudio");

    mError = Pa_Initialize();
    if(mError != paNoError) {
        LOG4CXX_ERROR(narratorPaLog, "Failed to initialize portaudio: " << Pa_GetErrorText(mError));
        isInitialized = false;
    }
}

PortAudio::~PortAudio()
{
    close();

    if(isInitialized)
        Pa_Terminate();
}

bool PortAudio::open(long rate, int channels)
{

    if(mRate != rate || mChannels != channels) {
        close();
    }

    if(!isOpen) {
        //PaDeviceIndex default_device = 2;
        PaDeviceIndex default_device = Pa_GetDefaultOutputDevice();
        if( default_device == paNoDevice ) {
            LOG4CXX_ERROR(narratorPaLog, "Pa_GetDefaultOutputDevice failed, however, device count is: " << Pa_GetDeviceCount() );
            return false;
        }

        mOutputParameters.device = default_device; /* default output device */
        mOutputParameters.channelCount = channels;
        mOutputParameters.sampleFormat = paFloat32;
        mOutputParameters.suggestedLatency = Pa_GetDeviceInfo( mOutputParameters.device )->defaultHighOutputLatency;
        mOutputParameters.hostApiSpecificStreamInfo = NULL;

        const PaDeviceInfo* devinfo = Pa_GetDeviceInfo(mOutputParameters.device);
        const PaHostApiInfo* hostapiinfo = Pa_GetHostApiInfo(devinfo->hostApi);

        LOG4CXX_DEBUG(narratorPaLog, "Opening device: " << devinfo->name << " (" << hostapiinfo->name << "), channels: " << channels << ", rate: " << rate <<" (" << devinfo->defaultSampleRate << ")");

        unsigned long framesPerBuffer = 1024;

#ifdef WIN32
        framesPerBuffer = 4096;
#endif

        mError = Pa_OpenStream(&pStream, NULL, &mOutputParameters, rate, framesPerBuffer/*paFramesPerBufferUnspecified*/,
                paNoFlag, pa_stream_callback, this);

        if(mError != paNoError) {
            LOG4CXX_ERROR(narratorPaLog, "Failed to open stream: " << Pa_GetErrorText(mError));
            return false;
        }

        mRate = rate;
        mChannels = channels;
        isOpen = true;
        isStarted = false;

        mError = Pa_SetStreamFinishedCallback(pStream, pa_stream_finished_callback);
        if(mError != paNoError) {
            LOG4CXX_ERROR(narratorPaLog, "Failed to set FinishedCallback: " << Pa_GetErrorText(mError));
        }

    }
    return true;
}

long PortAudio::stop()
{
    if(isStarted) {
        mError = Pa_StopStream(pStream);
        if(mError != paNoError)
            LOG4CXX_ERROR(narratorPaLog, "Failed to stop stream: " << Pa_GetErrorText(mError));

        ringbuf.flush();
        isStarted = false;
    }

    return mLatency;
}

long PortAudio::abort()
{
    if(isStarted) {
        LOG4CXX_DEBUG(narratorPaLog, "Aborting stream");
        mError = Pa_AbortStream(pStream);
        if(mError != paNoError)
            LOG4CXX_ERROR(narratorPaLog, "Failed to abort stream: " << Pa_GetErrorText(mError));

        ringbuf.flush();
        isStarted = false;
    }

    return mLatency;
}

bool PortAudio::close()
{
    stop();
    if(isOpen) {
        mError = Pa_CloseStream(pStream);
        if(mError != paNoError)
        {
            LOG4CXX_ERROR(narratorPaLog, "Failed to close stream: " << Pa_GetErrorText(mError));
        }
        else
        {
            isOpen = false;
        }
    }
    return true;
}

long PortAudio::getRate()
{
    return mRate;
}

int PortAudio::getChannels()
{
    return mChannels;
}

long PortAudio::getRemainingms()
{
    size_t bufferedData = ringbuf.getReadAvailable();

    long bufferms = 0;

    if(mChannels != 0 && mRate != 0)
        bufferms = (long) (1000.0 * bufferedData) / mChannels / mRate;

    return bufferms + mLatency;
}

/*
   Blocks until data can be written.
   If no data can be written in 2ms*100 the stream gets restarted.
   returns the amount of data that can be written.
*/
unsigned int PortAudio::getWriteAvailable()
{
    size_t writeAvailable = 0;
    int waitCount = 0;
    int failCount = 0;

    while( writeAvailable == 0 ) {
        writeAvailable = ringbuf.getWriteAvailable();

        if( writeAvailable == 0 ) {
            usleep(200000); //200 ms
            //LOG4CXX_DEBUG(narratorPaLog, "getWriteAvailable = 0, Pa_IsStreamActive = " << Pa_IsStreamActive(pStream));
        }

        if(waitCount++ > 10) {
            LOG4CXX_ERROR(narratorPaLog, "getWriteAvailable waittime exceeded, restarting stream");

            mError = Pa_AbortStream(pStream);
            // If abortstream fails, try reopening
            if(failCount++ > 0 || ( mError != paNoError && mError != paStreamIsStopped )) {
                LOG4CXX_ERROR(narratorPaLog, "Failed to abort stream, trying to close and reopen: " << Pa_GetErrorText(mError));

                mError = Pa_CloseStream(pStream);
                if(mError != paNoError)
                    LOG4CXX_ERROR(narratorPaLog, "Failed to close stream: " << Pa_GetErrorText(mError));

                isOpen = false;
                open(mRate, mChannels);
            }

            mError = Pa_StartStream(pStream);
            if(mError != paNoError) LOG4CXX_ERROR(narratorPaLog, "Failed to start stream: " << Pa_GetErrorText(mError));
            isStarted = true;

            waitCount = 0;
        }
    }

    //LOG4CXX_DEBUG(narratorPaLog, "getWriteAvailable = " << writeAvailable << ", Pa_IsStreamActive = " << Pa_IsStreamActive(pStream));
    return writeAvailable;
}

bool PortAudio::write(float *buffer, unsigned int samples)
{
    size_t elemWritten = ringbuf.writeElements(buffer, samples*mChannels);

    // Try starting the stream
    if(!isStarted && ringbuf.getWriteAvailable() <= (RINGBUFFERSIZE/2)) {
        LOG4CXX_TRACE(narratorPaLog, "Starting stream");
        mError = Pa_StartStream(pStream);
        if(mError != paNoError) {
            LOG4CXX_ERROR(narratorPaLog, "Failed to start stream: " << Pa_GetErrorText(mError));
        }
        mLatency = (long) (Pa_GetStreamInfo(pStream)->outputLatency * 1000.0);
        isStarted = true;
    }
    else if(!isStarted )
        LOG4CXX_TRACE(narratorPaLog, "Buffering: " << ((RINGBUFFERSIZE - ringbuf.getWriteAvailable()) * 100) / (RINGBUFFERSIZE) << "%");

    if( elemWritten < samples)
        return false;

    return false;
}

long unsigned int min( long unsigned int a, long unsigned int b )
{
    if( a < b )
        return a;
    return b;
}

/*
   @note With the exception of Pa_GetStreamCpuLoad() it is not permissable to call
   PortAudio API functions from within the stream callback.
*/
int pa_stream_callback(
        const void *input,
        void *output, // Write frameCount*channels here
        unsigned long frameCount,
        const PaStreamCallbackTimeInfo* timeInfo,
        PaStreamCallbackFlags statusFlags,
        void *userData )
{
    (void) timeInfo;    /* Prevent unused variable warning. */
    (void) input; /* Prevent unused variable warning. */

    //if(statusFlags & paOutputUnderflow)
    //    LOG4CXX_WARN(narratorPaLog, "Output underflow!");
    //if(statusFlags & paOutputOverflow)
    //    LOG4CXX_WARN(narratorPaLog, "Output overflow!");
    //if(statusFlags & paPrimingOutput)
    //    LOG4CXX_WARN(narratorPaLog, "Priming output!");

    static long underrunms = 0;

    RingBuffer *ringbuf = &((PortAudio*)userData)->ringbuf;

    int channels = ((PortAudio*)userData)->mChannels;
    long rate = ((PortAudio*)userData)->mRate;

    float* outbuf = (float*)output;

    size_t elementsRead = ringbuf->readElements(outbuf, frameCount * channels);

    if( elementsRead < frameCount*channels ) {
        memset( (outbuf+(elementsRead)), 0, (frameCount*channels-elementsRead)*sizeof(float) );
        underrunms += (long) (frameCount * channels * 1000.0) / rate;
        //LOG4CXX_DEBUG(narratorPaLog, " Less read than requested, underrun ms:" << underrunms );
    } else {
        //LOG4CXX_TRACE(narratorPaLog, " availableElements: " << availableElements << " elementsToRead: " << elementsToRead << " elementsRead:" << elementsRead);
        underrunms = 0;
    }

    if(underrunms > 500) return paComplete;

    return paContinue; // paAbort, paComplete
}

void pa_stream_finished_callback( void *userData )
{
    ((PortAudio*)userData)->isStarted = false;
    //std::cout << __FUNCTION__ << " Finished" << std::endl;
}
