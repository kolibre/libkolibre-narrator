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
#include <algorithm>
#include <cstdio>
#include "setup_logging.h"

bool narratorDone = false;

#define DATABASE "./dbtest.db"

void narrator_done() {
    std::cout << "narrator finished playback" << endl;
    narratorDone = true;
}

std::string getFileExtension(const std::string& filename)
{
    int start = filename.length() - 3;
    if (start < 0) return "";
    std::string ext = filename.substr(start, filename.length());
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return ext;
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
    if (argc < 3)
    {
        std::cout << "run this test with e.g. " << argv[0] << " /path/to/file identifier" << std::endl;
        return 1;
    }

    setup_logging();

    Narrator *speaker = Narrator::Instance();
    narratorDone = false;
    speaker->connectAudioFinished(&narrator_done);
    speaker->setDatabasePath(DATABASE);
    speaker->setLanguage("sv");

    std::string extension = getFileExtension(argv[1]);

    /*
     * try inserting audio -> expect successful insert
     */

    if (extension == "ogg")
    {
        if (!speaker->hasOggAudio(argv[2]))
        {
            // add ogg audio to database
            char *data = NULL;
            int size = readData(argv[1], &data);
            assert(size>=0);
            bool result = speaker->addOggAudio(argv[2], data, size);
            free(data);
            assert(result);
        }
        assert(speaker->hasOggAudio(argv[2]));
    }
    else if (extension == "mp3")
    {
        if (!speaker->hasMp3Audio(argv[2]))
        {
            // add ogg audio to database
            char *data = NULL;
            int size = readData(argv[1], &data);
            assert(size>=0);
            bool result = speaker->addMp3Audio(argv[2], data, size);
            free(data);
            assert(result);
        }
        assert(speaker->hasMp3Audio(argv[2]));
    }

    // play added audio
    speaker->play(argv[2]);
    while (speaker->isSpeaking());
    assert(narratorDone);

    /*
     * try inserting audio again with different identifier -> expect succussful insert
     */

    if (extension == "ogg")
    {
        if (!speaker->hasOggAudio("new identifier"))
        {
            // add ogg audio to database
            char *data = NULL;
            int size = readData(argv[1], &data);
            assert(size>=0);
            bool result = speaker->addOggAudio("new identifier", data, size);
            free(data);
            assert(result);
        }
        assert(speaker->hasOggAudio("new identifier"));
    }
    else if (extension == "mp3")
    {
        if (!speaker->hasMp3Audio("new identifier"))
        {
            // add ogg audio to database
            char *data = NULL;
            int size = readData(argv[1], &data);
            assert(size>=0);
            bool result = speaker->addMp3Audio("new identifier", data, size);
            free(data);
            assert(result);
        }
        assert(speaker->hasMp3Audio("new identifier"));
    }

    // play added audio
    speaker->play("new identifier");
    while (speaker->isSpeaking());
    assert(narratorDone);

    // stop thread and delete instance before exiting
    delete speaker;

    // delete database
    remove(DATABASE);

    return 0;
}
