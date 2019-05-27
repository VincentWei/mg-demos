/*!============================================================================
 * @file SettingTimeZoneActivity.hh 
 * @Synopsis  
 * @author Vincent Wei
 * @version 1.0
 * @date 09/11/2018
 *
 *  Copyright (C) 2018 Beijing Joobot Technogolies Inc.
 */

#ifndef _SETTING_TIME_ZONE_ACTIVITY_HH_
#define _SETTING_TIME_ZONE_ACTIVITY_HH_

class SettingTimeZoneActivity : public NCSActivity {

public:
    SettingTimeZoneActivity();
    ~SettingTimeZoneActivity();

    bool m_firstBoot;
};

const char* GetTimeZoneName (const char* tz);

#endif /* _SETTING_TIME_ZONE_ACTIVITY_HH_ */
