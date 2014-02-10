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
#include <Narrator.h>
#include <iostream>
#include <fstream>
#include "setup_logging.h"

bool narratorDone = false;

void narrator_done() {
    std::cout << "narrator finished playback" << endl;
    narratorDone = true;
}

int readData(std::string file, char ** buffer){
    int length;

    ifstream is;
    try {
        is.open (file.c_str(), ios::binary );
        if (is.fail()){
            cout << "Could not open file: " << file << endl;
            return -1;
        }
        // get length of file:
        is.seekg (0, ios::end);
        length = is.tellg();
        is.seekg (0, ios::beg);

        // allocate memory:
        *buffer = new char [length];

        // read data as a block:
        is.read (*buffer,length);
        is.close();
    }
    catch (ifstream::failure e) {
        cout << "Exception opening/reading file";
        return -1;
    }
    return length;
}

int main(int argc, char **argv)
{
    setup_logging();

    Narrator *speaker = Narrator::Instance();
    narratorDone = false;
    speaker->connectAudioFinished(&narrator_done);
    speaker->setDatabasePath("./empty.db");
    speaker->setLanguage("sv");

    speaker->play(10);
    usleep(500);
    assert(!narratorDone);

    std::string file, identifier;
    string srcdir = getenv("srcdir");
    if(srcdir.compare(""))
        srcdir = ".";
    file = srcdir + string("/testdata/file1.ogg");
    identifier = "File one";

    /*
     * try inserting ogg audio -> expect successful insert
     */

    if (!speaker->hasOggAudio(identifier.c_str()))
    {
        // add ogg audio to database
        char *data = NULL;
        int size = readData(file, &data);
        assert(size>=0);
        bool result = speaker->addOggAudio(identifier.c_str(), data, size);
        free(data);
        assert(result);
    }
    assert(speaker->hasOggAudio(identifier.c_str()));

    // play added audio
    speaker->play(identifier.c_str());
    while (speaker->isSpeaking());
    assert(narratorDone);

    /*
     * try inserting audio again with different identifier -> expect succussful insert
     */

    identifier = "file One";
    if (!speaker->hasOggAudio(identifier.c_str()))
    {
        // add ogg audio to database
        char *data = NULL;
        int size = readData(file, &data);
        assert(size>=0);
        bool result = speaker->addOggAudio(identifier.c_str(), data, size);
        free(data);
        assert(result);
    }
    assert(speaker->hasOggAudio(identifier.c_str()));

    // play added audio
    speaker->play(identifier.c_str());
    while (speaker->isSpeaking());
    assert(narratorDone);

    /*
     * try inserting audio again with different identifier -> expect succussful insert
     */

    identifier = "File One";
    if (!speaker->hasOggAudio(identifier.c_str()))
    {
        // add ogg audio to database
        char *data = NULL;
        int size = readData(file, &data);
        assert(size>=0);
        bool result = speaker->addOggAudio(identifier.c_str(), data, size);
        free(data);
        assert(result);
    }
    assert(speaker->hasOggAudio(identifier.c_str()));

    // play added audio
    speaker->play(identifier.c_str());
    while (speaker->isSpeaking());
    assert(narratorDone);

    // stop thread and delete instance before exiting
    delete speaker;

    return 0;
}
