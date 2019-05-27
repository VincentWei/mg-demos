/*!============================================================================
 * @file SettingCheckNetworkPromptActivity.hh 
 * @Synopsis  
 * @author Vincent Wei
 * @version 1.0
 * @date 20/05/2018
 *
 *  Copyright (C) 2018 Beijing Joobot Technogolies Inc.
 */

#ifndef _SETTING_CHECK_NETWORK_PROMPT_ACTIVITY_HH_
#define _SETTING_CHECK_NETWORK_PROMPT_ACTIVITY_HH_

class SettingCheckNetworkPromptActivity : public NCSActivity {

public:
    bool m_firstBoot;

    SettingCheckNetworkPromptActivity();
    ~SettingCheckNetworkPromptActivity() {};
};

#endif /* _SETTING_CHECK_NETWORK_PROMPT_ACTIVITY_HH_ */
