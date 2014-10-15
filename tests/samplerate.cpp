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

#define DATABASE "./samplerate.db"

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

    // insert first file to database
    char *data = NULL;
    int size = readData(argv[1], &data);
    assert(size>=0);
    std::string extension = getFileExtension(argv[1]);
    if (extension == "ogg")
    {
        bool result = speaker->addOggAudio(argv[1], data, size);
        assert(result);
        assert(speaker->hasOggAudio(argv[1]));
    }
    else if (extension == "mp3")
    {
        bool result = speaker->addMp3Audio(argv[1], data, size);
        assert(result);
        assert(speaker->hasMp3Audio(argv[1]));
    }
    free(data);

    narratorDone = false;
    // play first file from database
    speaker->play(argv[1]);
    // play second file from file system
    speaker->playFile(argv[2]);
    // play first file from database
    speaker->play(argv[1]);
    do { sleep(1); } while (speaker->isSpeaking());
    assert(narratorDone);

    // stop thread and delete instance before exiting
    delete speaker;

    // delete database
    remove(DATABASE);

    return 0;
}
