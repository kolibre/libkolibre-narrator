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
#include "setup_logging.h"

bool narratorDone = false;

void narrator_done() {
    std::cout << "narrator finished playback" << endl;
    narratorDone = true;
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cout << "run this test with e.g. " << argv[0] << " /path/to/file" << std::endl;
        return 1;
    }

    setup_logging();

    Narrator *speaker = Narrator::Instance();
    speaker->setLanguage("sv");

    boost::signals2::connection audio_finished_connection = speaker->connectAudioFinished(&narrator_done);

    // play file
    narratorDone = false;
    speaker->playFile(argv[1]);
    do { sleep(1); } while (speaker->isSpeaking());
    assert(narratorDone);

    // stop thread and delete instance before exiting
    delete speaker;

    return 0;
}
