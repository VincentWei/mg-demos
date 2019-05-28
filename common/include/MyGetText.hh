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
 * @file MyGetText.hh
 * @Synopsis
 * @author Vincent Wei
 * @version 1.0
 * @date 05/21/2018
 *
 *  Copyright (C) 2018 Beijing Joobot Technologies Inc.
 */

#ifndef _MYGETTEXT_H
#define _MYGETTEXT_H

#include <string>
#include <map>

#define BAD_LANG    "na_NA"
#define DEF_LANG    "en_US"

typedef std::map<std::string, std::string> TextMap;

class MyGetText
{
public:
    int loadText (const char* dirname, const char* locale, const char* category, const char* domainname);
    int loadText (const char* mo_file);

    const char* gettext (const char* msgid);

    void clear () {
        m_text_map.clear ();
    }

    // get the single instance of MyGetText
    static MyGetText* singleton ();

private:
    TextMap m_text_map;

    MyGetText () { };
    ~MyGetText() {
        m_text_map.clear ();
    };

    static MyGetText* s_service;

    class Gc
    {
    public:
        ~Gc() {
            if (MyGetText::s_service)
                delete MyGetText::s_service;
        }
    };

    static Gc gc;
};

#define MYGETTEXT MyGetText::singleton()

#ifdef __cplusplus
extern "C" {
#endif

int mysetlanguage (const char* locale);

#ifdef __cplusplus
}
#endif

#endif

