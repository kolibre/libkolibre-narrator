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

#include "RingBuffer.h"
#include <cassert>
#include <malloc.h>
#include <memory.h>

#include <iostream>

RingBuffer::RingBuffer():
    buffer(0),
    maxIndex(0),
    readIndex(0),
    writeIndex(0),
    full(0)
{
    // Initialize buffer mutex
    bufMutex = new pthread_mutex_t;
    pthread_mutex_init(bufMutex, NULL);
    pthread_mutex_lock(bufMutex);
    pthread_mutex_unlock(bufMutex);
}

RingBuffer::RingBuffer(size_t elements):
    buffer(0),
    maxIndex(0),
    readIndex(0),
    writeIndex(0),
    full(0)
{
    // Initialize buffer mutex
    bufMutex = new pthread_mutex_t;
    pthread_mutex_init(bufMutex, NULL);
    pthread_mutex_lock(bufMutex);
    pthread_mutex_unlock(bufMutex);
    initialize(elements);
}

RingBuffer::~RingBuffer()
{
    pthread_mutex_unlock(bufMutex);
    pthread_mutex_destroy(bufMutex);
    free(bufMutex);
}

const size_t RingBuffer::initialize(const size_t elements)
{
    if(buffer != NULL) delete buffer;
    writeIndex = 0;
    readIndex = 0;
    maxIndex = 0;

    buffer = new float[elements];

    if(buffer == NULL) return 0;
    maxIndex = elements;

    memset(buffer, 0, elements * sizeof(float));

    return elements;
}

const size_t RingBuffer::_getReadAvailable()
{
    if(full) return maxIndex;
    if(writeIndex >= readIndex) return writeIndex - readIndex;
    return maxIndex - readIndex + writeIndex;
}

const size_t RingBuffer::getReadAvailable()
{
    size_t elements = 0;

    pthread_mutex_lock(bufMutex);
    elements = _getReadAvailable();
    pthread_mutex_unlock(bufMutex);

    return elements;
}

const size_t RingBuffer::_getWriteAvailable()
{
    if(full) return 0;
    if(writeIndex >= readIndex) return maxIndex - writeIndex + readIndex;
    return readIndex - writeIndex;
}

const size_t RingBuffer::getWriteAvailable()
{
    size_t elements = 0;

    pthread_mutex_lock(bufMutex);
    elements = _getWriteAvailable();
    pthread_mutex_unlock(bufMutex);

    return elements;
}

void RingBuffer::_advanceWriteIndex(const size_t elements)
{
    //std::cout << __FUNCTION__ << " readIndex " << readIndex << " writeIndex " << writeIndex << " elements " << elements << //std::endl;
    //std::cout << __FUNCTION__ << " maxIndex  " << maxIndex << " (writeIndex + elements) " << (writeIndex + elements) << //std::endl;
    if(writeIndex + elements < maxIndex) writeIndex += elements;
    else writeIndex = (writeIndex + elements) - maxIndex;
    //std::cout << __FUNCTION__ << " readIndex " << readIndex << " writeIndex " << writeIndex << " elements " << elements << //std::endl;
}

const size_t RingBuffer::writeElements(const float * source, size_t elements)
{
    //std::cout << __FUNCTION__ << " readIndex " << readIndex << " writeIndex " << writeIndex << //std::endl;
    pthread_mutex_lock(bufMutex);
    size_t available = _getWriteAvailable();
    size_t wIndex = writeIndex;
    pthread_mutex_unlock(bufMutex);

    if(elements > available) elements = available;

    // If write is contiguous all data can be copied at once
    if(wIndex + elements <= maxIndex) {
        //std::cout << __FUNCTION__ << " writing " << elements << " continuously at pos " << writeIndex << //std::endl;
        memcpy(buffer + wIndex, source, elements * sizeof(float));

    } // ..if not we need two separate writes
    else {
        size_t rightSize = maxIndex - wIndex;
        size_t leftSize = elements - rightSize;

        // Write right part of buffer
        //std::cout << __FUNCTION__ << " writing right part of size " << rightSize << " elements at pos " << writeIndex << //std::endl;
        memcpy(buffer + wIndex, source, rightSize * sizeof(float));

        // Write the rest to left part of buffer
        //std::cout << __FUNCTION__ << " writing left part of size " << leftSize << " elements at pos 0" << //std::endl;
        memcpy(buffer, source + rightSize, leftSize * sizeof(float));
    }

    pthread_mutex_lock(bufMutex);
    _advanceWriteIndex(elements);
    if(writeIndex == readIndex) full = true;
    pthread_mutex_unlock(bufMutex);

    return elements;
}

void RingBuffer::_advanceReadIndex(const size_t elements)
{
    //std::cout << __FUNCTION__ << " readIndex " << readIndex << " writeIndex " << writeIndex << " elements " << elements << //std::endl;
    if(readIndex + elements < maxIndex) readIndex += elements;
    else readIndex = (readIndex + elements) - maxIndex;
    //else readIndex = maxIndex - (readIndex + elements) + 1;
    //std::cout << __FUNCTION__ << " readIndex " << readIndex << " writeIndex " << writeIndex << " elements " << elements << //std::endl;
}

const size_t RingBuffer::readElements(float *data, size_t elements)
{
    //std::cout << __FUNCTION__ << " readIndex " << readIndex << " writeIndex " << writeIndex << //std::endl;
    pthread_mutex_lock(bufMutex);
    size_t available = _getReadAvailable();
    size_t rIndex = readIndex;
    pthread_mutex_unlock(bufMutex);

    if(elements > available) elements = available;

    // If read is contiguous all data can be copied at once
    if(rIndex + elements <= maxIndex) {
        //std::cout << __FUNCTION__ << " reading " << elements << " continuously at pos " << readIndex << //std::endl;
        memcpy(data, buffer + rIndex, elements * sizeof(float));

    } // ..if not we need two separate reads
    else {
        size_t rightSize = maxIndex - rIndex;
        size_t leftSize = elements - rightSize;

        // Read the right part of the buffer
        //std::cout << __FUNCTION__ << " reading right part of size " << rightSize << " elements from pos " << readIndex << //std::endl;
        memcpy(data, buffer + rIndex, rightSize * sizeof(float));

        // Read the rest from the left part the buffer
        //std::cout << __FUNCTION__ << " reading left part of size " << leftSize << " elements from pos 0" << //std::endl;
        memcpy(data + rightSize, buffer, leftSize * sizeof(float));
    }

    pthread_mutex_lock(bufMutex);
    _advanceReadIndex(elements);
    if(elements) full = false;
    pthread_mutex_unlock(bufMutex);

    return elements;
}

void RingBuffer::flush()
{
    pthread_mutex_lock(bufMutex);
    readIndex = writeIndex = 0;
    full = false;
    pthread_mutex_unlock(bufMutex);
}
