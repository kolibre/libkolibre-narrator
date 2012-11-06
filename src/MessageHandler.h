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

#ifndef _MESSAGEHANDLER_H
#define _MESSAGEHANDLER_H

#include "Message.h"
#include "Db.h"

using namespace std;

class MessageHandler
{
    public:
        MessageHandler();

        // adds/updates a message in the database, returns id if success -1 if not
        long updateMessage(const Message &msg);

        // finds a message in database, returns id if success -1 if not
        long findMessage(const Message &msg);

    private:
        DB *db;

        Message *currentMessage;
        MessageTranslation *currentMessageTranslation;
        MessageAudio *currentMessageAudio;
        string currentLanguage;

        int mCurrentTagid;

        // adds/updates a message in the database, returns id if success -1 if not
        long checkMessage(const Message &msg);
        long updateMessage_with_id(long messageid, const Message &msg);
        long insertMessage(const Message &msg);

        // adds/updates messageparameters in the database, returns id if success -1 if not
        long checkMessageParameters(long messageid, const Message &msg);
        long checkParameter(long messageid, const MessageParameter &mp);
        long updateParameter_with_id(long parameterid, const MessageParameter &mp);
        long insertParameter(long messageid, const MessageParameter &msg);

        // adds/updates a messagetranslation in the database, returns id if success -1 if not
        long checkMessageTranslation(long messageid, const MessageTranslation &mt);
        long checkTranslation(long messageid, const MessageTranslation &mt);
        long updateTranslation_with_id(long translationid, const MessageTranslation &mt);
        long insertTranslation(long messageid, const MessageTranslation &mt);

        // adds/updates a messageaudio in the database, returns id if success -1 if not
        long checkMessageAudio(long translationid, const MessageTranslation &mt);
        long checkAudio(long translationid, const MessageAudio &mt);
        long updateAudio_with_id(long audioid, const MessageAudio &mt);
        long insertAudio(long translationid, const MessageAudio &mt);
};

#endif
