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

#ifndef _AUDIOSYSTEM_H
#define _AUDIOSYSTEM_H

class AudioSystem {
    public:
        virtual ~AudioSystem() {}

        // Opens a new output stream, reuses the old one in case rate and channels match
        virtual bool open(long rate, int channels) = 0;
        virtual long stop() = 0;
        virtual long abort() = 0;
        virtual bool close() = 0;

        // Checks how many samples we can write
        virtual unsigned int getWriteAvailable() = 0;

        // Return remaining audio in internal buffers (ms)
        virtual long getRemainingms() = 0;

        // Writes no more than getWriteAvailable samples
        virtual bool write(float *buffer, unsigned int samples) = 0;
};

#endif
