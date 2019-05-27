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

