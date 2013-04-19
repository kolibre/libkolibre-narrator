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

#ifndef _NARRATOR_DB_H
#define _NARRATOR_DB_H

#include <sqlite3.h>
#include <string>

using namespace std;

class DBResult;

namespace narrator {

class DB {
    public:
        DB(const string &);
        DB(sqlite3 *handle);
        ~DB();

        const string &getDatabase() { return mDatabase; };
        sqlite3 *getHandle() { return pDBHandle; };

        // returns true if connected OK, false if not
        bool connect();

        // returns true if we have an active databasehandle
        bool isOpen() { return (pDBHandle != NULL); };

        // constructs a new query
        bool prepare(const char *);

        // resets parameters bound to query
        bool reset();

        // returns id of an argument, -1 if not found
        bool bind(const int idx, const int value);
        bool bind(const char *key, const int value);

        bool bind(const int idx, const long value);
        bool bind(const char *key, const long value);

        bool bind(const int idx, const double value);
        bool bind(const char *key, const double value);

        bool bind(const int idx, const char *value, int length = -1, void(*freefunc)(void*) = NULL);
        bool bind(const char *key,  const char *value, int length = -1, void(*freefunc)(void*) = NULL);

        bool bind(const int idx, const void *value, int length, void(*freefunc)(void*));
        bool bind(const char *key,  const void *value, int length, void(*freefunc)(void*));

        // Perform the query, return true and fill in dbresult if successful,
        // if unsuccessful dbresult is NULL and return false
        bool perform(DBResult *result = NULL);

        string getLasterror() { return mLastquery + ":" + mLasterror; };
        bool verifyDBStructure();

    private:
        sqlite3 *pDBHandle;
        sqlite3_stmt *pStatement;
        string mLasterror;
        string mLastquery;
        string mDatabase;
        bool bClosedb;
        int rc;
};

}

class DBResult {
    public:
        DBResult();
        ~DBResult();

        bool setup(sqlite3 *handle, sqlite3_stmt* statement);

        bool loadRow();
        bool isError();
        bool isDone();

        long getInt(long column);
        double getDouble(long column);
        const char *getText(long column);
        const void  *getData(long column);
        long  getDataSize(long column);

        long getInsertId();

        const string &getLasterror() { return mLasterror; };

        // debug
        void printRow();

    private:
        int step();
        string mLasterror;
        int rc;

        bool bFirstcall;
        bool bError;
        bool bDone;

        sqlite3 *pDBHandle;
        sqlite3_stmt *pStatement;
};

#endif
