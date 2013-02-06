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
#include "setup_logging.h"

int sleepDuration = 1;
bool narratorDone = false;

void narrator_done() {
    std::cout << "narrator finished playback" << endl;
    narratorDone = true;
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        std::cout << "run this test with e.g. " << argv[0] << " sv database.db" << std::endl;
        return 1;
    }

    setup_logging();

    Narrator *speaker = Narrator::Instance();

    speaker->connectAudioFinished(&narrator_done);

    /*
     * getters and setters
     */

    // test set language
    speaker->setLanguage(argv[1]);
    assert(speaker->getLanguage() == argv[1]);

    // test set database
    speaker->setDatabasePath(argv[2]);
    assert(speaker->getDatabasePath() == argv[2]);

    // test set gain
    speaker->setVolumeGain(1.5);
    assert(speaker->getVolumeGain() == 1.5);
    speaker->setVolumeGain(2);
    assert(speaker->getVolumeGain() == 1.5);
    speaker->setVolumeGain(0.5);
    assert(speaker->getVolumeGain() == 0.5);
    speaker->setVolumeGain(0);
    assert(speaker->getVolumeGain() == 0.5);
    speaker->adjustVolumeGain(0.5);
    assert(speaker->getVolumeGain() == 1.0);
    speaker->adjustVolumeGain(-1);
    assert(speaker->getVolumeGain() == 0.5);
    speaker->adjustVolumeGain(2);
    assert(speaker->getVolumeGain() == 1.5);
    speaker->setVolumeGain(1);
    assert(speaker->getVolumeGain() == 1.0);

    // test set tempo
    speaker->setTempo(1.5);
    assert(speaker->getTempo() == 1.5);
    speaker->setTempo(2);
    assert(speaker->getTempo() == 1.5);
    speaker->setTempo(0.5);
    assert(speaker->getTempo() == 0.5);
    speaker->setTempo(0);
    assert(speaker->getTempo() == 0.5);
    speaker->adjustTempo(0.5);
    assert(speaker->getTempo() == 1.0);
    speaker->adjustTempo(-1);
    assert(speaker->getTempo() == 0.5);
    speaker->adjustTempo(2);
    assert(speaker->getTempo() == 1.5);
    speaker->setTempo(1);
    assert(speaker->getTempo() == 1.0);

    // test set pitch
    speaker->setPitch(1.5);
    assert(speaker->getPitch() == 1.5);
    speaker->setPitch(2);
    assert(speaker->getPitch() == 1.5);
    speaker->setPitch(0.5);
    assert(speaker->getPitch() == 0.5);
    speaker->setPitch(0);
    assert(speaker->getPitch() == 0.5);
    speaker->adjustPitch(0.5);
    assert(speaker->getPitch() == 1.0);
    speaker->adjustPitch(-1);
    assert(speaker->getPitch() == 0.5);
    speaker->adjustPitch(2);
    assert(speaker->getPitch() == 1.5);
    speaker->setPitch(1);
    assert(speaker->getPitch() == 1.0);

    /*
     * play interface
     */

    // test play identifier
    narratorDone = false;
    speaker->play("Monday");
    do { sleep(sleepDuration); } while (speaker->isSpeaking());
    assert(narratorDone);

    // test play file
    narratorDone = false;
    char* srcdir = getenv("srcdir");
    if(!srcdir)
        srcdir = ".";
    string file = string(srcdir) + string("/aktuell_sida.ogg");
    speaker->playFile(file);
    do { sleep(sleepDuration); } while (speaker->isSpeaking());
    assert(narratorDone);

    // test play date
    narratorDone = false;
    speaker->playDate(1,1,1970);
    do { sleep(sleepDuration); } while (speaker->isSpeaking());
    assert(narratorDone);

    // test play time
    narratorDone = false;
    speaker->playTime(12,12,12);
    do { sleep(sleepDuration); } while (speaker->isSpeaking());
    assert(narratorDone);

    // test play duration
    narratorDone = false;
    speaker->playDuration(12,12,12);
    do { sleep(sleepDuration); } while (speaker->isSpeaking());
    assert(narratorDone);

    // test play duration
    narratorDone = false;
    speaker->playDuration(12);
    do { sleep(sleepDuration); } while (speaker->isSpeaking());
    assert(narratorDone);

    // test play resource
    narratorDone = false;
    speaker->playResource("Monday", "prompt", 1);
    do { sleep(sleepDuration); } while (speaker->isSpeaking());
    assert(narratorDone);

    // test play identifier with parameter
    narratorDone = false;
    speaker->setParameter("2", 12);
    speaker->play("{2} minutes");
    do { sleep(sleepDuration); } while (speaker->isSpeaking());
    assert(narratorDone);

    // test play pauses
    narratorDone = false;
    speaker->playLongpause();
    do { sleep(sleepDuration); } while (speaker->isSpeaking());
    assert(narratorDone);
    narratorDone = false;
    speaker->playShortpause();
    do { sleep(sleepDuration); } while (speaker->isSpeaking());
    assert(narratorDone);

    // test play wait jingle
    narratorDone = false;
    speaker->playWait();
    do { sleep(sleepDuration); } while (speaker->isSpeaking());
    assert(narratorDone);

    //void setParameter(const string &key, const string &value);

    // test set push command finished
    narratorDone = false;
    speaker->setPushCommandFinished(false);
    assert(speaker->getPushCommandFinished() == false);
    speaker->play("Monday");
    do { sleep(sleepDuration); } while (speaker->isSpeaking());
    assert(narratorDone == false);

    return 0;
}
