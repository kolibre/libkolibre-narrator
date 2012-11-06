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
#include <MessageHandler.h>

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

/*
 * try building Message, MessageTranslation and MessageAudio object,
 * and insert/update messages database
 */
int insert_new_message(std::string audio_file_str, std::string message_text, int labelId, std::string lang = "sv", std::string labelClass = "string") {

    // create MessageAudio object
    MessageAudio messageAudio;
    messageAudio.setTagid( 0 );
    messageAudio.setText( message_text );
    messageAudio.setSize( 0 );
    messageAudio.setLength( 0 ); // not available via DO interface
    messageAudio.setMd5( "" ); // not available via DO inteface
    messageAudio.setUri( "" );

    // create MessageTranslation object
    MessageTranslation messageTranslation;
    messageTranslation.setLanguage( lang );
    messageTranslation.setText( message_text );
    messageTranslation.setAudiotags( "[0]" );

    // create Message object
    Message message;
    message.setString( message_text );
    message.setClass( labelClass );
    message.setId( labelId );

    // use functions in MessageHandler to find/insert messages
    MessageHandler mh;
    long messageid = mh.findMessage(message);


    // download audio data and insert message
    if( messageid < 0 )
    {
        // download audio
        char *audio_data = NULL;
        int audio_size = readData(audio_file_str, &audio_data);

        if (audio_size > 0)
        {
            messageAudio.setAudioData(audio_data, audio_size);
            messageTranslation.addAudio(messageAudio);
            message.setTranslation(messageTranslation);

            // insert message
            messageid = mh.updateMessage(message);
            free(audio_data);

            if( messageid < 0 )
            {
                // MessageHandler writes error messages to log in case of failure
                cout << "Inserting message " << message_text << " FAILED." << endl;
                return -1;
            }
        }
        else
        {
            cout << "Downloading audio data FAILED." << endl;
            return -1;
        }

    }
    return messageid;
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

    std::string file, message_text;
    char* srcdir = getenv("srcdir");
    if(!srcdir)
        srcdir = ".";
    file = string(srcdir) + string("/aktuell_sida.ogg");
    message_text = "Aktuell sida";
    int labelId = 1;
    int resourceId = insert_new_message(file, message_text, labelId);
    cout << "Insert Aktuell sida " << resourceId << endl;
    labelId++;

    speaker->play(message_text.c_str());
    do { sleep(1); } while (speaker->isSpeaking());
    assert(narratorDone);

    return 0;
}
