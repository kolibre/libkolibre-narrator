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

#ifndef _MESSAGE_H
#define _MESSAGE_H

#define _N(string) string

#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <ogg/os_types.h>

#include "Db.h"

using namespace std;

enum MessageParameterType {
    param_unknown = 0,
    param_number,
    param_number_en,
    param_digits,
    param_message,
    param_date_date,
    param_date_dayname,
    param_date_month,
    param_date_year,
    param_date_hour,
    param_date_hour12,
    param_date_minute_zeropad,
    param_date_second_zeropad };

class MessageAudio {
    public:
        MessageAudio();
        ~MessageAudio();

        void setAudioid(int audioid) { mAudioid = audioid; };
        int getAudioid() const { return mAudioid; };

        void setTagid(int tagid) { mTagid = tagid; };
        int getTagid() const { return mTagid; };

        void setText(string text) { mText = text; };
        const string &getText() const { return mText; };

        void setUri(string uri) { mUri = uri; };
        const string &getUri() const { return mUri; };

        void setMd5(string md5) { mMd5 = md5; };
        const char *getMd5() const { return mMd5.c_str(); };

        void setSize(size_t size) { mSize = size; };
        size_t getSize() const { return mSize; };

        void setLength(int length) { mLength = length; };
        int getLength() const { return mLength; };

        void setAudioData(const char *source, size_t num);
        const char *getAudioData() const { return pAudioData; };
        bool isAudioDataNil() const { return pAudioData == NULL ? true : false; };

        //bool setDatabase(const string &db) { mDatabase = db; };

        // vorbisfile interface functions.
        size_t read(void *ptr, size_t size, size_t nmemb);
        int close();
        int seek(long offset, int whence);
        long tell() { return mCurrentPos; };

        // debugfunctions
        void print() const;

    private:
        int mAudioid;
        int mTagid;
        string mText;
        int mLength;
        string mUri;
        size_t mSize;
        string mMd5;
        char *pAudioData;

        // Read variables
        int mCurrentPos;
        sqlite3_blob *pBlob;
        sqlite3 *pDBHandle;
        DB *db;
        //string mDatabase;
};

// ov_file interface for ov_open_callbacks
extern "C" {
    size_t MessageAudio_read(void *ptr, size_t size, size_t nmemb, void *datasource);
    int MessageAudio_seek(void *datasource, ogg_int64_t offset, int whence);
    int MessageAudio_close(void *datasource);
    long MessageAudio_tell(void *datasource);
}

class MessageTranslation {
    public:
        MessageTranslation();
        MessageTranslation(string text, string audiotags);
        ~MessageTranslation();

        void setLanguage(string lang) { mLanguage = lang; };
        const string getLanguage() const { return mLanguage; };

        void setText(string text) { mText = text; };
        const string &getText() const { return mText; };

        void setAudiotags(string tags) { mAudiotags = tags; };
        const string &getAudiotags() const { return mAudiotags; };

        int numAudio() const { return vAudio.size(); };
        void addAudio(const MessageAudio &audio);
        int findAudioIdx(int tagid);
        const MessageAudio &getAudio(int idx) const { return vAudio[idx]; };
        const vector<MessageAudio> &getAudio() { return vAudio; };

        // debugfunctions
        void print() const;

    private:
        string mText;
        string mLanguage;
        string mAudiotags;
        vector <MessageAudio> vAudio;
};

class MessageParameter {
    public:
        MessageParameter(string key);
        MessageParameter(string key, int value);
        MessageParameter(string key, string type);

        // Getters and setters
        void setKey(string key) { mKey = key; };
        const string &getKey() const { return mKey; };

        bool compareType(string type) const {
            return (type.compare(typeToString(mType)) == 0);
        };

        void setType(string type) { mType = stringToType(type); };
        MessageParameterType getType() const { return mType; };
        string getTypeStr() const { return string(typeToString(mType)); };

        void setIntValue(int value) { mIntValue = value; };
        int getIntValue() const { return mIntValue; };

        void setStringValue(string value) { mStringValue = value; };
        const string &getStringValue() const { return mStringValue; };
        // debugfunctions
        void print() const;
    private:
        string mKey;
        MessageParameterType mType;

        int mIntValue;
        string mStringValue;

        MessageParameterType stringToType(const string type) const;
        const string typeToString(MessageParameterType type) const;

};

class Message {
    public:
        Message();
        Message(DB *db, Message *parent);
        ~Message();

        bool load(string identifier, string cls, int id);

        // compiles the audio, inserting parameters and submessages in the audioqueue
        bool compile();
        bool appendMessage(string identifier, string cls);
        bool appendParameter(MessageParameter &param);
        bool appendNumber(int number, MessageParameterType type=param_number);
        bool appendDigits(string str);

        bool hasAudio();

        bool setParameterValue(const string &key, const string &value);
        bool setParameterValue(const string &key, int value);
        bool loadParameterValues(const vector<MessageParameter>&params);

        // Queue functions for recursive compiling
        bool appendAudioQueue(const MessageAudio &);
        bool appendAudioQueue(const vector<MessageAudio>&);
        const vector<MessageAudio> &getAudioQueue();

        // Getters and setters
        void setLanguage(string lang) { mLanguage = lang; };
        const string &getLanguage() { return mLanguage; };
        void setId(long id) { mId = id; };
        long getId() const { return mId; };
        void setString(string str) { mString = str; };
        const string &getString() const { return mString; };
        void setClass(string cl) { mClass = cl; };
        const string &getClass() const { return mClass; };
        // Parameters
        void setParameters(const vector<MessageParameter>&params) { vParameters = params; };
        const vector<MessageParameter>& getParameters() const { return vParameters; };

        void addParameter(string key, string type);
        int findParameterIdx(const string &key);
        int numParameters() const { return vParameters.size(); };
        const MessageParameter &getParameter(int idx) const { return vParameters[idx]; };

        // Translation
        void setTranslation(const MessageTranslation &translation);
        bool hasTranslation() const { return bHasTranslation; };
        const MessageTranslation &getTranslation() const { return mTranslation; };

        // debugfunctions
        void print() const;

    private:
        long mId;
        string mLanguage;
        string mString;
        string mClass;
        vector <MessageParameter> vParameters;
        MessageTranslation mTranslation;
        bool bHasTranslation;

        Message *pParent;

        vector<MessageAudio> mAudioQueue;

        int split(const string& input, const string &delimiter, vector<string>&results, bool includeEmpties);

        bool openDB();
        DB *db;
        bool bClosedb;
};

#endif
