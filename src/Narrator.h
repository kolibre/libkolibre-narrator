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

//Narrator header file

#ifndef NARRATOR_H
#define NARRATOR_H

#include <pthread.h>
#include <iostream>
#include <string>
#include <queue>
#include <map>
#include <sstream>
#include <boost/signals2.hpp>

#define NARRATOR_MIN_TEMPO 0.5
#define NARRATOR_MAX_TEMPO 2.0
#define NARRATOR_MIN_PITCH 0.5
#define NARRATOR_MAX_PITCH 2.0
#define NARRATOR_MIN_VOLUMEGAIN 0.5
#define NARRATOR_MAX_VOLUMEGAIN 2.0

using namespace std;

class Filter;
class PortAudio;
class MessageParameter;

class Narrator
{
    protected:
        Narrator();
    public:
        //Define signals and slot types
        typedef boost::signals2::signal<void ()> AudioFinished;
        typedef AudioFinished::slot_type AudioFinishedSlotType;

        static Narrator *Instance();
        ~Narrator();

        void play(const char *identifier);
        void play(int number);
        void playFile(const string filepath);
        void playDate(int day, int month, int year);
        void playTime(int hour, int minute, int second);
        void playDuration(int seconds, int minutes, int hours);
        void playDuration(long seconds);
        void playResource(string str, string cls);
        void playLongpause();
        void playShortpause();
        void playWait();
        void spell(string word);

        void stop();
        void printMessages();

        bool isSpeaking();
        string getState_str();

        // If set to true narrator will push a COMMAND_NARRATORFINISHED onto the commandqueue
        // when all the prompts have been played back
        void setPushCommandFinished(bool);
        bool getPushCommandFinished();

        // Connect to audiofinished
        boost::signals2::connection connectAudioFinished(const AudioFinishedSlotType &slot);

        float getVolumeGain();
        void setVolumeGain(float);
        void adjustVolumeGain(float);

        float getTempo();
        void setTempo(float);
        void adjustTempo(float);

        float getPitch();
        void setPitch(float);
        void adjustPitch(float);

        void setParameter(const string &key, int value);
        void setParameter(const string &key, const string &value);

        bool setLanguage(string lang);
        string getLanguage();

        void setDatabasePath(string path);
        string getDatabasePath();

        // Functions to find and insert audio
        bool hasOggAudio(const char *identifier);
        bool hasMp3Audio(const char *identifier);
        bool addOggAudio(const char *identifier, const char *data, int size);
        bool addMp3Audio(const char *identifier, const char *data, int size);

    private:
        enum threadState { DEAD, WAIT, PLAY, RESET, EXIT };

        static Narrator *pinstance;

        bool setupThread();
        /*! \cond PRIVATE */
        friend void adjustGainTempoPitch( Narrator* n, Filter& filter, float& gain, float& tempo, float& pitch, Narrator::threadState& state );
        friend void writeSamplesToPortaudio( Narrator* n, PortAudio& portaudio, Filter& filter, float* buffer );
        friend void *narrator_thread(void *narrator);
        /*! \endcond */
        pthread_mutex_t *narratorMutex;
        pthread_t playbackThread;

        string mLanguage;
        string mDatabasePath;

        float mVolumeGain;
        float mTempo;
        float mPitch;

        bool bPushCommandFinished;

        enum ItemType { type_unknown, type_message, type_resource };

        struct PlaylistItem {
            ItemType mType;
            string mIdentifier;
            string mClass;
            vector <MessageParameter>vParameters;
        };

        int numPlaylistItems();
        queue <PlaylistItem> mPlaylist;

        vector <MessageParameter>vParameters;

        void setState(threadState state);
        threadState getState();
        threadState mState;

        void audioFinishedPlaying();

        AudioFinished m_signal_audio_finished;
};

#endif
