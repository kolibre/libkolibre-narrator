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

#ifndef _RINGBUFFER_H
#define _RINGBUFFER_H

#include <pthread.h>

class RingBuffer {
    public:
        RingBuffer();
        RingBuffer(size_t elements);
        ~RingBuffer();

        // Initializes a buffer of size elements
        const size_t initialize(const size_t elements);

        // Returns number of elements available for read
        const size_t getReadAvailable();

        // Returns number of elements available for write
        const size_t getWriteAvailable();

        // Writes specified number of elements into ringbuffer from source, returns number of elements actually written
        const size_t writeElements(const float * data, size_t elements);

        // Reads specified number of elements from ringbuffer into target, returns number of elements actually read
        const size_t readElements(float *data, size_t elements);

        // Flushes all data in ringbuffer
        void flush();

    private:
        const size_t _getReadAvailable();
        const size_t _getWriteAvailable();

        void _advanceReadIndex(const size_t elements);
        void _advanceWriteIndex(const size_t elements);

        float *buffer;
        pthread_mutex_t *bufMutex;

        size_t maxIndex;
        size_t readIndex;
        size_t writeIndex;
        bool full;

};

#endif
