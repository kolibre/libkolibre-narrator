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

#include "Message.h"
#include "Narrator.h"

#include <iostream>
#include <sstream>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <log4cxx/logger.h>

// create logger which will become a child to logger kolibre.narrator
log4cxx::LoggerPtr narratorMsgLog(log4cxx::Logger::getLogger("kolibre.narrator.message"));

using namespace std;

////////////////////////////////
/////////// Message
////////////////////////////////
Message::Message()
{
    db = NULL;
    pParent = NULL;

    bClosedb = false;
    vParameters.empty();
    bHasTranslation = false;
    mLanguage = "unknown";

}

Message::Message(narrator::DB *dbptr, Message *parent)
{
    db = dbptr;
    bClosedb = false;
    pParent = parent;

    bHasTranslation = false;

    if(pParent) {
        setLanguage(parent->getLanguage());
        setParameters(parent->getParameters());
    } else {
        vParameters.empty();
        mLanguage = "unknown";
    }
}

Message::~Message()
{
    if(db && bClosedb) {
        delete db;
    }
}

// Only the root message should setup the db
bool Message::openDB()
{
    string messagedb = Narrator::Instance()->getDatabasePath();
    db = new narrator::DB(messagedb);

    if(!db->connect()) {
        LOG4CXX_ERROR(narratorMsgLog, "Could not open database " << messagedb << " '" << db->getLasterror() << "'");
        return false;
    }

    bClosedb = true;
    return true;
}

bool Message::load(string identifier, string cls)
{
    if(!db) openDB();
    //LOG4CXX_DEBUG(narratorMsgLog, "Loading '" << identifier << "' (class=" << cls << ") in lang " << mLanguage);

    // Query the database for message
    if(!db->prepare("select rowid, string, class from message where (string=? AND class=?) OR string=? group by rowid")) {
        LOG4CXX_ERROR(narratorMsgLog, "Query failed '" << db->getLasterror() << "'");
        return false;
    }

    if(!db->bind(1, identifier.c_str()) ||
            !db->bind(2, cls.c_str()) ||
            !db->bind(3, identifier.c_str())) {
        return false;
    }

    DBResult result;
    if(!db->perform(&result))
        return false;

    int messageid = -1;
    int count = 0;
    while(result.loadRow()) {
        // Use only first result returned
        if(messageid == -1) {
            messageid = result.getInt(0);
            setString(result.getText(1));
            setClass(result.getText(2));
        }
        //result.printRow();
        count++;
    }

    if(count == 0) {
        LOG4CXX_ERROR(narratorMsgLog, "Message not found for '" << identifier << "'");
        return false;
    }

    if(count > 1) {
        LOG4CXX_WARN(narratorMsgLog, "Multiple messages (" << count << ") found for '" << identifier << "'");
    }

    // Query the database for messageparameters
    if(!db->prepare("select key, type from messageparameter where message_id=?")) {
        LOG4CXX_ERROR(narratorMsgLog, "Query failed '" << db->getLasterror() << "'");
        return false;
    }


    if(!db->bind(1, messageid)) {
        LOG4CXX_ERROR(narratorMsgLog, "Bind failed '" << db->getLasterror() << "'");
        return false;
    }

    DBResult result2;

    if(!db->perform(&result2)) {
        LOG4CXX_ERROR(narratorMsgLog, "Query failed '" << db->getLasterror() << "'");
        return false;
    }

    count = 0;
    while(result2.loadRow()) {
        //result2.printRow();
        addParameter(result2.getText(0), result2.getText(1));
        count++;

    }

    //printf("Got %d paramters\n", count);

    // Query the database for messagetranslation
    if(!db->prepare("select rowid, translation, audiotags, language from messagetranslation where message_id=? order by language=?")) {
        LOG4CXX_ERROR(narratorMsgLog, "Query failed '" << db->getLasterror() << "'");
        return false;
    }

    if(!db->bind(1, messageid) || !db->bind(2, mLanguage.c_str())) {
        LOG4CXX_ERROR(narratorMsgLog, "Bind failed '" << db->getLasterror() << "'");
        return false;
    }

    DBResult result3;
    if(!db->perform(&result3)) {
        LOG4CXX_ERROR(narratorMsgLog, "Query failed '" << db->getLasterror() << "'");
        return false;
    }

    MessageTranslation mt;
    int translationid = -1;
    count = 0;
    while(result3.loadRow()) {
        //Use last result returned
        //result3.printRow();
        translationid = result3.getInt(0);
        mt.setText(result3.getText(1));
        mt.setAudiotags(result3.getText(2));
        mt.setLanguage(result3.getText(3));
        count++;
    }

    if(count == 0) {
        LOG4CXX_ERROR(narratorMsgLog, "Translation not found for message with id: " << messageid);
        return false;
    }

    if(count > 1) {
        LOG4CXX_WARN(narratorMsgLog, "Multiple translations (" << count << ") found for message with id: " << messageid);
    }


    if(!db->prepare("select rowid, tagid, text, size, length, md5 from messageaudio where translation_id=? order by tagid")) {
        LOG4CXX_ERROR(narratorMsgLog, "Query failed '" << db->getLasterror() << "'");
        return false;
    }

    if(!db->bind(1, translationid)) {
        LOG4CXX_ERROR(narratorMsgLog, "Bind failed '" << db->getLasterror() << "'");
        return false;
    }

    DBResult result4;
    if(!db->perform(&result4)) {
        LOG4CXX_ERROR(narratorMsgLog, "Query failed '" << db->getLasterror() << "'");
        return false;
    }

    count = 0;
    while(result4.loadRow()) {
        //Add all audioids
        MessageAudio *ma = new MessageAudio();
        ma->setAudioid(result4.getInt(0));
        ma->setTagid(result4.getInt(1));
        ma->setText(result4.getText(2));
        ma->setSize(result4.getInt(3));
        ma->setLength(result4.getInt(4));
        ma->setMd5(result4.getText(5));

        // Set the db where messageaudio can find data
        //ma->setDatabase(db->getDatabase());
        mt.addAudio(*ma);
        delete ma;
        count++;
    }

    setTranslation(mt);

    if(count == 0) {
        LOG4CXX_ERROR(narratorMsgLog, "No audio found for translation with id: " << translationid);
        return true;
    }

    if(count > 1) {
        LOG4CXX_WARN(narratorMsgLog, "Multiple audio (" << count << ") found for translation with id: " << translationid);
    }

    return true;
}

bool Message::appendAudioQueue(const MessageAudio &ma)
{
    //cout << "---->pushing back " << ma.getText() << endl;
    mAudioQueue.push_back(ma);
    return true;
}

bool Message::appendAudioQueue(const vector<MessageAudio>&q)
{
    vector<MessageAudio>::const_iterator i;
    i = q.begin();
    while(i != q.end()) {
        appendAudioQueue(*i);
        i++;
    }
    return true;
}

const vector<MessageAudio>& Message::getAudioQueue()
{
    return mAudioQueue;
}

// Compiles a message, creating submessages on the way
bool Message::compile()
{
    if(bHasTranslation) {
        //cout << "Message '" << mTranslation.getText() << "' has audiotags: " << mTranslation.getAudiotags() << endl;
        string audiotags = mTranslation.getAudiotags();

        // Split the string into parameters and tags
        vector<string>tags;
        if(split(audiotags, " ", tags, false) == 0) {
            if(audiotags.length()) tags.push_back(audiotags);
        }

        if(tags.size()) {

            vector<string>::iterator i = tags.begin();
            while(i != tags.end()) {
                string tag = *i;
                size_t pos1;
                size_t pos2;

                // Check if this tag is an audioid
                if((pos1 = tag.find('[')) != string::npos &&
                        (pos2 = tag.find(']')) != string::npos)
                {
                    stringstream ss(tag.substr(pos1+1, pos2-pos1-1));
                    int tagid = -1;
                    ss >> tagid;
                    //cout << "Got audio tag " << tagid << endl;

                    int idx = mTranslation.findAudioIdx(tagid);
                    if(idx >= 0) appendAudioQueue(mTranslation.getAudio(idx));
                    else {
                        LOG4CXX_WARN(narratorMsgLog, "No audio found for tagid: '" << tagid << "' in '" << audiotags << "'");
                    }
                }

                // Check if this is a parameter
                else if((pos1 = tag.find('{')) != string::npos &&
                        (pos2 = tag.find('}')) != string::npos)
                {
                    string paramkey = tag.substr(pos1+1, pos2-pos1-1);
                    //cout << "Got parameter tag " << paramkey << endl;

                    vector<MessageParameter>::iterator i = vParameters.begin();
                    while(i != vParameters.end()) {
                        //(*i).print();
                        if((*i).getKey() == paramkey) {
                            //paramtype = (*i).getType();
                            appendParameter((*i));
                        } //else cout << (*i).getKey() << " does not match " << paramkey << endl;
                        i++;
                    }


                }

                // This is an unknown tag type
                else {
                    LOG4CXX_WARN(narratorMsgLog, "Strange tag: '" << tag << "' in '" << audiotags << "'");
                }
                i++;
            }
            //iCurrentAudioQueue = mAudioQueue.begin();

        } else {
            LOG4CXX_WARN(narratorMsgLog, "No audiotags for: '" << mTranslation.getText() << "'");
        }

        //cout << "Compilation of '" << mString << "' finished" << endl;
        //getTranslation().print();
        //cout << endl;

        if(!mAudioQueue.empty()) return true;
    }
    LOG4CXX_ERROR(narratorMsgLog, "No audio or translation for '" << mString << "'");
    return false;
}

bool Message::appendParameter(MessageParameter &param)
{
    switch(param.getType())
    {
        case param_number:
            //LOG4CXX_WARN(narratorMsgLog, "Compiling in number: '" << param.getIntValue() << "'");
            appendNumber(param.getIntValue(), param_number);
            break;

        case param_number_en:
            //LOG4CXX_WARN(narratorMsgLog, "Compiling in number: '" << param.getIntValue() << "'");
            appendNumber(param.getIntValue(), param_number_en);
            break;

        case param_digits:
            //LOG4CXX_WARN(narratorMsgLog, "Compiling in number: '" << param.getIntValue() << "'");
            appendDigits(param.getStringValue());
            break;

        case param_message:
            //LOG4CXX_WARN(narratorMsgLog, "Compiling in number: '" << param.getIntValue() << "'");
            appendMessage(param.getStringValue(), "prompt");
            break;

        case param_date_date:
            switch(param.getIntValue())
            {
                case 1: appendMessage(_N("1st"), "date"); break;
                case 2: appendMessage(_N("2nd"), "date"); break;
                case 3: appendMessage(_N("3rd"), "date"); break;
                case 4: appendMessage(_N("4th"), "date"); break;
                case 5: appendMessage(_N("5th"), "date"); break;
                case 6: appendMessage(_N("6th"), "date"); break;
                case 7: appendMessage(_N("7th"), "date"); break;
                case 8: appendMessage(_N("8th"), "date"); break;
                case 9: appendMessage(_N("9th"), "date"); break;
                case 10: appendMessage(_N("10th"), "date"); break;
                case 11: appendMessage(_N("11th"), "date"); break;
                case 12: appendMessage(_N("12th"), "date"); break;
                case 13: appendMessage(_N("13th"), "date"); break;
                case 14: appendMessage(_N("14th"), "date"); break;
                case 15: appendMessage(_N("15th"), "date"); break;
                case 16: appendMessage(_N("16th"), "date"); break;
                case 17: appendMessage(_N("17th"), "date"); break;
                case 18: appendMessage(_N("18th"), "date"); break;
                case 19: appendMessage(_N("19th"), "date"); break;
                case 20: appendMessage(_N("20th"), "date"); break;
                case 21: appendMessage(_N("21st"), "date"); break;
                case 22: appendMessage(_N("22nd"), "date"); break;
                case 23: appendMessage(_N("23rd"), "date"); break;
                case 24: appendMessage(_N("24th"), "date"); break;
                case 25: appendMessage(_N("25th"), "date"); break;
                case 26: appendMessage(_N("26th"), "date"); break;
                case 27: appendMessage(_N("27th"), "date"); break;
                case 28: appendMessage(_N("28th"), "date"); break;
                case 29: appendMessage(_N("29th"), "date"); break;
                case 30: appendMessage(_N("30th"), "date"); break;
                case 31: appendMessage(_N("31st"), "date"); break;
                default:
                         LOG4CXX_WARN(narratorMsgLog, "No date string specified for int '" << param.getIntValue() << "'");
            }
            break;

        case param_date_dayname:
            switch(param.getIntValue())
            {
                case 1: appendMessage(_N("Monday"), "date"); break;
                case 2: appendMessage(_N("Tuesday"), "date"); break;
                case 3: appendMessage(_N("Wednesday"), "date"); break;
                case 4: appendMessage(_N("Thursday"), "date"); break;
                case 5: appendMessage(_N("Friday"), "date"); break;
                case 6: appendMessage(_N("Saturday"), "date"); break;
                case 0: appendMessage(_N("Sunday"), "date"); break;
                default:
                        LOG4CXX_WARN(narratorMsgLog, "No day name specified for int '" << param.getIntValue() << "'");
            }
            break;

        case param_date_month:
            switch(param.getIntValue())
            {
                case 1: appendMessage(_N("jan"), "date"); break;
                case 2: appendMessage(_N("feb"), "date"); break;
                case 3: appendMessage(_N("mar"), "date"); break;
                case 4: appendMessage(_N("apr"), "date"); break;
                case 5: appendMessage(_N("may"), "date"); break;
                case 6: appendMessage(_N("jun"), "date"); break;
                case 7: appendMessage(_N("jul"), "date"); break;
                case 8: appendMessage(_N("aug"), "date"); break;
                case 9: appendMessage(_N("sep"), "date"); break;
                case 10: appendMessage(_N("oct"), "date"); break;
                case 11: appendMessage(_N("nov"), "date"); break;
                case 12: appendMessage(_N("dec"), "date"); break;
                default:
                         LOG4CXX_WARN(narratorMsgLog, "No month name specified for int '" << param.getIntValue() << "'");
            }
            break;

        case param_date_year:
            if(param.getIntValue() >= 2000) {
                appendNumber(param.getIntValue());
            } else {
                appendNumber(19);
                appendMessage(_N("hundred"), "number");
                appendNumber(param.getIntValue() - 1900);
            }
            break;

        case param_date_hour:
        case param_date_hour12:
            appendNumber(param.getIntValue());
            break;

        case param_date_minute_zeropad:
        case param_date_second_zeropad:
            if(param.getIntValue() < 10) appendMessage(_N("zero"), "number");
            if(param.getIntValue() == 0) appendMessage(_N("zero"), "number");
            else appendNumber(param.getIntValue());
            break;

        default:
            LOG4CXX_WARN(narratorMsgLog, "Unknown parameter type: " << param.getTypeStr());
            break;
    }

    return true;
}

bool Message::appendMessage(string identifier, string cls)
{
    LOG4CXX_INFO(narratorMsgLog, "Appending message '" << identifier << "'");

    bool compileStatus = false;
    Message *m;
    m = new Message(db, this);
    m->load(identifier, cls);
    compileStatus = m->compile();
    if(compileStatus) {
        appendAudioQueue(m->getAudioQueue());
    } else {
        LOG4CXX_WARN(narratorMsgLog, "Failed to append message '" << identifier << "'");
    }

    delete m;
    return compileStatus;
}

bool Message::appendDigits(string str)
{
    LOG4CXX_INFO(narratorMsgLog, "Appending digits '" << str << "'");

    std::transform(str.begin(), str.end(), str.begin(), (int(*)(int))toupper);

    unsigned char digit;
    string tmp;
    for(unsigned int i = 0; i < str.length(); i++) {
        digit = str.c_str()[i];
        tmp = digit;
        switch(digit)
        {
            case '+':
                appendMessage(_N("plus"), "number");
                break;
            case 'A':
            case 'B':
            case 'C':
            case 'D':
            case 'E':
            case 'F':
                appendMessage(tmp, "number");
                break;
            case '0': appendNumber(0); break;
            case '1': appendNumber(1); break;
            case '2': appendNumber(2); break;
            case '3': appendNumber(3); break;
            case '4': appendNumber(4); break;
            case '5': appendNumber(5); break;
            case '6': appendNumber(6); break;
            case '7': appendNumber(7); break;
            case '8': appendNumber(8); break;
            case '9': appendNumber(9); break;
            default:
                      LOG4CXX_WARN(narratorMsgLog, "Unknown digit '" << tmp << "'");
        }
    }
    return true;
}

bool Message::appendNumber(int number, MessageParameterType type)
{
    if(number < 0) {
        appendMessage(_N("minus"), "prompt");
        number = -number;
    }

    if(number == 1000) {
        appendMessage(_N("one thousand"), "number");
        return true;
    } if(number > 1000) {
        appendNumber(number/1000, type);
        appendMessage(_N("thousand"), "number");
        number -= (number/1000) * 1000;
        if(number == 0) return true;
    }


    if(number == 100) {
        appendMessage(_N("one hundred"), "number");
        return true;
    } else if(number > 100) {
        appendNumber(number/100, type);
        appendMessage(_N("hundred"), "number");
        number -= (number/100) * 100;
        if(number == 0) return true;
    }

    switch(number / 10)
    {
        case 0:
            switch(number)
            {
                case 0: appendMessage(_N("zero"), "number"); break;
                case 1:
                    // case for en/ett in swedish
                    if(type == param_number_en) {
                        appendMessage(_N("one_special"), "number"); break;
                    } else  {
                        appendMessage(_N("one"), "number"); break;
                    }
                case 2: appendMessage(_N("two"), "number"); break;
                case 3: appendMessage(_N("three"), "number"); break;
                case 4: appendMessage(_N("four"), "number"); break;
                case 5: appendMessage(_N("five"), "number"); break;
                case 6: appendMessage(_N("six"), "number"); break;
                case 7: appendMessage(_N("seven"), "number"); break;
                case 8: appendMessage(_N("eight"), "number"); break;
                case 9: appendMessage(_N("nine"), "number"); break;
            }

            break;

        case 1:
            switch(number)
            {
                case 10: appendMessage(_N("ten"), "number"); break;
                case 11: appendMessage(_N("eleven"), "number"); break;
                case 12: appendMessage(_N("twelve"), "number"); break;
                case 13: appendMessage(_N("thirteen"), "number"); break;
                case 14: appendMessage(_N("fourteen"), "number"); break;
                case 15: appendMessage(_N("fifteen"), "number"); break;
                case 16: appendMessage(_N("sixteen"), "number"); break;
                case 17: appendMessage(_N("seventeen"), "number"); break;
                case 18: appendMessage(_N("eighteen"), "number"); break;
                case 19: appendMessage(_N("nineteen"), "number"); break;
            }
            break;
        case 2:
            appendMessage(_N("twenty"), "number");
            if(number != 20) appendNumber(number-20, type);
            break;
        case 3:
            appendMessage(_N("thirty"), "number");
            if(number != 30) appendNumber(number-30, type);
            break;
        case 4:
            appendMessage(_N("fourty"), "number");
            if(number != 40) appendNumber(number-40, type);
            break;
        case 5:
            appendMessage(_N("fifty"), "number");
            if(number != 50) appendNumber(number-50, type);
            break;
        case 6:
            appendMessage(_N("sixty"), "number");
            if(number != 60) appendNumber(number-60, type);
            break;
        case 7:
            appendMessage(_N("seventy"), "number");
            if(number != 70) appendNumber(number-70, type);
            break;
        case 8:
            appendMessage(_N("eighty"), "number");
            if(number != 80) appendNumber(number-80, type);
            break;
        case 9:
            appendMessage(_N("ninety"), "number");
            if(number != 90) appendNumber(number-90, type);
            break;
    }

    return true;
}

int Message::split(const string& input, const string &delimiter, vector<string>&results, bool includeEmpties)
{

    int iPos = 0;
    int newPos = -1;
    int sizeS2 = (int)delimiter.size();
    int isize = (int)input.size();

    if(isize == 0 || sizeS2 == 0)
    {
        return 0;
    }

    vector<int> positions;

    newPos = input.find(delimiter, 0);

    if(newPos < 0)
    {
        return 0;
    }

    int numFound = 0;

    while( newPos >= iPos )
    {
        numFound++;
        positions.push_back(newPos);
        iPos = newPos;
        newPos = input.find (delimiter, iPos+sizeS2);
    }

    if(numFound == 0) {
        return 0;
    }

    for(unsigned int i=0; i <= positions.size(); ++i)
    {
        string s("");
        if(i == 0)
        {
            s = input.substr(i, positions[i]);
            results.push_back(s);
            continue;
        }

        int offset = positions[i-1] + sizeS2;
        if(offset < isize)
        {
            if(i == positions.size())
            {
                s = input.substr(offset);
            }
            else if(i > 0)
            {
                s = input.substr(positions[i-1] + sizeS2,
                        positions[i] - positions[i-1] - sizeS2);
            }
        }
        if(includeEmpties || (s.size() > 0))
        {
            results.push_back(s);
        }
    }
    return numFound;
}

bool Message::hasAudio()
{
    if(!mAudioQueue.empty()) return true;
    return false;
}


/*
bool Message::nextAudio()
  {
  if(mAudioQueue.empty()) return false;
  iCurrentAudioQueue++;
  if(iCurrentAudioQueue != mAudioQueue.end()) return true;

  return false;
  }

  const MessageAudio &Message::getAudio()
  {
  return (*iCurrentAudioQueue);
  }
*/

void Message::print() const
{
    cout << "String: " << mString << endl;
    vector <MessageParameter>::const_iterator i;
    for(i = vParameters.begin(); i != vParameters.end(); i++)
        i->print();

    if(bHasTranslation)
        mTranslation.print();
    else cout << "Message has no translation" << endl;

    if(!mAudioQueue.empty())
        cout << "Message has audio" << endl;

}

void Message::addParameter(string key, string type) {
    MessageParameter mp(key, type);
    vParameters.push_back(mp);
}

int Message::findParameterIdx(const string &key)
{
    vector <MessageParameter>::const_iterator i;
    int count = 0;
    i = vParameters.begin();
    while(i != vParameters.end()) {
        if((*i).getKey() == key) return count;
        count++;
        i++;
    }
    return -1;
}


bool Message::loadParameterValues(const vector<MessageParameter> &param)
{
    string key = "";
    vector <MessageParameter>::const_iterator i;
    i = param.begin();
    while(i != param.end()) {
        key = (*i).getKey();

        int idx = findParameterIdx(key);
        if(idx == -1) {
            LOG4CXX_ERROR(narratorMsgLog, "Failed to set parameter value for key '" << key << "'");
            return false;
        }

        vParameters[idx].setStringValue((*i).getStringValue());
        vParameters[idx].setIntValue((*i).getIntValue());
        i++;
    }

    return true;
}

bool Message::setParameterValue(const string &key, const string &value)
{
    int idx = findParameterIdx(key);
    if(idx == -1) {
        LOG4CXX_ERROR(narratorMsgLog, "Failed to set parameter value for key '" << key << "'");
        return false;
    }

    vParameters[idx].setStringValue(value);
    return true;
}

bool Message::setParameterValue(const string &key, int value)
{
    int idx = findParameterIdx(key);
    if(idx == -1) {
        LOG4CXX_ERROR(narratorMsgLog, "Failed to set parameter value for key '" << key << "'");
        return false;
    }

    vParameters[idx].setIntValue(value);
    return true;
}

void Message::setTranslation(const MessageTranslation &translation)
{
    mTranslation = translation;
    bHasTranslation = true;
}

////////////////////////////////
/////////// TRANSLATION
////////////////////////////////
MessageTranslation::MessageTranslation()
{
    mAudiotags = "";
    mLanguage = "unknown";
}

MessageTranslation::~MessageTranslation()
{
}

MessageTranslation::MessageTranslation(string text, string audiotags)
{
    //cout << "constructing messagetranslation" << endl;
    mText = text;
    mAudiotags = audiotags;
}

void MessageTranslation::addAudio(const MessageAudio &audio)
{
    if(mAudiotags == "") mAudiotags = "[0]";
    //if(audio.getTagid() == -1) audio.setTagid(vAudio.size() + 1);
    vAudio.push_back(audio);
}

int MessageTranslation::findAudioIdx(int tagid)
{
    vector<MessageAudio>::iterator i = vAudio.begin();
    int count = 0;
    while(i != vAudio.end()) {
        //cout << "comparing '" << tagid << "' to '" << (*i).getTagid() << "'" << endl;
        if((*i).getTagid() == tagid) return count;
        count++;
        i++;
    }

    return -1;
}

void MessageTranslation::print() const
{
    cout << "translation: " << mText << endl;
    cout << "language: " << mLanguage << endl;
    cout << "audiotags: " << mAudiotags << endl;

    vector <MessageAudio>::const_iterator i;
    for(i = vAudio.begin(); i != vAudio.end(); i++)
        i->print();
}

////////////////////////////////
/////////// PARAMETER
////////////////////////////////
MessageParameter::MessageParameter(string key, string type)
{
    //cout << "constructing messageparameter: " << key << " type: " << type << " typeid " << stringToType(type) << endl;
    mKey = key;
    mType = stringToType(type);
    mIntValue = -1;
    mStringValue = "";

    //cout << "String type: '" << type << "' became " << typeToString(mType) << endl;
}


MessageParameter::MessageParameter(string key, int value)
{
    //cout << "constructing messageparameter: " << key << " type: " << type << " typeid " << stringToType(type) << endl;
    mKey = key;
    mType = param_unknown;
    mIntValue = value;
    mStringValue = "";
}

MessageParameter::MessageParameter(string key)
{
    //cout << "constructing messageparameter: " << key << " type: " << type << " typeid " << stringToType(type) << endl;
    mKey = key;
    mType = param_unknown;
    mIntValue = -1;
    mStringValue = "";
}

void MessageParameter::print() const
{
    cout << "Parameter: " << mKey << "(" << typeToString(mType) << ") = ";
    switch(mType)
    {
        case param_date_date:
        case param_date_dayname:
        case param_date_month:
        case param_date_year:
        case param_date_hour:
        case param_date_hour12:
        case param_date_minute_zeropad:
        case param_date_second_zeropad:
        case param_number:
        case param_number_en:
            cout << mIntValue;
            break;

        case param_digits:
        case param_message:
            cout << mStringValue;
            break;

        default:
            cout << mIntValue << " / " << mStringValue;;
    }
    cout << endl;
}

MessageParameterType MessageParameter::stringToType(const string type) const
{
    if(type.compare("number") == 0) return param_number;
    else if(type.compare("number-en") == 0) return param_number_en;
    else if(type.compare("digits") == 0) return param_digits;
    else if(type.compare("message") == 0) return param_message;
    else if(type.compare("date(date)") == 0) return param_date_date;
    else if(type.compare("date(dayname)") == 0) return param_date_dayname;
    else if(type.compare("date(month)") == 0) return param_date_month;
    else if(type.compare("date(year)") == 0) return param_date_year;
    else if(type.compare("date(hour)") == 0) return param_date_hour;
    else if(type.compare("date(hour12)") == 0) return param_date_hour12;
    else if(type.compare("date(minute)") == 0) return param_date_minute_zeropad;
    else if(type.compare("date(second)") == 0) return param_date_second_zeropad;

    return param_unknown;
}

const string MessageParameter::typeToString(MessageParameterType type) const
{
    switch(type) {
        case param_number: return "number";
        case param_number_en: return "number-en";
        case param_digits: return "digits";
        case param_message: return "message";
        case param_date_date: return "date(date)";
        case param_date_dayname: return "date(dayname)";
        case param_date_month: return "date(month)";
        case param_date_year: return "date(year)";
        case param_date_hour: return "date(hour)";
        case param_date_hour12: return "date(hour12)";
        case param_date_minute_zeropad: return "date(minute)";
        case param_date_second_zeropad: return "date(second)";
        case param_unknown: return "unknown";
    }
    return "unknown";
}

////////////////////////////////
/////////// AUDIO
////////////////////////////////
MessageAudio::MessageAudio()
{
    //cout << "constructing messageaudio" << endl;
    mText = "";
    mSize = 0;
    mLength = 0;
    mTagid = 0;
    mAudioid = 0;
    mCurrentPos = 0;
    pAudioData = NULL;
    pDBHandle = NULL;
    pBlob = NULL;
}

MessageAudio::~MessageAudio()
{
    //if (!isAudioDataNil()) free(pAudioData); // causes double free
}

void MessageAudio::setAudioData(const char *source, size_t num)
{
    pAudioData = (char *) malloc (num * sizeof(char));
    memcpy(pAudioData, source, num);
    mSize = num;
}

void MessageAudio::print() const
{
    cout << "this: " << this << endl;
    cout << "audioid: " << mAudioid << endl;
    cout << "tagid: " << mTagid << endl;
    cout << "audiotext: " << mText << endl;
    cout << "uri: " << mUri << "[" << mMd5 << "]" << endl;
}

size_t MessageAudio::read(void *ptr, size_t size, size_t nmemb)
{
    // Open db blob for read if it's not already open
    int rc = 0;
    if(pBlob == NULL) {
        if(pDBHandle == NULL) {
            string messagedb = Narrator::Instance()->getDatabasePath();
            db = new narrator::DB(messagedb);
            if(!db->connect()) {
                LOG4CXX_ERROR(narratorMsgLog, "Could not open database " << messagedb << " '" << db->getLasterror() << "'");
                return 0;
            }
            pDBHandle = db->getHandle();
        }

        rc = sqlite3_blob_open(pDBHandle, "main", "messageaudio", "data", mAudioid, 0, &pBlob);
        if(rc) {
            pBlob = NULL;
            LOG4CXX_ERROR(narratorMsgLog, "An error occurred opening audioid: " << mAudioid << " , " << sqlite3_errmsg(pDBHandle));
            return 0;
        }

        if(sqlite3_blob_bytes(pBlob) != (int)mSize) {
            LOG4CXX_ERROR(narratorMsgLog, "Blob size " << sqlite3_blob_bytes(pBlob) << " does not match file size " << mSize);
            mSize = sqlite3_blob_bytes(pBlob);
        }
        mCurrentPos = 0;
    }

    int bytes_to_read = 0;
    unsigned int bytes_left = mSize - mCurrentPos;

    if(bytes_left > size * nmemb) {
        bytes_to_read = size * nmemb;
    } else {
        bytes_to_read = bytes_left;
    }

    rc = sqlite3_blob_read(pBlob, ptr, bytes_to_read, mCurrentPos);
    if(rc) {
        LOG4CXX_ERROR(narratorMsgLog, "An error occurred reading audioid: " << mAudioid << ", " << sqlite3_errmsg(pDBHandle));
        return 0;
    }

    mCurrentPos += bytes_to_read;

    //printf("read %d bytes out of requested %d\n", bytes_to_read, size*nmemb);

    return bytes_to_read;
}

int MessageAudio::close()
{
    if(pBlob != NULL) {
        int rc = sqlite3_blob_close(pBlob);
        if(rc) {
            LOG4CXX_ERROR(narratorMsgLog, "An error occurred while closing audioid: " << mAudioid << ", " << sqlite3_errmsg(pDBHandle));
        }
    }

    if(pDBHandle != NULL) {
        delete db;
    }
    return 0;
}

int MessageAudio::seek(long offset, int whence)
{
    int seekpos = -1;
    if(pBlob != NULL) {
        switch(whence) {
            case SEEK_SET:
                mCurrentPos = offset;
                break;

            case SEEK_END:
                mCurrentPos = mSize + 1 - offset;
                if((size_t)mCurrentPos > mSize) mCurrentPos = mSize + 1;
                break;

            case SEEK_CUR:
                mCurrentPos = mCurrentPos + offset;
                break;
        }
        seekpos = mCurrentPos;
    }

    return seekpos;
}

size_t MessageAudio_read(void *ptr, size_t size, size_t nmemb, void *datasource)
{
    //printf("in read func\n");
    MessageAudio *ma = static_cast<MessageAudio *>(datasource);
    return ma->read(ptr, size, nmemb);
}

int MessageAudio_seek(void *datasource, ogg_int64_t offset, int whence)
{
    MessageAudio *ma = static_cast<MessageAudio *>(datasource);
    return ma->seek(offset, whence);
}

int MessageAudio_close(void *datasource) {
    MessageAudio *ma = static_cast<MessageAudio *>(datasource);
    return ma->close();
}

long MessageAudio_tell(void *datasource) {
    MessageAudio *ma = static_cast<MessageAudio *>(datasource);
    return ma->tell();
}
