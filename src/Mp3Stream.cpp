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

#include "Message.h"
#include "Mp3Stream.h"

#include <cstdio>
#include <sstream>
#include <math.h>
#include <log4cxx/logger.h>

// create logger which will become a child to logger kolibre.narrator
log4cxx::LoggerPtr narratorMsLog(log4cxx::Logger::getLogger("kolibre.narrator.mp3stream"));

Mp3Stream::Mp3Stream()
{
    LOG4CXX_INFO(narratorMsLog, "Initializing mp3stream");

    mError = 0;
    mEncoding = 0;
    mChannels = 0;
    mRate = 0;
    isOpen = false;
    openTmpFile = false;
    mTmpFile = "";
    scaleNegative = powf(2, 16);
    scalePositive = scaleNegative - 1;

    mpg123_init();
    mh = mpg123_new(NULL, &mError);
    mFrameSize = mpg123_outblock(mh);
}

Mp3Stream::~Mp3Stream()
{
    close();
    if (mh) mpg123_delete(mh);
    mpg123_exit();
}

bool Mp3Stream::open(const MessageAudio &ma)
{
    // opening mp3 audio from database could possibly be done by libmpg123 feed API calls
    // but since this fearture will bu used very seldom it's sufficient to create temporary files
    // and let libmpg123 open a file on the filesystem

    size_t bytesToRead = ma.getSize();

    // read audio data from database to buffer
    LOG4CXX_DEBUG(narratorMsLog, "reading " << bytesToRead << " bytes from database");
    currentAudio = ma;
    char *buffer;
    buffer = (char *) malloc (bytesToRead * sizeof(char));
    size_t bytesRead = currentAudio.read(buffer, sizeof(char), bytesToRead);
    currentAudio.close();

    // write buffer data to tmp file
    mTmpFile = std::tmpnam(NULL);
    LOG4CXX_DEBUG(narratorMsLog, "creating temporary file " << mTmpFile);
    FILE *pFile = fopen(mTmpFile.c_str(), "wb");
    if (pFile == NULL)
    {
        LOG4CXX_ERROR(narratorMsLog, "failed to open temporary file");
        free(buffer);
        return false;
    }
    LOG4CXX_DEBUG(narratorMsLog, "writing " << bytesRead << " bytes to file");
    fwrite(buffer, sizeof(char), bytesRead, pFile);
    fclose(pFile);
    free(buffer);

    openTmpFile = true;
    return open(mTmpFile);
}

bool Mp3Stream::open(string path)
{
    // open file
    int result = mpg123_open(mh, path.c_str());
    if (result != MPG123_OK)
    {
        LOG4CXX_ERROR(narratorMsLog, "File " << path << " could not be opened");
        return false;
    }

    // get rate, channels and encoding
    mpg123_getformat(mh, &mRate, &mChannels, &mEncoding);
    isOpen = true;

    return true;
}

// Returns samples (1 sample contains data from all channels)
long Mp3Stream::read(float* buffer, int bytes)
{
    LOG4CXX_TRACE(narratorMsLog, "read " << bytes << " bytes from mp3 file");

    // mpg123 uses enconding MPG123_ENC_SIGNED_16 which results in decoded short samples
    short *shortBuffer;
    shortBuffer = new short[bytes*mChannels];

    size_t done = 0;
    int result = mpg123_read(mh, (unsigned char*)shortBuffer, bytes*mChannels*sizeof(short), &done);

    switch (result)
    {
        case MPG123_DONE:
            LOG4CXX_DEBUG(narratorMsLog, "End of stream");
            break;
        case MPG123_OK:
            break;
    }

    LOG4CXX_TRACE(narratorMsLog, done << " bytes decoded");

    // convert short buffer to scaled float buffer
    float *bufptr = buffer;
    for (int i = 0; i < done/sizeof(short); i++)
    {
        int value = (int)shortBuffer[i];
        if (value == 0)
        {
            *buffer++ = 0.f;
        }
        else if (value < 0)
        {
            *buffer++ = (float)(value/scaleNegative);
        }
        else
        {
            *buffer++ = (float)(value/scalePositive);
        }
    }
    delete shortBuffer;

    return done/sizeof(short);
}

bool Mp3Stream::close()
{
    if (openTmpFile)
    {
        LOG4CXX_DEBUG(narratorMsLog, "deleting temporary file " << mTmpFile);
        if (remove(mTmpFile.c_str()) != 0) LOG4CXX_WARN(narratorMsLog, "file could not be deleted");
        openTmpFile = false;
        mTmpFile = "";
    }
    if (isOpen)
    {
        mpg123_close(mh);
        isOpen = false;
    }
    return true;
}

long Mp3Stream::getRate()
{
    return mRate;
}

long Mp3Stream::getChannels()
{
    return mChannels;
}
