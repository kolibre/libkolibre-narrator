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

#include <unistd.h>
#include <RingBuffer.h>
#include "setup_logging.h"
#include <iostream>
#include <cassert>
#include <cstdlib>

using namespace std;

#define BUFFER_SIZE 10

int getRand(){
    return rand() % BUFFER_SIZE + 1;
}

int main(int argc, char **argv)
{
    setup_logging();

    RingBuffer ringbuf(BUFFER_SIZE);

    int count;
    float elements[] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18};

    size_t space;
    size_t elemsLeft;
    for(int i=1; i<=BUFFER_SIZE; i++){
        ringbuf.writeElements(elements, i);
        space = ringbuf.getWriteAvailable();
        ringbuf.writeElements(elements, space);

        float readElements[BUFFER_SIZE] = {0};
        count = ringbuf.readElements(readElements, i);
        assert(count == i);
        /*for(int j=0; j<i; j++){
            cout << readElements[j] << ", ";
        }*/


        float readElements2[BUFFER_SIZE] = {0};
        elemsLeft = ringbuf.getReadAvailable();
        count = ringbuf.readElements(readElements2, elemsLeft);
        assert(count == elemsLeft);
        /*for(int j=0; j<elemsLeft; j++){
            cout << readElements2[j] << ", ";
        }
        cout << endl;*/
    }

    //cout << endl;

    //Write more than there is space
    for(int i=1; i<=BUFFER_SIZE; i++){
        ringbuf.writeElements(elements, i);
        space = 10;
        ringbuf.writeElements(elements, space);

        float readElements[BUFFER_SIZE] = {0};
        count = ringbuf.readElements(readElements, i);
        assert(count == i);
        /*for(int j=0; j<i; j++){
            cout << readElements[j] << ", ";
        }*/


        float readElements2[BUFFER_SIZE] = {0};
        elemsLeft = ringbuf.getReadAvailable();
        count = ringbuf.readElements(readElements2, elemsLeft);
        assert(count == elemsLeft);
        /*for(int j=0; j<elemsLeft; j++){
            cout << readElements2[j] << ", ";
        }
        cout << endl;*/
    }

    //cout << endl;


    //Read more than can be found
    for(int i=1; i<=BUFFER_SIZE/2; i++){
        ringbuf.writeElements(elements, i);
        ringbuf.writeElements(elements, i);

        float readElements2[BUFFER_SIZE] = {0};
        elemsLeft = ringbuf.getReadAvailable();
        count = ringbuf.readElements(readElements2, BUFFER_SIZE);
        assert(elemsLeft == count);
        /*for(int j=0; j<BUFFER_SIZE; j++){
            cout << readElements2[j] << ", ";
        }
        cout << endl;*/
    }

    //cout << endl;

    //Write and read random to buffer
    srand(0);
    int randInt;
    float theLine[1000*BUFFER_SIZE] = {0};
    float *linePointer = &theLine[0];
    float *lineCheckPointer = &theLine[0];
    for(int i=1; i<=100; i++){
        randInt = getRand();
        if(randInt > ringbuf.getWriteAvailable())
            randInt = ringbuf.getWriteAvailable();
        assert(ringbuf.writeElements(elements, randInt) == randInt);
        for(int k=0;k<randInt;k++, linePointer++){
            *linePointer = elements[k];

        }

        float readElements[BUFFER_SIZE] = {0};
        randInt = getRand();
        if(randInt > ringbuf.getReadAvailable())
            randInt = ringbuf.getReadAvailable();
        count = ringbuf.readElements(readElements, randInt);
        //cout << "(" << count << "," << randInt << ")";
        assert(randInt == count);
        for(int j=0; j<count; j++, lineCheckPointer++){
            //cout << readElements[j] << "(" << *lineCheckPointer <<"), ";
            assert(*lineCheckPointer == readElements[j]);
        }
        //cout << endl;

    }

    return 0;
}
