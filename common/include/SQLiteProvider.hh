/*
** This file is a part of mg-demos package.
**
** Copyright (C) 2010 ~ 2019 FMSoft (http://www.fmsoft.cn).
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/
/*!============================================================================
 * @file SQLiteProvider.hh
 * @Synopsis
 * @author DongKai
 * @version 1.0
 *  Company: Beijing FMSoft Technology Co., Ltd.
 */

#ifndef  SQLITEPROVIDER_INC
#define  SQLITEPROVIDER_INC
#include <stdio.h>
#include <stdlib.h>

#include "ContentProvider.hh"
#include "SQLiteDB.hh"

class SQLiteProvider : public ContentProvider {
public:
    SQLiteProvider (const std::string& db_name, bool useCache = true) :
        m_db(db_name, useCache)
    {}
    virtual ~SQLiteProvider ()
    {
#ifdef DEBUG
        printf ("~SQLiteProvider.......................\n");
#endif
    }

    ContentCursor* query(const std::string& uri, const ContentValues::Strings* projection, const std::string*
                    selection, const ContentValues::Strings* selectionArgs, const std::string* sortOrder) {
        return m_db.query(ContentProvider::getDomainFromURI(uri), projection, selection,
                selectionArgs, sortOrder);
    }

    int remove(const std::string& uri, const std::string* selection, const ContentValues::Strings* selectionArgs) {
        m_db.remove(ContentProvider::getDomainFromURI(uri), selection, selectionArgs);
        return -1;
    }

    int update(const std::string& uri, const ContentValues& values,
            const std::string* selection, const ContentValues::Strings* selectionArgs) {
        m_db.update(ContentProvider::getDomainFromURI(uri), values, selection, selectionArgs);
        return -1;
    }

    std::string insert(const std::string& uri, const ContentValues& values) {
        m_db.insert(ContentProvider::getDomainFromURI(uri), values);
        return "";
    }

protected:
    SQLiteDB m_db;

private:
    SQLiteProvider();
};

#endif   /* ----- #ifndef SQLITEPROVIDER_INC  ----- */

