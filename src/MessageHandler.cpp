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

#include <sstream>
#include <iostream>

#include "MessageHandler.h"
#include "Narrator.h"

#include <cstring>
#include <log4cxx/logger.h>

// create logger which will become a child to logger kolibre.narrator
log4cxx::LoggerPtr narratorMsgHlrLog(log4cxx::Logger::getLogger("kolibre.narrator.messagehandler"));

MessageHandler::MessageHandler()
{
    currentMessage = NULL;
    currentMessageTranslation = NULL;
    currentMessageAudio = NULL;

    string messagedb = Narrator::Instance()->getDatabasePath();

    // Setup database connection
    db = new DB(messagedb);

    if(!db->connect()) {
        LOG4CXX_ERROR(narratorMsgHlrLog, "Could not open database " << messagedb << " '" << db->getLasterror() << "'");
        return;
    }
}

/// Message handling routines
long MessageHandler::updateMessage(const Message &msg)
{
    long messageid = -1;
    long translationid = -1;
    long params = -1;
    //msg.print();

    if(msg.getString() == "" ||
            msg.getClass() == "" ||
            msg.getId() == -1) {
        LOG4CXX_ERROR(narratorMsgHlrLog, "Not enough info to add message to database '" << msg.getString() << "'");
        //msg.print();
        return -1;
    }


    // Start a new transaction
    if(!db->prepare("BEGIN")) {
        LOG4CXX_ERROR(narratorMsgHlrLog, "Query failed '" << db->getLasterror() << "'");
    }

    DBResult result;
    if(!db->perform(&result)) {
        LOG4CXX_ERROR(narratorMsgHlrLog, "%s: Query failed '" << db->getLasterror() << "'");
        return false;
    }


    // Check if we already have this message in the database
    messageid = checkMessage(msg);
    if(messageid < 0)
        messageid = insertMessage(msg);

    // Check message translationx
    if(messageid > 0) {
        if(msg.hasTranslation())
            translationid = checkMessageTranslation(messageid, msg.getTranslation());
        else translationid = 0;
    }

    // Check message parameters
    if(messageid > 0) {
        params = checkMessageParameters(messageid, msg);
    }

    // If all went well commit changes
    if(messageid > 0 && translationid >= 0 && params >= 0) {
        if(!db->prepare("COMMIT")) {
            LOG4CXX_ERROR(narratorMsgHlrLog, "Query failed '" << db->getLasterror() << "'");
        }

        DBResult result2;
        if(!db->perform(&result2)) {
            LOG4CXX_ERROR(narratorMsgHlrLog, "Query failed '" << db->getLasterror() << "'");
            return -1;
        }
    } else {
        // If we had an error discard changes
        cout << "An error ocurred, rolling back changes" << endl;
        if(!db->prepare("COMMIT")) {
            LOG4CXX_ERROR(narratorMsgHlrLog, "Query failed '" << db->getLasterror() << "'");
        }

        DBResult result3;
        if(!db->perform(&result3)) {
            LOG4CXX_ERROR(narratorMsgHlrLog, "Query failed '" << db->getLasterror() << "'");
            return -1;
        }
    }
    return messageid;
}

// Finds a message in the database
long MessageHandler::findMessage(const Message &msg)
{
    long messageid = -1;

    if(!db->prepare("SELECT rowid, string, class, id FROM message WHERE string=? AND class=? AND id=?")) {
        LOG4CXX_ERROR(narratorMsgHlrLog, "Query failed '" << db->getLasterror() << "'");
        return -1;
    }

    if(!db->bind(1, msg.getString().c_str()) ||
            !db->bind(2, msg.getClass().c_str()) ||
            !db->bind(3, msg.getId())) {
        LOG4CXX_ERROR(narratorMsgHlrLog, "Bind failed '" << db->getLasterror() << "'");
        return -1;
    }

    DBResult result;
    if(!db->perform(&result)) {
        LOG4CXX_ERROR(narratorMsgHlrLog, "Query failed '" << db->getLasterror() << "'");
        return -1;
    }

    while(result.loadRow()) {
        messageid = result.getInt(0);
    }
    return messageid;
}

// Checks that data in message is the same as in the database, if the values differ update them
long MessageHandler::checkMessage(const Message &msg)
{
    long messageid = -1;
    string str = "";
    string cls = "";
    long id = 0;

    if(!db->prepare("SELECT rowid, string, class, id FROM message WHERE string=? AND class=? AND id=?")) {
        LOG4CXX_ERROR(narratorMsgHlrLog, "Query failed '" << db->getLasterror() << "'");
        return -1;
    }

    if(!db->bind(1, msg.getString().c_str()) ||
            !db->bind(2, msg.getClass().c_str()) ||
            !db->bind(3, msg.getId())) {
        LOG4CXX_ERROR(narratorMsgHlrLog, "Bind failed '" << db->getLasterror() << "'");
        return -1;
    }

    DBResult result;
    if(!db->perform(&result)) {
        LOG4CXX_ERROR(narratorMsgHlrLog, "Query failed '" << db->getLasterror() << "'");
        return -1;
    }

    while(result.loadRow()) {
        messageid = result.getInt(0);
        str = result.getText(1);
        cls = result.getText(2);
        id = result.getInt(3);
    }

    // If we already have a message with the same string, update the values if they differ
    bool differs = false;
    if(messageid > 0) {
        if(msg.getString() != str) {
            differs = true;
            cout << "message changed: " << msg.getString() << endl;
        }

        if(msg.getClass() != cls) {
            differs = true;
            cout << "message got new class: " << msg.getClass() << endl;
        };

        if(msg.getId() != id) {
            differs = true;
            cout << "message changed id: " << id << "-> " << msg.getId() << endl;
        }

        if(differs) updateMessage_with_id(messageid, msg);
    }
    return messageid;
}

long MessageHandler::checkMessageTranslation(long messageid, const MessageTranslation &mt)
{
    long translationid = -1;
    long audioid = -1;

    translationid = checkTranslation(messageid, mt);
    if(translationid < 0)
        translationid = insertTranslation(messageid, mt);

    if(translationid > 0)
        audioid = checkMessageAudio(translationid, mt);

    if(audioid >= 0)
        return translationid;

    else return -1;
}

long MessageHandler::checkMessageParameters(long messageid, const Message &msg)
{
    int i;
    int parameterid = -1;

    for(i = 0; i < msg.numParameters(); i++)
    {
        const MessageParameter &mp = msg.getParameter(i);
        parameterid = checkParameter(messageid, mp);
        if(parameterid < 0) parameterid = insertParameter(messageid, mp);
    }

    if(msg.numParameters() == 0) return 0;

    return parameterid;
}

// Checks that data in messageparameter is the same as in the database, if the values differ update them
long MessageHandler::checkParameter(long messageid, const MessageParameter &mp)
{
    long parameterid = -1;
    string key = "";
    string type = "";

    if(!db->prepare("SELECT rowid, key, type FROM messageparameter WHERE key=? AND message_id=?")) {
        LOG4CXX_ERROR(narratorMsgHlrLog, "Query failed '" << db->getLasterror() << "'");
        return -1;
    }

    if(!db->bind(1, mp.getKey().c_str()) ||
            !db->bind(2, messageid)) {
        LOG4CXX_ERROR(narratorMsgHlrLog, "Bind failed '" << db->getLasterror() << "'");
        return -1;
    }


    DBResult result;
    if(!db->perform(&result)) {
        LOG4CXX_ERROR(narratorMsgHlrLog, "Query failed '" << db->getLasterror() << "'");
        return -1;
    }

    int count = 0;
    while(result.loadRow()) {
        parameterid = result.getInt(0);
        key = result.getText(1);
        type = result.getText(2);
        count ++;
    }

    if(count > 1) {
        LOG4CXX_ERROR(narratorMsgHlrLog, "Multiple parameters for messageid '" << messageid << "'");
    }

    // If we already have a messageparameter with the same string, update the values if they differ
    bool differs = false;
    if(parameterid > 0) {
        if(mp.getKey() != key) {
            differs = true;
            cout << "messageparameter " << mp.getKey() << " key changed: " << mp.getKey() << endl;
        }

        if(mp.compareType(type) == false) {
            differs = true;
            cout << "messageparameter " << mp.getKey() << " got new type: " << mp.getTypeStr() << endl;
        };

        if(differs) updateParameter_with_id(parameterid, mp);
    }

    return parameterid;
}

long MessageHandler::checkMessageAudio(long translationid, const MessageTranslation &mt)
{
    int i;
    long audioid = -1;

    //cout << "Checking audio for messagetranslation " << mt.getText() << endl;
    //cout << "Audiocount " << mt.numAudio() << endl;

    for(i = 0; i < mt.numAudio(); i++)
    {
        const MessageAudio &ma = mt.getAudio(i);
        audioid = checkAudio(translationid, ma);
        if(audioid < 0) audioid = insertAudio(translationid, ma);
    }

    if(mt.numAudio() == 0) return 0;

    return audioid;
}

long MessageHandler::insertParameter(long messageid, const MessageParameter &mp)
{
    int messageparameterid = -1;

    LOG4CXX_INFO(narratorMsgHlrLog, "Inserting new messageparameter for message with id: " << messageid);
    LOG4CXX_DEBUG(narratorMsgHlrLog, "key: " << mp.getKey() << ", type: " << mp.getTypeStr());

    if(!db->prepare("INSERT INTO messageparameter (key, type, message_id) VALUES (?, ?, ?)")) {
        LOG4CXX_ERROR(narratorMsgHlrLog, "Query failed '" << db->getLasterror() << "'");
        return -1;
    }

    if(!db->bind(1, mp.getKey().c_str()) ||
            !db->bind(2, mp.getTypeStr().c_str()) ||
            !db->bind(3, messageid)) {
        LOG4CXX_ERROR(narratorMsgHlrLog, "Bind failed '" << db->getLasterror() << "'");
        return -1;
    }

    DBResult result;
    if(!db->perform(&result)) {;
        LOG4CXX_ERROR(narratorMsgHlrLog, "Query failed '" << db->getLasterror() << "'");
        return -1;
    }

    // Get the id of the inserted parameter
    messageparameterid = result.getInsertId();

    return messageparameterid;
}

// Update values for a messageparameter
long MessageHandler::updateParameter_with_id(long parameterid, const MessageParameter &mp)
{
    LOG4CXX_INFO(narratorMsgHlrLog, "Updating messageparameter with id: " << parameterid);
    LOG4CXX_DEBUG(narratorMsgHlrLog, "key: " << mp.getKey() << ", type: " << mp.getTypeStr());

    if(!db->prepare("UPDATE messageparameter SET key=?, type=? WHERE rowid=?")) {
        LOG4CXX_ERROR(narratorMsgHlrLog, "Query failed '" << db->getLasterror() << "'");
        return -1;
    }

    if(!db->bind(1, mp.getKey().c_str()) ||
            !db->bind(2, mp.getTypeStr().c_str()) ||
            !db->bind(3, parameterid)) {
        LOG4CXX_ERROR(narratorMsgHlrLog, "Bind failed '" << db->getLasterror() << "'");
        return -1;
    }

    if(!db->perform()) {
        LOG4CXX_ERROR(narratorMsgHlrLog, "Query failed '" << db->getLasterror() << "'");
        return -1;
    }

    return parameterid;
}

// Update values for a message
long MessageHandler::updateMessage_with_id(long messageid, const Message &msg)
{
    LOG4CXX_INFO(narratorMsgHlrLog, "Updating message with id: " << messageid);
    LOG4CXX_DEBUG(narratorMsgHlrLog, "string: " << msg.getString() << ", class: " << msg.getClass());

    if(!db->prepare( "UPDATE message SET string=?, class=?, id=? WHERE rowid=?")) {
        LOG4CXX_ERROR(narratorMsgHlrLog, "Query failed '" << db->getLasterror() << "'");
        return -1;
    }

    if(!db->bind(1, msg.getString().c_str()) ||
            !db->bind(2, msg.getClass().c_str()) ||
            !db->bind(3, msg.getId()) ||
            !db->bind(4, messageid)) {
        LOG4CXX_ERROR(narratorMsgHlrLog, "Bind failed '" << db->getLasterror() << "'");
        return -1;
    }

    if(!db->perform()) {
        LOG4CXX_ERROR(narratorMsgHlrLog, "Query failed '" << db->getLasterror() << "'");
        return -1;
    }

    return messageid;
}

long MessageHandler::insertMessage(const Message &msg)
{
    int messageid = -1;
    LOG4CXX_INFO(narratorMsgHlrLog, "Inserting new message");
    LOG4CXX_DEBUG(narratorMsgHlrLog, "string: " << msg.getString() << ", class: " << msg.getClass());

    if(!db->prepare("INSERT INTO message (string, class, id) VALUES (?, ?, ?)")) {
        LOG4CXX_ERROR(narratorMsgHlrLog, "Query failed '" << db->getLasterror() << "'");
        return -1;
    }

    if(!db->bind(1, msg.getString().c_str()) ||
            !db->bind(2, msg.getClass().c_str()) ||
            !db->bind(3, msg.getId())) {
        LOG4CXX_ERROR(narratorMsgHlrLog, "Bind failed '" << db->getLasterror() << "'");
        return -1;
    }

    DBResult result;
    if(!db->perform(&result)) {
        LOG4CXX_ERROR(narratorMsgHlrLog, "Query failed '" << db->getLasterror() << "'");
        return -1;
    }

    messageid = result.getInsertId();

    return messageid;
}

// Update values for a translation
long MessageHandler::updateTranslation_with_id(long translationid, const MessageTranslation &mt)
{
    LOG4CXX_INFO(narratorMsgHlrLog, "Updating messagetranslation with id: " << translationid);
    LOG4CXX_DEBUG(narratorMsgHlrLog, "text: " << mt.getText() << ", audiotags: " << mt.getAudiotags());

    if(!db->prepare("UPDATE messagetranslation SET translation=?, audiotags=? WHERE rowid=?")) {
        LOG4CXX_ERROR(narratorMsgHlrLog, "Query failed '" << db->getLasterror() << "'");
        return -1;
    }


    if(!db->bind(1,  mt.getText().c_str()) ||
            !db->bind(2, mt.getAudiotags().c_str()) ||
            !db->bind(3, translationid)) {
        LOG4CXX_ERROR(narratorMsgHlrLog, "Bind failed '" << db->getLasterror() << "'");
        return -1;
    }

    if(!db->perform()) {
        LOG4CXX_ERROR(narratorMsgHlrLog, "Query failed '" << db->getLasterror() << "'");
        return -1;
    }

    return translationid;
}

// Checks that data in translation is the same as in the database, if the values differ update them
long MessageHandler::checkTranslation(long messageid, const MessageTranslation &mt)
{
    long translationid = -1;
    string translation = "";
    string audiotags = "";

    if(!db->prepare("SELECT rowid, translation, audiotags FROM messagetranslation WHERE message_id=? AND language=?")) {
        LOG4CXX_ERROR(narratorMsgHlrLog, "Query failed '" << db->getLasterror() << "'");
        return -1;
    }

    if(!db->bind(1, messageid) ||
            !db->bind(2, mt.getLanguage().c_str())) {
        LOG4CXX_ERROR(narratorMsgHlrLog, "Bind failed '" << db->getLasterror() << "'");
        return -1;
    }

    DBResult result;
    if(!db->perform(&result)) {
        LOG4CXX_ERROR(narratorMsgHlrLog, "Query failed '" << db->getLasterror() << "'");
        return -1;
    }

    int count = 0;
    while(result.loadRow()) {
        translationid = result.getInt(0);
        translation = result.getText(1);
        audiotags = result.getText(2);
        count ++;
    }

    // If we already have a translation in the correct language, update the values if they differ
    bool differs = false;
    if(translationid > 0) {

        if(mt.getText() != translation) {
            differs = true;
            cout << "translation changed: " << mt.getText() << endl;
        }

        if(mt.getAudiotags() != audiotags) {
            differs = true;
            cout << "translation got new audiotags: " << mt.getAudiotags() << endl;
        };

        if(differs) updateTranslation_with_id(translationid, mt);
    }

    return translationid;
}

long MessageHandler::insertTranslation(long messageid, const MessageTranslation &mt)
{
    int translationid = -1;
    LOG4CXX_INFO(narratorMsgHlrLog, "Inserting new translation '" << mt.getText() << "' for message with id " << messageid);

    if(!db->prepare("INSERT INTO messagetranslation (message_id, language, translation, audiotags) VALUES (?, ?, ?, ?)")) {
        LOG4CXX_ERROR(narratorMsgHlrLog, "Query failed '" << db->getLasterror() << "'");
        return -1;
    }

    if(!db->bind(1, messageid) ||
            !db->bind(2,  mt.getLanguage().c_str()) ||
            !db->bind(3,  mt.getText().c_str()) ||
            !db->bind(4, mt.getAudiotags().c_str())) {
        LOG4CXX_ERROR(narratorMsgHlrLog, "Bind failed '" << db->getLasterror() << "'");
        return -1;
    }

    DBResult result;
    if(!db->perform(&result)) {
        LOG4CXX_ERROR(narratorMsgHlrLog, "Query failed '" << db->getLasterror() << "'");
        return -1;
    }

    translationid = result.getInsertId();

    return translationid;
}

// Update values for a audio
long MessageHandler::updateAudio_with_id(long audioid, const MessageAudio &ma)
{
    LOG4CXX_INFO(narratorMsgHlrLog, "Updating audio with id " << audioid);
    LOG4CXX_DEBUG(narratorMsgHlrLog, "tagid: " << ma.getTagid() << ", uri: " << ma.getUri() << ", md5: " << ma.getMd5() << ", size: " << ma.getSize() << ", length: " << ma.getLength());

    if(ma.isAudioDataNil()) {
        LOG4CXX_ERROR(narratorMsgHlrLog, "No audio data has been set");
        return -1;
    }

    if(!db->prepare("UPDATE messageaudio SET tagid=?, text=?, length=?, data=?, size=?, md5=? WHERE rowid=?")) {
        LOG4CXX_ERROR(narratorMsgHlrLog, "Query failed '" << db->getLasterror() << "'");
        return -1;
    }

    if(!db->bind(1, ma.getTagid()) ||
            !db->bind(2, ma.getText().c_str()) ||
            !db->bind(3, ma.getLength()) ||
            !db->bind(4, ma.getAudioData(), ma.getSize()) ||
            !db->bind(5, (long)ma.getSize()) ||
            !db->bind(6, ma.getMd5(), 32, NULL) ||
            !db->bind(7, audioid)) {
        LOG4CXX_ERROR(narratorMsgHlrLog, "Bind failed '" << db->getLasterror() << "'");
        return -1;
    }

    if(!db->perform()) {
        LOG4CXX_ERROR(narratorMsgHlrLog, "Query failed '" << db->getLasterror() << "'");
        return -1;
    }

    return audioid;
}

// Checks that data in audio is the same as in the database, if the values differ update them
long MessageHandler::checkAudio(long translationid, const MessageAudio &ma)
{
    long audioid = -1;
    string text = "";
    string md5 = "";
    size_t size = 0;
    int length= 0;

    if(!db->prepare("SELECT rowid, text, size, length, md5 FROM messageaudio WHERE translation_id=? AND tagid=?")) {
        LOG4CXX_ERROR(narratorMsgHlrLog, "Query failed '" << db->getLasterror() << "'");
        return -1;
    }

    if(!db->bind(1, translationid) ||
            !db->bind(2,ma.getTagid())) {
        LOG4CXX_ERROR(narratorMsgHlrLog, "Bind failed '" << db->getLasterror() << "'");
        return -1;
    }

    DBResult result;
    if(!db->perform(&result)) {
        LOG4CXX_ERROR(narratorMsgHlrLog, "Query failed '" << db->getLasterror() << "'");
        return -1;
    }

    int count = 0;
    while(result.loadRow()) {
        audioid = result.getInt(0);
        text = result.getText(1);
        size = result.getInt(2);
        length = result.getInt(3);
        md5 = result.getText(4);
        count++;
    }

    // If we already have a audio in the correct language, update the values if they differ
    bool differs = false;
    if(audioid > 0) {
        if(ma.getLength() != length) { differs = true; cout << "audio got new length: " << ma.getLength() << endl; };
        if(ma.getSize() != size) { differs = true; cout << "audio got new size: " << ma.getSize() << endl; };
        if(std::strcmp(ma.getMd5(), md5.c_str()) != 0) { differs = true; cout << "audio got new md5: " << ma.getMd5() << endl; };
        if(ma.getText() != text) { differs = true; cout << "audio got new text: " << ma.getText() << endl; };


        if(differs) updateAudio_with_id(audioid, ma);
    }

    return audioid;
}

long MessageHandler::insertAudio(long translationid, const MessageAudio &ma)
{
    LOG4CXX_INFO(narratorMsgHlrLog, "Inserting new messageaudio for translation with id: " << translationid);
    LOG4CXX_DEBUG(narratorMsgHlrLog, "tagid: " << ma.getTagid() << ", uri: " << ma.getUri() << ", md5: " << ma.getMd5() << ", size: " << ma.getSize() << ", length: " << ma.getLength());

    int audioid = -1;

    if(ma.isAudioDataNil()) {
        LOG4CXX_ERROR(narratorMsgHlrLog, "No audio data has been set");
        return -1;
    }

    if(!db->prepare("INSERT INTO messageaudio (translation_id, tagid, text, length, data, size, md5) VALUES (?, ?, ?, ?, ?, ?, ?)")) {
        LOG4CXX_ERROR(narratorMsgHlrLog, "Query failed '" << db->getLasterror() << "'");
        return -1;
    }

    if(!db->bind(1, translationid) ||
            !db->bind(2, ma.getTagid()) ||
            !db->bind(3, ma.getText().c_str()) ||
            !db->bind(4, ma.getLength()) ||
            !db->bind(5, ma.getAudioData(), ma.getSize()) ||
            !db->bind(6, (long)ma.getSize()) ||
            !db->bind(7, ma.getMd5(), 32, NULL)) {
        LOG4CXX_ERROR(narratorMsgHlrLog, "Bind failed '" << db->getLasterror() << "'");
        return -1;
    }

    DBResult result;
    if(!db->perform(&result)) {
        LOG4CXX_ERROR(narratorMsgHlrLog, "Query failed '" << db->getLasterror() << "'");
        return -1;
    }

    audioid = result.getInsertId();

    return audioid;
}
