/*!============================================================================
 * @file SettingWifiPasswordActivity.hh 
 * @Synopsis  
 * @author Vincent Wei
 * @version 1.0
 * @date 23/05/2018
 *
 *  Copyright (C) 2018 Beijing Joobot Technogolies Inc.
 */

#ifndef _SETTING_WIFI_PASSWROD_ACTIVITY_HH_
#define _SETTING_WIFI_PASSWROD_ACTIVITY_HH_

typedef struct _WIFI_PASSWORD_INFO {
    char* ssid;
    char* password;
} WIFI_PASSWORD_INFO;

class SettingWifiPasswordActivity : public NCSActivity {

public:
    SettingWifiPasswordActivity ();

private:
    ~SettingWifiPasswordActivity() {};
};

#endif /* _SETTING_WIFI_PASSWROD_ACTIVITY_HH_ */
