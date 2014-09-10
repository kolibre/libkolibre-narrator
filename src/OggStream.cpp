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
#include "OggStream.h"

#include <sstream>
#include <log4cxx/logger.h>

// create logger which will become a child to logger kolibre.narrator
log4cxx::LoggerPtr narratorOsLog(log4cxx::Logger::getLogger("kolibre.narrator.oggstream"));

OggStream::OggStream()
{
    LOG4CXX_INFO(narratorOsLog, "Initializing oggstream");

    mChannels = 0;
    mRate = 0;
    mSection = 0;
    isOpen = false;
}

OggStream::~OggStream()
{
    if(isOpen) close();
}

bool OggStream::open(const MessageAudio &ma)
{
    mCallbacks.read_func = MessageAudio_read;
    mCallbacks.close_func = MessageAudio_close;
    mCallbacks.seek_func = MessageAudio_seek;
    mCallbacks.tell_func = MessageAudio_tell;

    currentAudio = ma;
    int audioid = ma.getAudioid();
    int error = ov_open_callbacks(&currentAudio, &mStream, NULL, 0, mCallbacks);

    ostringstream oss;
    oss << "MessageAudio:" << audioid;
    mStreamInfo = oss.str();

    if(error) {
        switch(error) {
            case OV_EREAD:
                LOG4CXX_ERROR(narratorOsLog, "MessageAudio id: " << audioid << " A read from media returned an error");
                break;
            case OV_ENOTVORBIS:
                LOG4CXX_ERROR(narratorOsLog, "MessageAudio id: " << audioid << " Bitstream is not Vorbis data");
                break;
            case OV_EVERSION:
                LOG4CXX_ERROR(narratorOsLog, "MessageAudio id: " << audioid << " Vorbis version mismatch");
                break;
            case OV_EBADHEADER:
                LOG4CXX_ERROR(narratorOsLog, "MessageAudio id: " << audioid << " Invalid Vorbis bitstream header");
                break;
            case OV_EFAULT:
                LOG4CXX_ERROR(narratorOsLog, "MessageAudio id: " << audioid<< " Internal logic fault");
                break;
            default:
                LOG4CXX_ERROR(narratorOsLog, "MessageAudio id: " << audioid << " unknown error occurred");
                break;
        }
        return false;
    }

    vorbis_info *vi = ov_info(&mStream,-1);

    isOpen = true;
    mChannels = vi->channels;
    mRate = vi->rate;

    /*
       char **ptr = ov_comment(&mStream,-1)->user_comments;
       while(*ptr){
       LOG4CXX_DEBUG(narratorOsLog, "comment: " << *ptr);
       ++ptr;
       }

       size_t total_size = ov_pcm_total(&mStream, -1);
       LOG4CXX_DEBUG(narratorOsLog, "total size: " << total_size);
       */
    return true;
}

bool OggStream::open(string path)
{
    int error = ov_fopen((char*)path.c_str(), &mStream);

    ostringstream oss;
    oss << "AudioFile:" << path;
    mStreamInfo = oss.str();

    if(error) {
        switch(error)
        {
            case OV_EREAD:
                LOG4CXX_ERROR(narratorOsLog, "File " << path << ": A read from media returned an error");
                break;
            case OV_ENOTVORBIS:
                LOG4CXX_ERROR(narratorOsLog, "File " << path << ": Bitstream is not Vorbis data");
                break;
            case OV_EVERSION:
                LOG4CXX_ERROR(narratorOsLog, "File " << path << ": Vorbis version mismatch");
                break;
            case OV_EBADHEADER:
                LOG4CXX_ERROR(narratorOsLog, "File " << path << ": Invalid Vorbis bitstream header");
                break;
            case OV_EFAULT:
                LOG4CXX_ERROR(narratorOsLog, "File " << path << ": Internal logic fault");
                break;
            default:
                LOG4CXX_ERROR(narratorOsLog, "File " << path << ": unknown error occurred");
                break;
        }
        return false;
    }

    vorbis_info *vi = ov_info(&mStream,-1);

    isOpen = true;
    mChannels = vi->channels;
    mRate = vi->rate;

    /*
       char **ptr = ov_comment(&mStream,-1)->user_comments;
       while(*ptr){
       LOG4CXX_DEBUG(narratorOsLog, "comment: " << *ptr);
       ++ptr;
       }

       size_t total_size = ov_pcm_total(&mStream, -1);
       LOG4CXX_DEBUG(narratorOsLog, "total size: " << total_size);
    */
    return true;
}

// Returns samples (1 sample contains data from all channels)
long OggStream::read(float* buffer, int bytes)
{
    float **pcm;
    long samples_read = ov_read_float(&mStream, &pcm, bytes, &mSection);
    switch(samples_read) {
        case OV_HOLE:
            LOG4CXX_WARN(narratorOsLog, "Interruption in data while playing " << mStreamInfo);
            break;
        case OV_EBADLINK:
            LOG4CXX_WARN(narratorOsLog, "Invalid stream section was supplied while playing " << mStreamInfo);
            break;
    }
    //Convert the samples to a linear vector
    float *bufptr = buffer;
    for (long i = 0; i < samples_read; i++)
        for(int c = 0; c < mChannels; c++)
            *bufptr++ = pcm[c][i];
    return (samples_read)/mChannels;
}

bool OggStream::close()
{
    if(isOpen) {
        ov_clear(&mStream);
        isOpen = false;
    }
    return true;
}
