/*!============================================================================
 * @file SettingWifiListActivity.hh 
 * @Synopsis  
 * @author Vincent Wei
 * @version 1.0
 * @date 19/05/2018
 *
 *  Copyright (C) 2018 Beijing Joobot Technogolies Inc.
 */

#ifndef _SETTING_WIFI_LIST_ACTIVITY_HH_
#define _SETTING_WIFI_LIST_ACTIVITY_HH_

class SettingWifiListActivity : public NCSActivity {

public:
    bool m_firstBoot;

    AsyncTaskGetHotspotList* m_async_task;
    
    SettingWifiListActivity();
    ~SettingWifiListActivity();
};

#endif /* _SETTING_WIFI_LIST_ACTIVITY_HH_ */
