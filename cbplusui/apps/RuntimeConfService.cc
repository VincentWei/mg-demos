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
 * @file RuntimeConfService.cc
 * @Synopsis
 * @author Vincent Wei
 * @version 1.0
 * @date 17/05/2018
 */

#include "global.h"
#include "resource.h"
#include "MyGetText.hh"
#include "RuntimeConfService.hh"

RuntimeConfService* RuntimeConfService::s_service = NULL;

// get the single instance of RuntimeConfService
RuntimeConfService* RuntimeConfService::singleton()
{
    if (NULL == RuntimeConfService::s_service) {
        RuntimeConfService::s_service = new RuntimeConfService();
    }
    return RuntimeConfService::s_service;
}

bool RuntimeConfService::load ()
{
    const char *env_value = NULL;

    if ((env_value = getenv (ENV_KEY_CFG_PATH))) {
        int len = strlen (env_value);

        if (env_value [len - 1] == '/') {
            strcpy (m_cfg_file_path, env_value);
        }
        else {
            strcpy (m_cfg_file_path, env_value);
            strcat (m_cfg_file_path, "/");
        }
    }
    else {
        strcpy (m_cfg_file_path, "./");
    }

    strcat (m_cfg_file_path, CFG_FILE_NAME);
    m_etc = LoadEtcFile (m_cfg_file_path);

    if (!m_etc) {
        _ERR_PRINTF ("Failed to load run-time config file (%s) from %s\n", CFG_FILE_NAME, m_cfg_file_path);
        return false;
    }

    return true;
}

bool RuntimeConfService::checkSettings ()
{
    if (m_etc == 0) {
        _ERR_PRINTF ("RuntimeConfService::checkSettings: please call RuntimeConfService::load() first.\n");
        return false;
    }

    char current_lang [LEN_LANG];

    if (GetValueFromEtc (m_etc, "language", "current", current_lang, LEN_LANG) != ETC_OK) {
        _ERR_PRINTF ("RuntimeConfService::checkSettings: failed to get value for language[current].\n");
        strcpy (current_lang, BAD_LANG);
    }

    if (strcmp (m_lang_settings->current, BAD_LANG) == 0) {
        return false;
    }

    return true;
}

bool RuntimeConfService::getLanguageSettings ()
{
    if (m_etc == 0) {
        _ERR_PRINTF ("RuntimeConfService::getLanguageSettings: please call RuntimeConfService::load() first.\n");
        return false;
    }

    int count;
    m_lang_settings = new LanguageSettings;

    if (GetIntValueFromEtc (m_etc, "language", "count", &count) != ETC_OK) {
        _ERR_PRINTF ("RuntimeConfService::getLanguageSettings: failed to get integer value for language[count].\n");
        delete m_lang_settings;
        m_lang_settings = NULL;
        return false;
    }

    if (count > 0) {
        for (int i = 0; i < count; i++) {
            char key_lang [16];
            char key_desc [16];
            char lang [LEN_LANG];
            char desc [LEN_DESC];

            sprintf (key_lang, "lang%d", i);
            sprintf (key_desc, "desc%d", i);

            if (GetValueFromEtc (m_etc, "language", key_lang, lang, LEN_LANG) != ETC_OK) {
                _ERR_PRINTF ("RuntimeConfService::getLanguageSettings: failed to get value for language[%s].\n", key_lang);
                continue;
            }

            if (GetValueFromEtc (m_etc, "language", key_desc, desc, LEN_DESC) != ETC_OK) {
                _ERR_PRINTF ("RuntimeConfService::getLanguageSettings: failed to get value for language[%s].\n", key_lang);
                continue;
            }

            m_lang_settings->langs.insert (LangEntry (lang, desc));
        }
    }
    else {
        _ERR_PRINTF ("RuntimeConfService::getLanguageSettings: invalid language[count]: %d.\n", count);
        delete m_lang_settings;
        m_lang_settings = NULL;
        return false;
    }

#ifdef OEM_TPP
    strcpy (m_lang_settings->current, "zh_CN");
#else
    if (GetValueFromEtc (m_etc, "language", "current", m_lang_settings->current, LEN_LANG) != ETC_OK) {
        _ERR_PRINTF ("RuntimeConfService::getLanguageSettings: failed to get value for language[current].\n");
        strcpy (m_lang_settings->current, BAD_LANG);
    }

    if (strcmp (m_lang_settings->current, BAD_LANG)) {
        LangMap::iterator it = m_lang_settings->langs.find (m_lang_settings->current);
        if (it == m_lang_settings->langs.end ()) {
            _ERR_PRINTF ("RuntimeConfService::getLanguageSettings: invalid current language: %s\n", m_lang_settings->current);
            strcpy (m_lang_settings->current, BAD_LANG);
        }
    }
#endif

    if (mysetlanguage (m_lang_settings->current) >= 0)
        return true;

    return false;
}

bool RuntimeConfService::setCurrentLanguage (const char* lang)
{
    if (m_etc == 0) {
        _ERR_PRINTF ("RuntimeConfService::setCurrentLanguage: please call RuntimeConfService::load() first.\n");
        return false;
    }

    if (m_lang_settings == 0) {
        _ERR_PRINTF ("RuntimeConfService::setCurrentLanguage: please call RuntimeConfService::getLanguageSettings() first.\n");
        return false;
    }

    LangMap::iterator it = m_lang_settings->langs.find (lang);
    if (it == m_lang_settings->langs.end ()) {
        _ERR_PRINTF ("RuntimeConfService::setCurrentLanguage: invalid current language: %s\n", lang);
        return false;
    }

    strcpy (m_lang_settings->current, lang);
    SetValueToEtc (m_etc, "language", "current", m_lang_settings->current);

    if (mysetlanguage (lang) >= 0)
        return true;

    return false;

}

const char* RuntimeConfService::getScreenLockPassword ()
{
    if (m_etc == 0) {
        _ERR_PRINTF ("RuntimeConfService::getScreenLockPassword: please call RuntimeConfService::load first.\n");
        return NULL;
    }

    if (strlen (m_enc_passwd) == 0) {
        if (GetValueFromEtc (m_etc, "screenlock", "password", m_enc_passwd, LEN_ENC_PASSWD) != ETC_OK) {
            _ERR_PRINTF ("RuntimeConfService::getScreenLockPassword: can not get password.\n");
            return NULL;
        }
    }

    _MG_PRINTF ("RuntimeConfService::getScreenLockPassword: current setting: %s.\n", m_enc_passwd);
    return m_enc_passwd;
}

bool RuntimeConfService::setScreenLockPassword (const char* new_passwd)
{
    if (m_etc == 0) {
        _ERR_PRINTF ("RuntimeConfService::setValue: please call RuntimeConfService::load first.\n");
        return false;
    }

    strncpy (m_enc_passwd, new_passwd, LEN_ENC_PASSWD);

    if (SetValueToEtc (m_etc, "screenlock", "password", m_enc_passwd) != ETC_OK) {
        return false;
    }

    return true;
}

int RuntimeConfService::getScreenLockTimeout ()
{
    int timeout;

    if (m_etc == 0) {
        _ERR_PRINTF ("RuntimeConfService::getScreenLockTimeout: please call RuntimeConfService::load first.\n");
        return DEF_TIMEOUT_SCREEN_LOCK;
    }

    if (GetIntValueFromEtc (m_etc, "screenlock", "timeout", &timeout) != ETC_OK) {
        return DEF_TIMEOUT_SCREEN_LOCK;
    }

    return timeout;
}

const char* RuntimeConfService::getScreenLockTimeoutText ()
{
    int timeout = getScreenLockTimeout ();

    switch (timeout) {
    case (30*60):
        return _("30 minutes");
    case (15*60):
        return _("15 minutes");
    case (5*60):
        return _("5 minutes");
    case (60):
        return _("60 seconds");
    case (30):
        return _("30 seconds");
    default:
        break;
    }

    return _("Unknown");
}

bool RuntimeConfService::setScreenLockTimeout (int timeout)
{
    if (m_etc == 0) {
        _ERR_PRINTF ("RuntimeConfService::setScreenLockTimeout: please call RuntimeConfService::load first.\n");
        return false;
    }

    if (timeout <= 0)
        timeout = DEF_TIMEOUT_SCREEN_LOCK;

    char buff [16];
    sprintf (buff, "%d", timeout);
    if (SetValueToEtc (m_etc, "screenlock", "timeout", buff) != ETC_OK) {
        return false;
    }

    return true;
}

#if 0
static void set_time_zone (const char* tz)
{
    char buf [LEN_TIMEZONE + 1];

    strncpy (buf, tz, LEN_TIMEZONE);

    /* trim the postfix */
    char* ch = buf;
    while (*ch) {
        if (*ch == '#') {
            *ch = '\0';
            break;
        }
        ch++;
    }

    setenv ("TZ", buf, 1);
    tzset ();
}
#endif

const char* RuntimeConfService::getTimeZone ()
{
    if (m_etc == 0) {
        _ERR_PRINTF ("RuntimeConfService::getTimeZone: please call RuntimeConfService::load first.\n");
        return DEF_TIMEZONE;
    }

    if (GetValueFromEtc (m_etc, "time", "timezone", m_time_zone, LEN_TIMEZONE) != ETC_OK) {
        return DEF_TIMEZONE;
    }

    return m_time_zone;
}

bool RuntimeConfService::setTimeZone (const char* tz)
{
    if (m_etc == 0) {
        _ERR_PRINTF ("RuntimeConfService::setTimeZone: please call RuntimeConfService::load first.\n");
        return false;
    }

    strncpy (m_time_zone, tz, LEN_TIMEZONE);
    if (SetValueToEtc (m_etc, "time", "timezone", m_time_zone) != ETC_OK) {
        return false;
    }

    return true;
}

bool RuntimeConfService::save ()
{
    if (m_etc == 0) {
        _ERR_PRINTF ("RuntimeConfService::save: please call RuntimeConfService::load first.\n");
        return false;
    }

    return (SaveEtcToFile (m_etc, m_cfg_file_path) == ETC_OK);
}

// private:
RuntimeConfService::RuntimeConfService ()
{
    m_etc = 0;
    m_lang_settings = NULL;
    strcpy (m_enc_passwd, "");
}

RuntimeConfService::~RuntimeConfService ()
{
    if (m_etc) {
        UnloadEtcFile (m_etc);
    }

    if (m_lang_settings) {
        m_lang_settings->langs.clear ();
        delete m_lang_settings;
    }
}

