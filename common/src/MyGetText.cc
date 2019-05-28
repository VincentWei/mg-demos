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
/*! ============================================================================
 * @file MyGetText.cc
 * @Synopsis
 * @author Vincent Wei
 * @version 1.0
 *  Copyright (C) 2018 Beijing Joobot Technologies Inc.
 */

#include "global.h"

#include "MyGetText.hh"

/* the C interface */
const char* mygettext (const char* msgid)
{
    return MyGetText::singleton()->gettext (msgid);
}

int mysetlanguage (const char* locale)
{
    if (strcmp (locale, BAD_LANG)) {
        int nr;

        char path [MAX_PATH + 1];
        if (realpath (GetResPath (), path) == NULL) {
            _ERR_PRINTF ("Failed to get the real path of resource.\n");
            return -1;
        }

        strcat (path, "/languages");
        nr = MYGETTEXT->loadText (path, locale, "LC_MESSAGES", "messages");
        if (nr < 0 && strcmp (locale, DEF_LANG) == 0) {
            MYGETTEXT->clear ();
        }
    }

    return -1;
}

MyGetText* MyGetText::s_service = NULL;

// get the single instance of MyGetText
MyGetText* MyGetText::singleton ()
{
    if (NULL == MyGetText::s_service) {
        MyGetText::s_service = new MyGetText();
    }

    return MyGetText::s_service;
}

const char* MyGetText::gettext (const char* msgid)
{
    if (NULL == MyGetText::s_service) {
        return msgid;
    }

    if (this->m_text_map.size () == 0) {
        return msgid;
    }

    TextMap::iterator it = this->m_text_map.find (msgid);
    if (it == this->m_text_map.end ()) {
        return msgid;
    }

    return this->m_text_map [msgid].c_str ();
}

int MyGetText::loadText (const char* dirname, const char* locale, const char* category, const char* domainname)
{
    std::string mo_file;

    if (dirname && strlen (dirname) > 0)
        mo_file = dirname;
    else
        mo_file = ".";

    if (mo_file [mo_file.length() - 1]!= '/') {
        mo_file += '/';
    }

    if (locale) {
        mo_file += locale;
        mo_file += '/';
    }

    if (category) {
        mo_file += category;
        mo_file += '/';
    }
    else {
        mo_file += "LC_MESSAGES/";
    }

    if (domainname) {
        mo_file += domainname;
        mo_file += ".mo";
    }
    else {
        mo_file += "messages.mo";
    }

    return this->loadText (mo_file.c_str ());
}

/*
    load text from a GNU MO file.

    Please refer to

https://www.gnu.org/software/gettext/manual/gettext.html#MO-Files

    for the format of GNU MO files, or see the following chart:

        byte
             +------------------------------------------+
          0  | magic number = 0x950412de                |
             |                                          |
          4  | file format revision = 0                 |
             |                                          |
          8  | number of strings                        |  == N
             |                                          |
         12  | offset of table with original strings    |  == O
             |                                          |
         16  | offset of table with translation strings |  == T
             |                                          |
         20  | size of hashing table                    |  == S
             |                                          |
         24  | offset of hashing table                  |  == H
             |                                          |
             .                                          .
             .    (possibly more entries later)         .
             .                                          .
             |                                          |
          O  | length & offset 0th string  ----------------.
      O + 8  | length & offset 1st string  ------------------.
              ...                                    ...   | |
O + ((N-1)*8)| length & offset (N-1)th string           |  | |
             |                                          |  | |
          T  | length & offset 0th translation  ---------------.
      T + 8  | length & offset 1st translation  -----------------.
              ...                                    ...   | | | |
T + ((N-1)*8)| length & offset (N-1)th translation      |  | | | |
             |                                          |  | | | |
          H  | start hash table                         |  | | | |
              ...                                    ...   | | | |
  H + S * 4  | end hash table                           |  | | | |
             |                                          |  | | | |
             | NUL terminated 0th string  <----------------' | | |
             |                                          |    | | |
             | NUL terminated 1st string  <------------------' | |
             |                                          |      | |
              ...                                    ...       | |
             |                                          |      | |
             | NUL terminated 0th translation  <---------------' |
             |                                          |        |
             | NUL terminated 1st translation  <-----------------'
             |                                          |
              ...                                    ...
             |                                          |
             +------------------------------------------+
*/
static void read_mo_data (MG_RWops* src, Uint32* dst, int count, bool be)
{
    if (be) {
        for (int i = 0; i < count; i++) {
            dst [i] = MGUI_ReadBE32 (src);
        }
    }
    else {
        for (int i = 0; i < count; i++) {
            dst [i] = MGUI_ReadLE32 (src);
        }
    }
}

int MyGetText::loadText (const char* mo_file)
{
    MG_RWops* src = MGUI_RWFromFile (mo_file, "r");
    bool be = false;

    if (src == NULL) {
        _ERR_PRINTF ("MyGetText::loadText: failed to open mo file: %s\n", mo_file);
        return -1;
    }

    Uint32 magic_number = MGUI_ReadLE32 (src);
    Uint32 revision;
    if (magic_number == 0x950412de) {
        be = false;
        revision = MGUI_ReadLE32 (src);
    }
    else if (magic_number == 0xde120495) {
        be = true;
        revision = MGUI_ReadBE32 (src);
    }
    else {
        MGUI_FreeRW (src);
        _ERR_PRINTF ("MyGetText::loadText: wrong magic number: 0x%X (%s)\n", magic_number, mo_file);
        return -2;
    }

    /* TODO check revision here */
    _MG_PRINTF ("MyGetText::loadText: revision number: 0x%X (%s)\n", revision, mo_file);

    Uint32 N;
    Uint32 O;
    Uint32 T;
    if (!be) {
        N = MGUI_ReadLE32 (src);
        O = MGUI_ReadLE32 (src);
        T = MGUI_ReadLE32 (src);
    }
    else {
        N = MGUI_ReadBE32 (src);
        O = MGUI_ReadBE32 (src);
        T = MGUI_ReadBE32 (src);
    }

    if (N == 0) {
        MGUI_FreeRW (src);
        _MG_PRINTF ("MyGetText::loadText: there is no any string: %s\n", mo_file);
        return 0;
    }

    this->m_text_map.clear ();

    Uint32* Olist = new Uint32 [N * 2];
    Uint32* Tlist = new Uint32 [N * 2];

    MGUI_RWseek (src, O, SEEK_SET);
    read_mo_data (src, Olist, N * 2, be);
    MGUI_RWseek (src, T, SEEK_SET);
    read_mo_data (src, Tlist, N * 2, be);

    for (unsigned i = 0; i < N; i++) {
        Uint32 orig_len = Olist [i * 2];
        char* orig_str = new char[orig_len + 1];
        if (orig_len != 0) {
            MGUI_RWseek (src, Olist [i * 2 + 1], SEEK_SET);
            MGUI_RWread (src, orig_str, sizeof (char), orig_len + 1);

        } else {
            orig_str [0] = '\0';
        }

        Uint32 tran_len = Tlist [i * 2];
        char* tran_str = new char[tran_len + 1];
        if (tran_str != 0) {
            MGUI_RWseek (src, Tlist[i * 2 + 1], SEEK_SET);
            MGUI_RWread (src, tran_str, sizeof (char), tran_len + 1);
        }
        else {
            tran_str [0] = '\0';
        }

        /* store to map */
        this->m_text_map [orig_str] = tran_str;

        /* check plural */
        size_t str_len = strlen (orig_str);
        if (orig_len > str_len) {
            char* plural = orig_str + str_len + 1;
            _MG_PRINTF ("MyGetText::loadText, skip plural %s in %s\n", plural, mo_file);
        }

        delete [] orig_str;
        delete [] tran_str;
    }

    delete [] Olist;
    delete [] Tlist;

    MGUI_FreeRW (src);
    return N;
}

