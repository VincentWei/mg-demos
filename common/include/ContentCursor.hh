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
 * @file ContentCursor.hh
 * @Synopsis Interface of content cursor.
 * @author DongKai
 * @version 1.0
 *  Company: Beijing FMSoft Technology Co., Ltd.
 */

#ifndef  CONTENTCURSOR_INC
#define  CONTENTCURSOR_INC

#include "ContentValues.hh"

class ContentCursor {
public:
    virtual ~ContentCursor() {}
    virtual void close() = 0;
    virtual void deactivate() = 0;
    virtual int getColumnCount() const = 0;
    virtual int getColumnIndex(const std::string& name) const = 0;
    virtual std::string getString(const std::string& columnName) const = 0;
    virtual int getInt(const std::string& columnName, int defaultVal = 0) const = 0;
    virtual double getDouble(const std::string& columnName, double defaultVal = 0) const = 0;
    virtual size_t  getCount() const = 0;
    virtual ContentValues::Strings getColumnNames() const = 0;
    virtual int getPosition() const = 0;

    virtual bool isFirst() const = 0;
    virtual bool isLast() const = 0;
    virtual bool isClosed() const = 0;
    virtual bool moveToFirst() = 0;
    virtual bool moveToLast() = 0;
    virtual bool moveToNext() = 0;
    virtual bool moveToPosition(int pos) = 0;
    virtual bool moveToPrevious() = 0;

    virtual bool requery() = 0;
private:
    /* data */
};

#endif   /* ----- #ifndef CONTENTCURSOR_INC  ----- */

