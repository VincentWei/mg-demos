/*!============================================================================
 * @file RuntimeConfService.hh 
 * @Synopsis  
 * @author Vincent Wei
 * @version 1.0
 * @date 17/05/2018
 *
 * Copyright (C) 2018 Beijing Joobot Technologies Co., Ltd.
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
