/*!============================================================================
 * @file SettingLanguageActivity.hh 
 * @Synopsis  
 * @author Vincent Wei
 * @version 1.0
 * @date 19/05/2018
 *
 *  Copyright (C) 2018 Beijing Joobot Technogolies Inc.
 */

#ifndef _SETTING_LANGUAGE_ACTIVITY_HH_
#define _SETTING_LANGUAGE_ACTIVITY_HH_

class SettingLanguageActivity : public NCSActivity {

public:
    bool m_firstBoot;

    SettingLanguageActivity();
    ~SettingLanguageActivity();
};

#endif /* _SETTING_LANGUAGE_ACTIVITY_HH_ */
