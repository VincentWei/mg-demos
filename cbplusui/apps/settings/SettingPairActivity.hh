/*!============================================================================
 * @file SettingPairActivity.hh 
 * @Synopsis  
 * @author Vincent Wei
 * @version 1.0
 * @date 25/05/2018
 *
 * Copyright (C) 2018 Beijing Joobot Technogolies Inc.
 */

#ifndef _SETTING_PAIR_ACTIVITY_HH_
#define _SETTING_PAIR_ACTIVITY_HH_

class SettingPairActivity : public NCSActivity {

public:
    bool m_firstBoot;

    SettingPairActivity ();
    ~SettingPairActivity() {};
};

#endif /* _SETTING_PAIR_ACTIVITY_HH_ */
