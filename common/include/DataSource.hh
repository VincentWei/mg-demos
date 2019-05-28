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
 * @file DataSource.hh
 * @Synopsis DataSource interface.
 * @author DongKai
 * @version 1.0
 *  Company: Beijing FMSoft Technology Co., Ltd.
 */

#ifndef  DATASOURCE_INC
#define  DATASOURCE_INC

#include <string>

class ContentCursor;
class ContentValues;

class DataSource {
    public:
        virtual ContentCursor* query(const std::string& table,
                 const ContentValues::Strings* projection,
                 const std::string* selection,
                 const ContentValues::Strings* selectionArgs,
                 const std::string* sortOrder) = 0;

        virtual bool insert(const std::string& table, const ContentValues& values) = 0;
        virtual bool remove(const std::string& table, const std::string* selection,
                 const ContentValues::Strings* selectionArgs) = 0;
        virtual bool update(const std::string& table, const ContentValues& values,
                const std::string* selection,
                const ContentValues::Strings* selectionArgs) = 0;

};

#endif   /* ----- #ifndef DATASOURCE_INC  ----- */
