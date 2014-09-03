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

#include <log4cxx/logger.h>

// create logger which will become a child to logger kolibre.narrator
log4cxx::LoggerPtr narratorPuaLog(log4cxx::Logger::getLogger("kolibre.narrator.pulseaudio"));

PulseAudio::PulseAudio()
{
}

PulseAudio::~PulseAudio()
{
}

bool PulseAudio::open(long rate, int channels)
{
    return false;
}

long PulseAudio::stop()
{
    return 0;
}

long PulseAudio::abort()
{
    return 0;
}

bool PulseAudio::close()
{
    return false;
}

unsigned int PulseAudio::getWriteAvailable()
{
    return 0;
}

long PulseAudio::getRemainingms()
{
    return 0;
}

bool PulseAudio::write(float *buffer, unsigned int samples)
{
    return false;
}
