/*!============================================================================
 * @file SettingCheckNetworkActivity.hh 
 * @Synopsis  
 * @author Vincent Wei
 * @version 1.0
 * @date 20/05/2018
 *
 *  Copyright (C) 2018 Beijing Joobot Technogolies Inc.
 */

#ifndef _SETTING_CHECK_NETWORK_ACTIVITY_HH_
#define _SETTING_CHECK_NETWORK_ACTIVITY_HH_

class SettingCheckNetworkActivity : public NCSActivity {

public:
    bool m_firstBoot;

    SettingCheckNetworkActivity();
    ~SettingCheckNetworkActivity();
};

#endif /* _SETTING_CHECK_NETWORK_ACTIVITY_HH_ */
