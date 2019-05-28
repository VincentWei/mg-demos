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
 * @file SQLiteDB.hh
 * @Synopsis SQLite DataBase source.
 * @author DongKai
 * @version 1.0
 *  Company: Beijing FMSoft Technology Co., Ltd.
 */

#ifndef  SQLITEDB_INC
#define  SQLITEDB_INC

#include <vector>
#include <map>
#include <sqlite3.h>

#include "ContentValues.hh"
#include "DataSource.hh"

class SQLiteDB : public DataSource {
public:
    enum {
        CACHE_PAGE_SIZE = 4096,
        MAX_PAGE_NUM = 128
    };
    typedef std::vector<ContentValues *> SQLCache;
    typedef std::map<std::string, SQLCache> SQLCacheItem;

    static int onQuery(void *NotUsed, int argc, char **argv, char **azColName);
    static const size_t s_cacheItemTotal = 168;

    SQLiteDB (const std::string& db_file, bool useCache);
    virtual ~SQLiteDB ();
    virtual ContentCursor* query(const std::string& table,
            const ContentValues::Strings* projection,
            const std::string* selection,
            const ContentValues::Strings* selectionArgs,
            const std::string* sortOrder);
    ContentCursor* query(const std::string& sql_cmd);
    virtual bool execSQL(const std::string& sql_cmd, bool is_query = false);

    virtual bool insert(const std::string& table, const ContentValues& values);
    virtual bool remove(const std::string& table, const std::string* selection,
            const ContentValues::Strings* selectionArgs);
    virtual bool update(const std::string& table, const ContentValues& values,
            const std::string* selection,
            const ContentValues::Strings* selectionArgs);
    const std::string& lastSQLCmd() {
        return m_lastSQLCmd;
    }

    const SQLCache& lastQueryResult() {
        return m_sqlResult;
    }

    bool queryFromCache(const std::string& sql_cmd);
    void cacheQueryResult(const std::string& cmd, const SQLCache& result);
    bool useQueryCache() {return m_useCache;}

protected:
    void refreshTableCache(const std::string& tableName);

private:
    std::string replaceSelectionArgs(const std::string& selection,
        const ContentValues::Strings& selectionArgs);

    sqlite3* m_dbConnection;
    SQLCache m_sqlResult;
    std::string m_lastSQLCmd;
    SQLCacheItem m_resultCache;
    bool m_useCache;
};
#endif   /* ----- #ifndef SQLITEDB_INC  ----- */

