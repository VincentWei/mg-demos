///////////////////////////////////////////////////////////////////////////////
//
//                          IMPORTANT NOTICE
//
// The following open source license statement does not apply to any
// entity in the Exception List published by FMSoft.
//
// For more information, please visit:
//
// https://www.fmsoft.cn/exception-list
//
//////////////////////////////////////////////////////////////////////////////
/*
** This file is a part of mg-demos package.
**
** Copyright (C) 2019 FMSoft (http://www.fmsoft.cn).
** Copyright (C) 2018 Beijing Joobot Technologies Co., Ltd.
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
 * @file RuntimeConfService.hh
 * @Synopsis
 * @author Vincent Wei
 * @version 1.0
 * @date 17/05/2018
 */

#include <string>
#include <map>

#ifndef _RUNTIMECONFSERVICE_H
#define _RUNTIMECONFSERVICE_H

#define LEN_LANG        8
#define LEN_TIMEZONE    48
#define LEN_DESC        32

#define LEN_ENC_PASSWD  255

typedef std::pair<std::string, std::string> LangEntry;
typedef std::map<std::string, std::string> LangMap;

struct LanguageSettings
{
    LangMap langs;
    char current [LEN_LANG];
};

/* seconds */
#define DEF_TIMEOUT_SCREEN_LOCK     30
#define DEF_TIMEZONE                "CST"

class RuntimeConfService
{
public:
    static RuntimeConfService* singleton ();
    bool load ();
    bool save ();

    bool checkSettings ();
    bool getLanguageSettings ();
    bool setCurrentLanguage (const char* lang);

    const char* getScreenLockPassword ();
    bool setScreenLockPassword (const char* new_passwd);

    int getScreenLockTimeout ();
    const char* getScreenLockTimeoutText ();
    bool setScreenLockTimeout (int timeout);

    const char* getTimeZone ();
    bool setTimeZone (const char* tz);

    LanguageSettings* m_lang_settings;

private:
    GHANDLE m_etc;
    char m_cfg_file_path [MAX_PATH + 1];

    char m_time_zone [LEN_TIMEZONE + 1];
    char m_enc_passwd [LEN_ENC_PASSWD + 1];

    RuntimeConfService ();
    ~RuntimeConfService ();

    static RuntimeConfService* s_service;

    class Gc
    {
    public:
        ~Gc()
        {
            if (RuntimeConfService::s_service)
                delete RuntimeConfService::s_service;
        }
    };

    static Gc gc;
};

#define RTCFGSERVICE RuntimeConfService::singleton()

#endif
