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

/**
 * \class Narrator
 *
 * \brief This is main interface for the kolibre narrator. It takes care of playback of audio prompts in different languages. When using the Narrator, this class is the only one you should need to include.
 *
 * \note Remeber to register slots for the signals that the narrator sends.
 *
 * \author Kolibre (www.kolibre.org)
 *
 * Contact: info@kolibre.org
 *
 */

#include <string>
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <sstream>

using namespace std;

#define BUFFERSIZE 4096

#include "Narrator.h"
#include "OggStream.h"
#include "PortAudio.h"
#include "Filter.h"
#include "Message.h"
#include "MessageHandler.h"
#include <cmath>
#include <unistd.h>
#include <log4cxx/logger.h>
#include "Db.h"

#ifdef _WIN32 /* We need the following two to set stdin/stdout to binary */
#include <io.h>
#include <fcntl.h>
#endif
#include <stdio.h>

// create logger which will become a child to logger kolibre.narrator
log4cxx::LoggerPtr narratorLog(log4cxx::Logger::getLogger("kolibre.narrator.narrator"));

Narrator * Narrator::pinstance = 0;

void *narrator_thread(void *narrator) ;

/**
 * Get the narrator instance and create if it does not exist
 */
Narrator * Narrator::Instance()
{
    if(pinstance == 0) {
        pinstance = new Narrator;
    }

    return pinstance;
}


/**
 * Init narrator and setup default variables.
 */
Narrator::Narrator()
{
    mState = Narrator::DEAD;

    // Setup mutex variable
    narratorMutex = (pthread_mutex_t *) malloc (sizeof (pthread_mutex_t));
    pthread_mutex_init (narratorMutex, NULL);

    mVolumeGain = 1.0;
    mPitch = 1.0;
    mTempo = 1.0;

    mLanguage = "en";
    mDatabasePath = "";
    bPushCommandFinished = true;
    bResetFlag = false;
    nextMessage = NULL;

    pthread_mutex_lock(narratorMutex);
    pthread_mutex_unlock(narratorMutex);

    if(setupThread() == false) {
        LOG4CXX_ERROR(narratorLog, "Failed to initialize thread");
    }

}

/**
 * Free the filevector
 */
Narrator::~Narrator()
{
    LOG4CXX_TRACE(narratorLog, "Destructor");

    // Tell the playbackThread to exit
    pthread_mutex_lock(narratorMutex);
    mState = Narrator::EXIT;
    pthread_mutex_unlock(narratorMutex);

    LOG4CXX_INFO(narratorLog, "Waiting for playbackthread to join");
    pthread_join (playbackThread, NULL);
    free(narratorMutex);
}

/**
 * Prints messages (UNIMPLEMENTED)
 */
void Narrator::printMessages()
{
    /*
    map<const char*, Helptext>::iterator cur;
    bool printComma = false;

    for(cur = mHelptexts.begin(); cur != mHelptexts.end(); cur++) {
        switch(filter)
        {
            case FULL:
                cout << (*cur).second.filename << ":\t" << (*cur).first << endl;
                break;

            case ID:
                if(printComma) cout << ",";
                if(!printComma) printComma = true;
                cout << (*cur).second.id;
                break;
        }
    }
    cerr << endl;
    */
}

/**
 * Set which language the narrator should speak, meaning which language prompts the narrator fetches from the messages db.
 *
 * @param lang A two letter representation of the language. Values should be taken from the ISO-639 specification.
 * @return false if the language is not supported, otherwise true
 *
 */
bool Narrator::setLanguage(string lang)
{
    // Transform filename into lowercase (for comparison)
    std::transform(lang.begin(), lang.end(), lang.begin(), (int(*)(int))tolower);

    if (lang == "en")
    {
        LOG4CXX_DEBUG(narratorLog, "Setting language to: English");
    }
    else if(lang == "sv")
    {
        LOG4CXX_DEBUG(narratorLog, "Setting language to: Swedish");
    }
    else if (lang == "fi")
    {
        LOG4CXX_DEBUG(narratorLog, "Setting language to: Finnish");
    }
    else
    {
        LOG4CXX_DEBUG(narratorLog ,"Setting language to: Unknown");
        lang = "unknown";
    }

    pthread_mutex_lock(narratorMutex);
    mLanguage = lang;
    pthread_mutex_unlock(narratorMutex);

    if (lang == "unknown") return false;
    return true;
}

/**
 * Getter for the language the narrator is currently using.
 *
 * @return The language the narrator is currently speakning.
 */
string Narrator::getLanguage()
{
    string lang;
    pthread_mutex_lock(narratorMutex);
    lang = mLanguage;
    pthread_mutex_unlock(narratorMutex);
    return lang;
}

/**
 * Set path to where the database is located
 *
 * @param path Full path to the database
 */
void Narrator::setDatabasePath(string path)
{
    LOG4CXX_DEBUG(narratorLog, "Setting database path '" << path << "'");
    pthread_mutex_lock(narratorMutex);
    mDatabasePath = path;
    pthread_mutex_unlock(narratorMutex);

    // Verify that the database is initialized
    narrator::DB db(path);
    db.connect();
    if(!db.verifyDBStructure()){
        LOG4CXX_ERROR(narratorLog, "The database could could not be verified: " << path);
    }
}

/**
 * Get path to database
 *
 * @return Full path to the database
 */
string Narrator::getDatabasePath()
{
    string path;
    //pthread_mutex_lock(narratorMutex);
    path = mDatabasePath;
    //pthread_mutex_unlock(narratorMutex);
    return path;
}

/**
 * Check if OGG audio for an identifier exists in database
 *
 * @return True if audio exists, otherwise false
 */
bool Narrator::hasOggAudio(const char *identifier)
{
    LOG4CXX_DEBUG(narratorLog, "Find audio with identifier: '" << identifier << "'");
    Message message;
    message.setString(identifier);
    message.setClass("userdata");

    // use function in MessageHandler to find message
    MessageHandler mh;
    long id = mh.findMessage(message);

    if (id > 0) return true;
    LOG4CXX_DEBUG(narratorLog, "No entry found for identifier: '" << identifier << "'");
    return false;
}

/**
 * Check if MP3 audio for an identifier exists in database (UNIMPLEMENTED)
 *
 * @return True if audio exists, otherwise false
 */
bool Narrator::hasMp3Audio(const char *identifier)
{
    return false;
}

/**
 * Add OGG audio with identifier to database
 *
 * @return True if audio was inserted or update, otherwise false
 */
bool Narrator::addOggAudio(const char *identifier, const char *data, int size)
{
    LOG4CXX_DEBUG(narratorLog, "Add audio with identifier: '" << identifier << "'");
    // create MessageAudio object
    MessageAudio messageAudio;
    messageAudio.setTagid(0);
    messageAudio.setText(identifier);
    messageAudio.setAudioData(data, size);
    messageAudio.setSize(size);
    messageAudio.setLength(0);
    messageAudio.setMd5("");
    messageAudio.setUri("");

    // create MessageTranslation object
    MessageTranslation messageTranslation;
    messageTranslation.setLanguage(mLanguage); // use current language
    messageTranslation.setText(identifier);
    messageTranslation.setAudiotags("[0]");
    messageTranslation.addAudio(messageAudio);

    // create Message object
    Message message;
    message.setString(identifier);
    message.setClass("userdata");
    message.setTranslation(messageTranslation);

    // use function in MessageHandler to insert/update message
    MessageHandler mh;
    long id = mh.updateMessage(message);

    if (id > 0) return true;
    LOG4CXX_WARN(narratorLog, "Failed to add audio with identifier: '" << identifier << "'");
    return false;
}

/**
 * Add MP3 audio with identifier to database (UNIMPLEMENTED)
 *
 * @return True if audio was inserted or update, otherwise false
 */
bool Narrator::addMp3Audio(const char *identifier, const char *data, int size)
{
    return false;
}

/**
 * Toggle if Narrator shall send NarratorFinished when done
 *
 * @param value True if Narrator should send NarratorFinished, false otherwise.
 */
void Narrator::setPushCommandFinished(bool value)
{
    pthread_mutex_lock(narratorMutex);
    bPushCommandFinished = value;
    pthread_mutex_unlock(narratorMutex);
}

/**
 * Get current value for reporting NarratorFinnished
 *
 * @return True if narrator is currently sending NarratorFinished, false otherwise.
 */
bool Narrator::getPushCommandFinished()
{
    bool value;
    pthread_mutex_lock(narratorMutex);
    value = bPushCommandFinished;
    pthread_mutex_unlock(narratorMutex);
    return value;
}

/**
 * Adjust the playback volumegain
 *
 * @param adjustment +/- volumegain
 */
void Narrator::adjustVolumeGain(float adjustment)
{
    pthread_mutex_lock(narratorMutex);
    float value = mVolumeGain + adjustment;
    pthread_mutex_unlock(narratorMutex);

    return setVolumeGain(value);
}

/**
 * Get the current playback volumegain
 *
 * @return volumegain
 */
float Narrator::getVolumeGain()
{
    pthread_mutex_lock(narratorMutex);
    float volumegain = mVolumeGain;
    pthread_mutex_unlock(narratorMutex);

    return volumegain;
}

/**
 * Set the playback volumegain
 *
 * @param value volumegain (between MAX_VOLUMEGAIN and MIN_VOLUMEGAIN)
 */
void Narrator::setVolumeGain(float value)
{
    if(value <= NARRATOR_MIN_VOLUMEGAIN) value = NARRATOR_MIN_VOLUMEGAIN;
    if(value >= NARRATOR_MAX_VOLUMEGAIN) value = NARRATOR_MAX_VOLUMEGAIN;
    pthread_mutex_lock(narratorMutex);
    mVolumeGain = value;
    pthread_mutex_unlock(narratorMutex);

    LOG4CXX_DEBUG(narratorLog, "Setting volumegain to: " << value);
    return;
}

/**
 * Adjust the playback tempo
 *
 * @param adjustment +/- tempo
 */
void Narrator::adjustTempo(float adjustment)
{
    pthread_mutex_lock(narratorMutex);
    float value = mTempo + adjustment;
    pthread_mutex_unlock(narratorMutex);

    return setTempo(value);
}

/**
 * Get the current playback tempo
 *
 * @return tempo
 */
float Narrator::getTempo()
{
    pthread_mutex_lock(narratorMutex);
    float tempo = mTempo;
    pthread_mutex_unlock(narratorMutex);

    return tempo;
}

/**
 * Set the playback tempo
 *
 * @param value tempo (between NARRATOR_MAX_TEMPO and NARRATOR_MIN_TEMPO)
 */
void Narrator::setTempo(float value)
{
    if(value <= NARRATOR_MIN_TEMPO) value = NARRATOR_MIN_TEMPO;
    if(value >= NARRATOR_MAX_TEMPO) value = NARRATOR_MAX_TEMPO;
    pthread_mutex_lock(narratorMutex);
    mTempo = value;
    pthread_mutex_unlock(narratorMutex);

    LOG4CXX_DEBUG(narratorLog, "Setting tempo to: " << value);
    return;
}

/**
 * Adjust the playback pitch
 *
 * @param adjustment +/- pitch
 */
void Narrator::adjustPitch(float adjustment)
{
    pthread_mutex_lock(narratorMutex);
    float value = mPitch + adjustment;
    pthread_mutex_unlock(narratorMutex);

    return setPitch(value);
}

/**
 * Get the current playback pitch
 *
 * @return pitch
 */
float Narrator::getPitch()
{
    pthread_mutex_lock(narratorMutex);
    float pitch = mPitch;
    pthread_mutex_unlock(narratorMutex);

    return pitch;
}

/**
 * Set the playback pitch
 *
 * @param value pitch (between MAX_PITCH and MIN_PITCH)
 */
void Narrator::setPitch(float value)
{
    if(value <= NARRATOR_MIN_PITCH) value = NARRATOR_MIN_PITCH;
    if(value >= NARRATOR_MAX_PITCH) value = NARRATOR_MAX_PITCH;
    pthread_mutex_lock(narratorMutex);
    mPitch = value;
    pthread_mutex_unlock(narratorMutex);

    LOG4CXX_DEBUG(narratorLog, "Setting pitch to: " << value);
    return;
}

/**
 * Emitt the audio-finished-playing signal
 */
void Narrator::audioFinishedPlaying()
{
    if(bPushCommandFinished) {
        m_signal_audio_finished();
    }
}

/**
 * Setup the playback thread
 *
 * @return False if it failed, true otherwise.
 */
bool Narrator::setupThread() {
    LOG4CXX_INFO(narratorLog, "Setting up playback thread");
    if(pthread_create(&playbackThread, NULL, narrator_thread, this)) {
        usleep(500000);
        return false;
    }
    return true;
}

/**
 * Set integer parameter value for an audio prompt
 *
 * @param key Name of the parameter
 * @param value Value for the parameter
 */
void Narrator::setParameter(const string &key, int value)
{
    LOG4CXX_DEBUG(narratorLog, "Got parameter: " << key << ", with value: " << value);
    MessageParameter mp(key);
    mp.setIntValue(value);

    pthread_mutex_lock(narratorMutex);
    if(nextMessage == NULL)
        nextMessage = new Message();
    nextMessage->addParameter(mp);
    pthread_mutex_unlock(narratorMutex);
}

/**
 * Set string parameter value for an audio prompt
 *
 * @param key Name of the parameter
 * @param value Value for the parameter
 */
void Narrator::setParameter(const string &key, const string &value)
{
    LOG4CXX_DEBUG(narratorLog, "Got parameter: " << key << ", with value: " << value);

    pthread_mutex_lock(narratorMutex);
    if(nextMessage == NULL)
        nextMessage = new Message();
    nextMessage->setParameterValue(key, value);
    pthread_mutex_unlock(narratorMutex);
}

/**
 * Add a prompt onto queue of files to be played if action is CLEAR remove items from playlist and stop playback of the current item
 *
 * @param identifier Identifier of the audio prompt
 */
void Narrator::play(const char *identifier)
{
    PlaylistItem pi;

    pi.mIdentifier = identifier;
    pi.mClass = "prompt";

    pthread_mutex_lock(narratorMutex);
    if(nextMessage == NULL)
        nextMessage = new Message();
    pi.mMessage = nextMessage;
    nextMessage = NULL;
    mPlaylist.push(pi);
    pthread_mutex_unlock(narratorMutex);
}

/**
 * Narrate a file source from a given path
 *
 * @param filepath path to file as a string
 */
void Narrator::playFile(const string filepath)
{
    PlaylistItem pi;

    pi.mIdentifier = filepath;
    pi.mClass = "file";

    pthread_mutex_lock(narratorMutex);
    if(nextMessage == NULL)
        nextMessage = new Message();

    pi.mMessage = nextMessage;
    nextMessage = NULL;
    mPlaylist.push(pi);
    pthread_mutex_unlock(narratorMutex);
}

/**
 * Narrate a number based
 *
 * @param number the number to be narrated
 */
void Narrator::play(int number)
{
    PlaylistItem pi;

    setParameter("number", number);
    pi.mIdentifier = "{number}";
    pi.mClass = "number";

    pthread_mutex_lock(narratorMutex);
    if(nextMessage == NULL)
        nextMessage = new Message();

    pi.mMessage = nextMessage;
    nextMessage = NULL;
    mPlaylist.push(pi);
    pthread_mutex_unlock(narratorMutex);
}

/**
 * Narrate a audio prompt
 *
 * @param str Identifier of the prompt
 * @param cls Prompt type
 *
 */
void Narrator::playResource(string str, string cls)
{
    PlaylistItem pi;

    pi.mIdentifier = str;
    pi.mClass = cls;

    pthread_mutex_lock(narratorMutex);
    pi.mMessage = new Message();
    mPlaylist.push(pi);
    pthread_mutex_unlock(narratorMutex);
}

/**
 * Narrate a duration
 *
 * @param hours Number of hours
 * @param minutes Number of minutes
 * @param seconds Number of seconds
 */
void Narrator::playDuration(int hours, int minutes, int seconds)
{
    //printf("%02d:%02d:%02d\n", hours, minutes, seconds);
    long long totalSeconds = 0;
    if(hours > 0) totalSeconds += 3600 * hours;
    if(minutes > 0) totalSeconds += 60 * minutes;
    if(seconds > 0) totalSeconds += seconds;

    hours = (int)floor(totalSeconds / 60 / 60);
    minutes = (int)floor((totalSeconds - (hours * 60 * 60)) / 60);
    seconds = (int)floor((totalSeconds - (hours * 60 * 60) - (minutes * 60)));
    //printf("Duration: %02d:%02d:%02d\n", hours, minutes, seconds);

    if(hours > 0) {
        if(hours == 1) {
            play(_N("one hour"));
        } else {
            setParameter("2", hours);
            play(_N("{2} hours"));
        }
        if(minutes != 0 || seconds != 0) play(_N("and"));
    }

    if(minutes > 0) {
        if(minutes == 1) {
            play(_N("one minute"));
        } else {
            setParameter("2", minutes);
            play(_N("{2} minutes"));
        }
        if(seconds != 0) play(_N("and"));
    }

    if(seconds > 0) {
        if(seconds == 1) {
            play(_N("one second"));
        } else {
            setParameter("2", seconds);
            play(_N("{2} seconds"));
        }
    }

    // If the duration is zero say it
    if(hours == 0 && minutes == 0 && seconds == 0) {
        setParameter("2", 0);
        play(_N("{2} seconds"));
    }
}

/**
 * Narrate a duration
 *
 * @param seconds duration in seconds
 */
void Narrator::playDuration(long seconds)
{
    //printf("Duration in seconds %lld\n", seconds);
    int h = (int)floor(seconds / 60 / 60);
    int m = (int)floor((seconds - (h * 60 * 60)) / 60);
    int s = (int)floor((seconds - (h * 60 * 60) - (m * 60)));
    playDuration(h, m, s);
}

/**
 * Keep a long pause
 */
void Narrator::playLongpause()
{
    play(_N("longpause"));
}

/**
 * Keep a short pause
 */
void Narrator::playShortpause()
{
    play(_N("shortpause"));
}

/**
 * Play wait jingle
 */
void Narrator::playWait()
{
    play(_N("wait"));
}

/**
 * Spell word character by character
 *
 * The following characters are supported
 * letters: a-z and A-Z
 * numbers: 0-9
 * symbols: ! # % * + , - . / : ; ? @ _ ~
 */
void Narrator::spell(string word)
{
    // transform each character to upper case
    std::transform(word.begin(), word.end(), word.begin(), ::toupper);
    LOG4CXX_DEBUG(narratorLog, "spelling word '" << word << "'");

    string::iterator it;
    for (it=word.begin(); it<word.end(); it++)
    {
        char c = char(*it);
        stringstream ss;
        string s;
        ss << c;
        ss >> s;

        // char is a number in range 0-9
        if(c >= 48 && c <= 57)
        {
            int number = atoi(&c);
            play(number);
            LOG4CXX_DEBUG(narratorLog, "number '" << number << "'");
        }
        // char is a letter in range A-Z
        else if(c >= 65 && c <= 90)
        {
            LOG4CXX_DEBUG(narratorLog, "letter '" << c << "'");
            playResource(s, "letter");
        }
        else
        {
            switch(c)
            {
                case 32: // space
                    playShortpause();
                    break;
                case 33: // !
                case 35: // #
                case 37: // %
                case 42: // *
                case 43: // +
                case 44: // ,
                case 45: // -
                case 46: // .
                case 47: // /
                case 58: // :
                case 59: // ;
                case 63: // ?
                case 64: // @
                case 95: // _
                case 126: // ~
                    LOG4CXX_DEBUG(narratorLog, "symbol '" << c << "'");
                    playResource(s, "symbol");
                    break;
                default:
                    LOG4CXX_WARN(narratorLog, "character not supported");
                    break;
            }
        }
    }
}

/**
 * Narrate date
 *
 * @param day Day of the month
 * @param month Month of the year
 * @param year Year
 */
void Narrator::playDate(int day, int month, int year)
{
    //printf("%02d-%02d-%04d\n", day, month, year);

    if(day < 1) day = 1;
    if(day > 31) day = 31;
    if(month < 1) month = 1;
    if(month > 12) month = 12;
    if(year < 0) year = 0;
    if(year > 3000) year = 3000;

    setParameter("date", day);
    setParameter("month", month);
    setParameter("year", year);
    setParameter("yearnum", year);

    // Calculate the day this date occurred (http://users.aol.com/s6sj7gt/mikecal.htm)
    int daynum = (day+=month<3?year--:year-2,23*month/9+day+4+year/4-year/100+year/400)%7;
    setParameter("dayname", daynum);

    play(_N("{dayname} {date} of {month} {year} {yearnum}"));
}

/**
 * Narrate time
 *
 * @param hour Hour of the day
 * @param minute Minute of the hour
 * @param second Second of the minute
 */
void Narrator::playTime(int hour, int minute, int second)
{
    //printf("%02d:%02d:%02d\n", hour, minute, second);
    int hour12;
    bool isAm = false;

    if(hour < 0) hour = 0;
    if(hour > 24) hour = 0;
    if(hour > 12) {
        hour12 = hour - 12;
        isAm = true;
    } else hour12 = hour;

    if(minute < 0) minute = 0;
    if(minute > 59) minute = 59;
    if(second < 0) second = 0;
    if(second > 59) second = 59;

    setParameter("minute", minute);
    setParameter("second", second);
    setParameter("hour", hour);
    setParameter("hour12", hour12);
    if(isAm)
        setParameter("ampm", "am");
    else
        setParameter("ampm", "pm");

    play(_N("{hour} {hour12} {minute} {second} {ampm}"));
}

/**
 * Force narrator to stop speaking
 */
void Narrator::stop()
{
    pthread_mutex_lock (narratorMutex);
    // Clear the list
    while(!mPlaylist.empty()) {
        //LOG4CXX_DEBUG(narratorLog, "Removing :'" << mPlaylist.front());
        delete(mPlaylist.front().mMessage);
        mPlaylist.pop();
    }
    pthread_mutex_unlock (narratorMutex);

    // Tell the playbackthread to stop playing
    pthread_mutex_lock(narratorMutex);
    bResetFlag = true;
    pthread_mutex_unlock(narratorMutex);
}

/**
 * Setter for narrator state
 *
 * @param state A narrator thread state
 */
void Narrator::setState(Narrator::threadState state)
{

    pthread_mutex_lock(narratorMutex);
    if(mState == Narrator::EXIT) {
        LOG4CXX_INFO(narratorLog, "Narrator in state:" << getState_str(mState) << ", not changing to state: " << getState_str(state));
    }
    else
        mState = state;
    pthread_mutex_unlock(narratorMutex);
}

/**
 * Getter for the narrator state
 *
 * @return Current narator thread state
 */
Narrator::threadState Narrator::getState()
{
    Narrator::threadState state;
    pthread_mutex_lock(narratorMutex);
    state = mState;
    pthread_mutex_unlock(narratorMutex);

    return state;
}

/**
 * Check if narrator is currently speaking
 *
 * @return True if the narrator is currently speaking
 */
bool Narrator::isSpeaking()
{
    Narrator::threadState state = getState();

    if(state == Narrator::PLAY || mPlaylist.size() > 0) return true;
    else return false;
}

/**
 * Getter for narrator state as a human readable
 *
 * @return Narrator state as a string
 */
string Narrator::getState_str()
{
    Narrator::threadState state = getState();
    return getState_str(state);
}

/**
 * Getter for narrator state as a human readable
 *
 * @param state to convert
 * @return Narrator state as a string
 */
string Narrator::getState_str(Narrator::threadState state)
{
    switch(state)
    {
        case Narrator::DEAD: return "Narrator::DEAD";
        case Narrator::WAIT: return "Narrator::WAIT";
        case Narrator::PLAY: return "Narrator::PLAY";
        case Narrator::EXIT: return "Narrator::EXIT";
    }

    return "UNKNOWN";
}

/**
 * Getter for the number of items the narrator has left to narrate
 *
 * @returns How many items the narrator has left to narrate
 */
int Narrator::numPlaylistItems()
{
    int queueitems = 0;
    pthread_mutex_lock(narratorMutex);
    queueitems = mPlaylist.size();
    pthread_mutex_unlock(narratorMutex);
    return queueitems;
}

/*! \cond PRIVATE */

/**
 * Called from the narrator_thread to adjust playback parameters.
 *
 * @param n reference to the narrator to adjust
 * @param filter audio filter
 * @param gain new gain
 * @param tempo new tempo
 * @param pitch new pitch
 */
void adjustGainTempoPitch(Narrator* n, Filter& filter, float& gain, float& tempo, float& pitch)
{
    pthread_mutex_lock(n->narratorMutex);
    if(gain != n->mVolumeGain) {
        gain = n->mVolumeGain;
        LOG4CXX_DEBUG(narratorLog, "Setting gain(" << gain << ")");
        filter.setGain(gain);
    }

    if(tempo != n->mTempo) {
        tempo = n->mTempo;
        LOG4CXX_DEBUG(narratorLog, "Setting tempo(" << tempo << ")");
        filter.setTempo(tempo);
    }

    if(pitch != n->mPitch) {
        pitch = n->mPitch;
        LOG4CXX_DEBUG(narratorLog, "Setting pitch(" << pitch << ")");
        filter.setPitch(pitch);
    }
    pthread_mutex_unlock(n->narratorMutex);
}

/**
 * Set the signal slot for when playback has finished
 *
 * @param slot A slot which should be called when audio has finished
 */
boost::signals2::connection Narrator::connectAudioFinished(const AudioFinishedSlotType &slot)
{
    return m_signal_audio_finished.connect(slot);
}

/**
 * Called from the narrator_thread to copy audio data from the filter to portaudio.
 */
void writeSamplesToPortaudio( Narrator* n, PortAudio& portaudio, Filter& filter, float* buffer )
{
    int outSamples = 0;
    Narrator::threadState state = n->getState();

    // See if we have any finished samples
    // One filter sample contains data from all channels
    while((outSamples = filter.numSamples()) != 0 && state == Narrator::PLAY) {
        int available = portaudio.getWriteAvailable();

        //LOG4CXX_DEBUG(narratorLog, "Got writeavailable %d, outSamples: %d", available, outSamples);

        if(available > outSamples) available = outSamples;

        if(available > BUFFERSIZE) available = BUFFERSIZE;
        outSamples = filter.read(buffer, available);

        state = n->getState();
        if(state != Narrator::PLAY)
            LOG4CXX_INFO(narratorLog, "Aborting stream");

        portaudio.write(buffer, outSamples);
    }
}

/**
 * The playback thread code
 * \internal
 */
void *narrator_thread(void *narrator)
{
    //Narrator *n = static_cast<Narrator *>(narrator);
    Narrator *n = (Narrator*)narrator;

    int queueitems;

    // Set initial values to 0 so that they get updated when thread gets playsignal
    float gain = 0;
    float tempo = 0;
    float pitch = 0;

    PortAudio portaudio;
    Filter filter;
    OggStream oggstream;

    Narrator::threadState state = n->getState();
    LOG4CXX_INFO(narratorLog, "Starting playbackthread");

    do {
        queueitems = n->numPlaylistItems();

        if(queueitems == 0) {
            // Wait a little before calling callback
            long waitms = portaudio.getRemainingms();
            if(waitms != 0) {
                LOG4CXX_DEBUG(narratorLog, "Waiting " << waitms << " ms for playback to finish");
                while(waitms > 0 && queueitems == 0) {
                    usleep(100000);
                    queueitems = n->numPlaylistItems();
                    waitms -= 100;
                }
            }

            // Break if we during the pause got some more queued items to play
            if(queueitems == 0) {
                if(state != Narrator::DEAD)
                    n->audioFinishedPlaying();
                n->setState(Narrator::WAIT);
                LOG4CXX_INFO(narratorLog, "Narrator in WAIT state");
                portaudio.stop();

                while(queueitems == 0) {
                    state = n->getState();
                    if(state == Narrator::EXIT) break;

                    usleep(10000);
                    queueitems = n->numPlaylistItems();
                }
            }
            LOG4CXX_INFO(narratorLog, "Narrator starting playback");
        }

        if(state == Narrator::EXIT) break;

        n->setState(Narrator::PLAY);
        n->bResetFlag = false;

        Narrator::PlaylistItem pi;

        pthread_mutex_lock(n->narratorMutex);
        if(n->mPlaylist.size() > 0) {
            pi = n->mPlaylist.front();
            n->mPlaylist.pop();
        } else {
            LOG4CXX_ERROR(narratorLog, "Narrator started playback thread without playlistitems");
            pthread_mutex_unlock(n->narratorMutex);
            continue;
        }
        string lang = n->mLanguage;
        pthread_mutex_unlock(n->narratorMutex);

        // If trying to play a file, open it
        if(pi.mClass == "file") {
            LOG4CXX_DEBUG(narratorLog, "Playing file: " << pi.mIdentifier);

            // Open the oggstream
            if(!oggstream.open(pi.mIdentifier)) {
                LOG4CXX_ERROR(narratorLog, "Narrator translation not found: Error opening oggstream: " << pi.mIdentifier);
                continue;
            }

            if(!portaudio.open(oggstream.getRate(), oggstream.getChannels())) {
                LOG4CXX_ERROR(narratorLog, "error initializing portaudio, (rate: " << oggstream.getRate() << " channels: " << oggstream.getChannels() << ")");
                continue;
            }

            if(!filter.open(oggstream.getRate(), oggstream.getChannels())) {
                LOG4CXX_ERROR(narratorLog, "error initializing filter");
                continue;
            }


            int inSamples = 0;
            soundtouch::SAMPLETYPE* buffer = new soundtouch::SAMPLETYPE[oggstream.getChannels()*BUFFERSIZE];
            //buffer = (short*)malloc(sizeof(short) * 2 * BUFFERSIZE);
            // long totalSamplesRead = 0;
            do {
                // change gain, tempo and pitch
                adjustGainTempoPitch(n, filter, gain, tempo, pitch);

                // read some stuff from the oggstream
                inSamples = oggstream.read(buffer, BUFFERSIZE*oggstream.getChannels());

                //printf("Read %d samples from oggstream\n", inSamples);

                if(inSamples != 0) {
                    filter.write(buffer, inSamples); // One sample contains data for all channels here
                    writeSamplesToPortaudio( n, portaudio, filter, buffer );
                } else {
                    LOG4CXX_INFO(narratorLog, "Flushing soundtouch buffer");
                    filter.flush();
                }

                state = n->getState();

            } while (inSamples != 0 && state == Narrator::PLAY && !n->bResetFlag);

            if(buffer != NULL) delete [] (buffer);
            oggstream.close();
        }

        // Else try opening from database
        else {
            vector <MessageAudio> vAudioQueue;

            // Get a list of MessageAudio objects to play

            Message *m = pi.mMessage;
            if(m==NULL){
                LOG4CXX_ERROR(narratorLog, "Message was null");
            }

            m->setLanguage(lang);
            m->load(pi.mIdentifier, pi.mClass);

            if(!m->compile() || !m->hasAudio()) {
                LOG4CXX_ERROR(narratorLog, "Narrator translation not found: could not find audio for '" << pi.mIdentifier << "'");
            } else {
                vAudioQueue = m->getAudioQueue();
            }

            // Play what we got
            if(vAudioQueue.size() > 0) {
                vector <MessageAudio>::iterator audio;
                audio = vAudioQueue.begin();
                do {
                    LOG4CXX_INFO(narratorLog, "Saying: " << audio->getText());

                    // Open the oggstream
                    if(!oggstream.open(*audio)) {
                        LOG4CXX_ERROR(narratorLog, "error opening oggstream");
                        break;
                    }

                    if(!portaudio.open(oggstream.getRate(), oggstream.getChannels())) {
                        LOG4CXX_ERROR(narratorLog, "error initializing portaudio");
                        break;
                    }

                    if(!filter.open(oggstream.getRate(), oggstream.getChannels())) {
                        LOG4CXX_ERROR(narratorLog, "error initializing filter");
                        break;
                    }


                    int inSamples = 0;
                    soundtouch::SAMPLETYPE* buffer = new soundtouch::SAMPLETYPE[oggstream.getChannels()*BUFFERSIZE];

                    do {
                        // change gain, tempo and pitch
                        adjustGainTempoPitch(n, filter, gain, tempo, pitch);

                        // read some stuff from the oggstream
                        inSamples = oggstream.read(buffer, BUFFERSIZE*oggstream.getChannels());

                        if(inSamples != 0) {
                            filter.write(buffer, inSamples);
                            writeSamplesToPortaudio( n, portaudio, filter, buffer );
                        } else {
                            LOG4CXX_INFO(narratorLog, "Flushing soundtouch buffer");
                            filter.flush();
                        }

                        state = n->getState();

                    } while (inSamples != 0 && state == Narrator::PLAY && !n->bResetFlag);

                    if(buffer != NULL) delete [] (buffer);
                    oggstream.close();
                    audio++;

                } while(audio != vAudioQueue.end() && state == Narrator::PLAY && !n->bResetFlag);
            }
            //Cleanup message object
            delete(pi.mMessage);
        }

        // Abort stream?
        if(n->bResetFlag) {
            n->bResetFlag = false;
            portaudio.stop();
            filter.clear();
        }

    } while(state != Narrator::EXIT);

    LOG4CXX_INFO(narratorLog, "Shutting down playbackthread");

    pthread_exit(NULL);
    return NULL;
}
/*! \endcond */
