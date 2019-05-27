/*!============================================================================
 * @file SettingHotspotActivity.hh 
 * @Synopsis  
 * @author Vincent Wei
 * @version 1.0
 * @date 10/09/2018
 *
 * Copyright (C) 2018 Beijing Joobot Technogolies Inc.
 */

#ifndef _SETTING_HOTSPOT_ACTIVITY_HH_
#define _SETTING_HOTSPOT_ACTIVITY_HH_

class SettingHotspotActivity : public NCSActivity {

protected:
    HOTSPOT_INFO m_info;

public:
    SettingHotspotActivity ();
    ~SettingHotspotActivity() {};
};

#endif /* _SETTING_HOTSPOT_ACTIVITY_HH_ */
