/*
Copyright (C) 2012  The Kolibre Foundation

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


class NarratorControl
{
    public:
        Narrator *speaker;

        NarratorControl();

        void narratorDoneSlot();
        void stress();
};

NarratorControl::NarratorControl():
    speaker(Narrator::Instance())
{
    speaker->connectAudioFinished( boost::bind(&NarratorControl::narratorDoneSlot, this));
    speaker->setDatabasePath("./stress.db");
    speaker->setLanguage("sv");
    speaker->setTempo(1.5);

}

void NarratorControl::narratorDoneSlot() {
    std::cout << "narrator finished playback" << endl;
}

void NarratorControl::stress() {
    int counter = 0;
    while(counter++ < 150) {
        speaker->play("Monday");
        speaker->playDate(1,1,1970);
        speaker->playTime(12,12,12);
        speaker->playDuration(12,12,12);
        speaker->playDuration(12,12,12);
        speaker->playDate(1,1,1970);
        speaker->playDuration(12,12,12);
        sleep(1);
        speaker->stop();
    }
}

int main(int argc, char **argv)
{
    setup_logging();

    NarratorControl narratorcontrol; 
    narratorcontrol.stress();
    sleep(1);
    assert(!narratorcontrol.speaker->isSpeaking());
    return 0;
}
