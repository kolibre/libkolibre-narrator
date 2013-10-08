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

#include <Narrator.h>
#include <iostream>
#include "setup_logging.h"

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

    float defaultValue;
    float max, gtmax, ltmax;
    float min, gtmin, ltmin;

    // test set gain with border values
    defaultValue = speaker->getVolumeGain();
    max = NARRATOR_MAX_VOLUMEGAIN;
    gtmax = (float)(max + 0.1);
    ltmax = (float)(max - 0.1);
    min = NARRATOR_MIN_VOLUMEGAIN;
    gtmin = (float)(min + 0.1);
    ltmin = (float)(min - 0.1);

    speaker->setVolumeGain(max);
    assert(speaker->getVolumeGain() == max);
    speaker->setVolumeGain(gtmax);
    assert(speaker->getVolumeGain() == max);
    speaker->setVolumeGain(ltmax);
    assert(speaker->getVolumeGain() == ltmax);

    speaker->setVolumeGain(min);
    assert(speaker->getVolumeGain() == min);
    speaker->setVolumeGain(ltmin);
    assert(speaker->getVolumeGain() == min);
    speaker->setVolumeGain(gtmin);
    assert(speaker->getVolumeGain() == gtmin);

    speaker->setVolumeGain(defaultValue);
    assert(speaker->getVolumeGain() == defaultValue);

    // test set tempo with border values
    defaultValue = speaker->getTempo();
    max = NARRATOR_MAX_TEMPO;
    gtmax = (float)(max + 0.1);
    ltmax = (float)(max - 0.1);
    min = NARRATOR_MIN_TEMPO;
    gtmin = (float)(min + 0.1);
    ltmin = (float)(min - 0.1);

    speaker->setTempo(max);
    assert(speaker->getTempo() == max);
    speaker->setTempo(gtmax);
    assert(speaker->getTempo() == max);
    speaker->setTempo(ltmax);
    assert(speaker->getTempo() == ltmax);

    speaker->setTempo(min);
    assert(speaker->getTempo() == min);
    speaker->setTempo(ltmin);
    assert(speaker->getTempo() == min);
    speaker->setTempo(gtmin);
    assert(speaker->getTempo() == gtmin);

    speaker->setTempo(defaultValue);
    assert(speaker->getTempo() == defaultValue);

    // test set pitch with border values
    defaultValue = speaker->getPitch();
    max = NARRATOR_MAX_PITCH;
    gtmax = (float)(max + 0.1);
    ltmax = (float)(max - 0.1);
    min = NARRATOR_MIN_PITCH;
    gtmin = (float)(min + 0.1);
    ltmin = (float)(min - 0.1);

    speaker->setPitch(max);
    assert(speaker->getPitch() == max);
    speaker->setPitch(gtmax);
    assert(speaker->getPitch() == max);
    speaker->setPitch(ltmax);
    assert(speaker->getPitch() == ltmax);

    speaker->setPitch(min);
    assert(speaker->getPitch() == min);
    speaker->setPitch(ltmin);
    assert(speaker->getPitch() == min);
    speaker->setPitch(gtmin);
    assert(speaker->getPitch() == gtmin);

    speaker->setPitch(defaultValue);
    assert(speaker->getPitch() == defaultValue);

    /*
     * play interface
     */

    // test play identifier
    narratorDone = false;
    speaker->play("Monday");
    while (speaker->isSpeaking());
    assert(narratorDone);

    narratorDone = false;
    speaker->play(1);
    while (speaker->isSpeaking());
    assert(narratorDone);

    // test play file
    narratorDone = false;
    char* srcdir = getenv("srcdir");
    if(!srcdir)
        srcdir = ".";
    string file = string(srcdir) + string("/testdata/file1.ogg");
    speaker->playFile(file);
    while (speaker->isSpeaking());
    assert(narratorDone);

    // test play date
    narratorDone = false;
    speaker->playDate(1,1,1970);
    while (speaker->isSpeaking());
    assert(narratorDone);

    // test play time
    narratorDone = false;
    speaker->playTime(12,12,12);
    while (speaker->isSpeaking());
    assert(narratorDone);

    // test play duration
    narratorDone = false;
    speaker->playDuration(12,12,12);
    while (speaker->isSpeaking());
    assert(narratorDone);

    // test play duration
    narratorDone = false;
    speaker->playDuration(12);
    while (speaker->isSpeaking());
    assert(narratorDone);

    // test play resource
    narratorDone = false;
    speaker->playResource("Monday", "prompt");
    while (speaker->isSpeaking());
    assert(narratorDone);

    // test play identifier with parameter
    narratorDone = false;
    speaker->setParameter("2", 12);
    speaker->play("{2} minutes");
    while (speaker->isSpeaking());
    assert(narratorDone);

    // test play pauses
    narratorDone = false;
    speaker->playLongpause();
    while (speaker->isSpeaking());
    assert(narratorDone);
    narratorDone = false;
    speaker->playShortpause();
    while (speaker->isSpeaking());
    assert(narratorDone);

    // test play wait jingle
    narratorDone = false;
    speaker->playWait();
    while (speaker->isSpeaking());
    assert(narratorDone);

    /*
     * spell interface
     */

    // test spell
    narratorDone = false;
    speaker->spell("aBcXyZ012890");
    speaker->spell("http://google.com");
    speaker->spell("info@kolibre.org");
    speaker->spell("1.2.3~4");
    speaker->spell("+5 -6");
    while (speaker->isSpeaking());
    assert(narratorDone);

    //void setParameter(const string &key, const string &value);

    // test set push command finished
    narratorDone = false;
    speaker->setPushCommandFinished(false);
    assert(speaker->getPushCommandFinished() == false);
    speaker->play("Monday");
    while (speaker->isSpeaking());
    assert(narratorDone == false);

    // stop thread and delete instance before exiting
    delete speaker;

    return 0;
}
