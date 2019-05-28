#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <minigui/common.h>

#include "cJSON.h"
#include "jbus.h"

#define SYS_TRANSFER_PROTOCOL_JOOPIC    0
#define SYS_TRANSFER_PROTOCOL_FTP       1

#define UPLOAD_FILTER_JPEG          1
#define UPLOAD_FILTER_RAW           2
#define UPLOAD_FILTER_JPEG_RAW      3
#define UPLOAD_FILTER_JPEG_IN_RAW   4
#define UPLOAD_FILTER_SMALL_JPEG    5

#define RC_OK                       0
#define RC_BAD_PARAM                40009

#define RC_FTP_ERR_CONNECT          40046
#define RC_FTP_ERR_MKDIR            40047
#define RC_FTP_ERR_CD               40048
#define RC_FTP_ERR_PUT              40049
#define RC_FTP_ERR_DELETE           40050
#define RC_FTP_ERR_RMDIR            40051

#define LEN_TIMEZONE        127
#define LEN_FTP_HOST        127
#define LEN_FTP_PORT        31
#define LEN_FTP_USER        127
#define LEN_FTP_PASSWORD    127
#define LEN_WIFI_SSID       127
#define LEN_WIFI_PASSWORD   127

#define NETWORK_TYPE_MOBILE         0
#define NETWORK_TYPE_WIFI           1
#define NETWORK_TYPE_BLUETOOTH      2
#define NETWORK_TYPE_ETHERNE        3

#define NETWORK_STATE_CONNECTED         0
#define NETWORK_STATE_CONNECTING        1
#define NETWORK_STATE_SUSPENDED         2
#define NETWORK_STATE_DISCONNECTING     3
#define NETWORK_STATE_DISCONNECTED      4
#define NETWORK_STATE_NO_MEDIA          5
#define NETWORK_STATE_DISABLED          6
#define NETWORK_STATE_ABSENT            7

#define NETWORK_STATE_UNKNOWN           255

typedef struct _DeviceState {
    char timeZone[LEN_TIMEZONE + 1];
    int transferMode;

    int auc_filter;
    int auc_autoDel;
    int auc_writeLoc;

    int muc_filter;
    int muc_autoDel;
    int muc_writeLoc;

    char ftpHost[LEN_FTP_HOST + 1];
    char ftpPort[LEN_FTP_PORT + 1];
    char ftpUser[LEN_FTP_USER + 1];
    char ftpPassword[LEN_FTP_PASSWORD + 1];

    int lteState;
    int lteSignal;

    int  wifiState;
    char wifiSSID[LEN_WIFI_SSID + 1];
    char wifiPass[LEN_WIFI_PASSWORD + 1];
    int  wifiAp;
    char wifiApSSID[LEN_WIFI_SSID + 1];
    char wifiApPass[LEN_WIFI_PASSWORD + 1];
    int  wifiSignal;
} DeviceState;

static DeviceState device_state;

static void init_device_state(void)
{
    strcpy(device_state.timeZone, "Asia/Shanghai");
    device_state.transferMode = SYS_TRANSFER_PROTOCOL_JOOPIC;

    device_state.auc_filter = UPLOAD_FILTER_JPEG;
    device_state.auc_autoDel = 0;
    device_state.auc_writeLoc = 0;

    device_state.muc_filter = UPLOAD_FILTER_JPEG;
    device_state.muc_autoDel = 0;
    device_state.muc_writeLoc = 0;

    device_state.ftpHost[0] = 0;
    device_state.ftpPort[0] = 0;
    device_state.ftpUser[0] = 0;
    device_state.ftpPassword[0] = 0;

    device_state.lteState = NETWORK_STATE_CONNECTED;
    device_state.lteSignal = 75;
    device_state.wifiState = NETWORK_STATE_DISCONNECTED;
    device_state.wifiSignal = 0;
    device_state.wifiAp = 0;
}

static void set_sys_time_zone (const char* tz)
{
    char buf [LEN_TIMEZONE + 1];

    strncpy (buf, tz, LEN_TIMEZONE);

    /* trim the postfix */
    char* ch = buf;
    while (*ch) {
        if (*ch == '#') {
            *ch = '\0';
            break;
        }
        ch++;
    }

    setenv ("TZ", buf, 1);
    tzset ();
}

static int set_device_time_zone(const char* tz)
{
    memset(device_state.timeZone, 0, LEN_TIMEZONE + 1);
    strncpy(device_state.timeZone, tz, LEN_TIMEZONE);
    set_sys_time_zone(tz);
    return RC_OK;
}

static int set_device_transfer_mode(int mode)
{
    if (mode == SYS_TRANSFER_PROTOCOL_JOOPIC ||
            mode == SYS_TRANSFER_PROTOCOL_FTP) {
        device_state.transferMode = mode;
        return RC_OK;
    }

    return RC_BAD_PARAM;
}

static int set_device_auc_filter(int filter)
{
    if (filter >= UPLOAD_FILTER_JPEG ||
            filter <= UPLOAD_FILTER_SMALL_JPEG) {
        device_state.auc_filter = filter;
        return RC_OK;
    }

    return RC_BAD_PARAM;
}

static int set_device_auc_autodel(int autoDel)
{
    if (autoDel) {
        device_state.auc_autoDel = 1;
    }
    else
        device_state.auc_autoDel = 0;

    return RC_OK;
}

static int set_device_auc_writeloc(int writeLoc)
{
    if (writeLoc) {
        device_state.auc_writeLoc = 1;
    }
    else
        device_state.auc_writeLoc = 0;

    return RC_OK;
}

static int set_device_muc_filter(int filter)
{
    if (filter >= UPLOAD_FILTER_JPEG ||
            filter <= UPLOAD_FILTER_SMALL_JPEG) {
        device_state.muc_filter = filter;
        return RC_OK;
    }

    return RC_BAD_PARAM;
}

static int set_device_muc_autodel(int autoDel)
{
    if (autoDel) {
        device_state.muc_autoDel = 1;
    }
    else
        device_state.muc_autoDel = 0;

    return RC_OK;
}

static int set_device_muc_writeloc(int writeLoc)
{
    if (writeLoc) {
        device_state.muc_writeLoc = 1;
    }
    else
        device_state.muc_writeLoc = 0;

    return RC_OK;
}

static int set_ftp_config(const char* host, const char* port,
        const char* user, const char* password)
{
    int rc;

    sleep (3);
    rc = time(NULL) % 10;

    if (rc <= 5) {
        rc = RC_OK;
        memset(device_state.ftpHost, 0, LEN_FTP_HOST + 1);
        strncpy(device_state.ftpHost, host, LEN_FTP_HOST);
        memset(device_state.ftpPort, 0, LEN_FTP_PORT + 1);
        strncpy(device_state.ftpPort, port, LEN_FTP_PORT);
        memset(device_state.ftpUser, 0, LEN_FTP_USER + 1);
        strncpy(device_state.ftpUser, user, LEN_FTP_USER);
        memset(device_state.ftpPassword, 0, LEN_FTP_PASSWORD + 1);
        strncpy(device_state.ftpPassword, password, LEN_FTP_PASSWORD);
    }
    else {
        rc -= 6;
        rc += RC_FTP_ERR_CONNECT;
    }

    return rc;
}

static char* on_config_method (const char* method, cJSON* msg)
{
    char* res = NULL;

    if (strcmp(method, "getAllConfig") == 0) {

        static const char* all_cfg = "{"
            "\"retCode\": 0,"
            "\"data\": {"
                "\"config\": ["
                    "{"
                        "\"module\": \"device\","
                        "\"key\": \"hardwareId\","
                        "\"value\": \"6c1180001f588917881020090461872a\""
                    "},"
                    "{"
                        "\"module\": \"device\","
                        "\"key\": \"sn\","
                        "\"value\": \"1600100301561633\""
                    "},"
                    "{"
                        "\"module\": \"device\","
                        "\"key\": \"model\","
                        "\"value\": \"CBPlus\""
                    "},"
                    "{"
                        "\"module\": \"os\","
                        "\"key\": \"verCode\","
                        "\"value\": \"100\""
                    "},"
                    "{"
                        "\"module\": \"os\","
                        "\"key\": \"verName\","
                        "\"value\": \"2.0.0\""
                    "},"
                    "{"
                        "\"module\": \"os\","
                        "\"key\": \"timeZone\","
                        "\"value\": \"%s\""
                    "},"
                    "{"
                        "\"module\": \"user\","
                        "\"key\": \"userId\","
                        "\"value\": \"1234567890\""
                    "},"
                    "{"
                        "\"module\": \"user\","
                        "\"key\": \"joobotId\","
                        "\"value\": \"13501001000\""
                    "},"
                    "{"
                        "\"module\": \"user\","
                        "\"key\": \"nickname\","
                        "\"value\": \"暖男\""
                    "},"
                    "{"
                        "\"module\": \"user\","
                        "\"key\": \"transferMode\","
                        "\"value\": %d"
                    "},"
                    "{"
                        "\"module\": \"autoUploadCloud\","
                        "\"key\": \"filter\","
                        "\"value\": %d"
                    "},"
                    "{"
                        "\"module\": \"autoUploadCloud\","
                        "\"key\": \"autoDel\","
                        "\"value\": %d"
                    "},"
                    "{"
                        "\"module\": \"autoUploadCloud\","
                        "\"key\": \"writeLoc\","
                        "\"value\": %d"
                    "},"
                    "{"
                        "\"module\": \"manualUploadCloud\","
                        "\"key\": \"filter\","
                        "\"value\": %d"
                    "},"
                    "{"
                        "\"module\": \"manualUploadCloud\","
                        "\"key\": \"autoDel\","
                        "\"value\": %d"
                    "},"
                    "{"
                        "\"module\": \"manualUploadCloud\","
                        "\"key\": \"writeLoc\","
                        "\"value\": %d"
                    "},"
                    "{"
                        "\"module\": \"ftp\","
                        "\"key\": \"host\","
                        "\"value\": \"%s\""
                    "},"
                    "{"
                        "\"module\": \"ftp\","
                        "\"key\": \"port\","
                        "\"value\": \"%s\""
                    "},"
                    "{"
                        "\"module\": \"ftp\","
                        "\"key\": \"user\","
                        "\"value\": \"%s\""
                    "},"
                    "{"
                        "\"module\": \"ftp\","
                        "\"key\": \"password\","
                        "\"value\": \"%s\""
                    "}"
                "]"
            "}"
        "}";

        res = calloc(sizeof(char), strlen(all_cfg) + sizeof(DeviceState) + 1);
        if (res) {
            sprintf(res, all_cfg,
                device_state.timeZone,
                device_state.transferMode,

                device_state.auc_filter,
                device_state.auc_autoDel,
                device_state.auc_writeLoc,

                device_state.muc_filter,
                device_state.muc_autoDel,
                device_state.muc_writeLoc,

                device_state.ftpHost,
                device_state.ftpPort,
                device_state.ftpUser,
                device_state.ftpPassword);
        }
    }
    else if (strcmp(method, "getModuleConfig") == 0) {
        cJSON* module = cJSON_GetObjectItem (msg, "module");
        if (module == NULL || module->type != cJSON_String) {
            goto error;
        }

        if (strcasecmp (module->valuestring, "device") == 0) {
            static const char* device_cfg = "{"
                "\"retCode\": 0,"
                "\"data\": {"
                    "\"config\": ["
                        "{"
                            "\"module\": \"device\","
                            "\"key\": \"hardwareId\","
                            "\"value\": \"6c1180001f588917881020090461872a\""
                        "},"
                        "{"
                            "\"module\": \"device\","
                            "\"key\": \"sn\","
                            "\"value\": \"1600100301561633\""
                        "},"
                        "{"
                            "\"module\": \"device\","
                            "\"key\": \"model\","
                            "\"value\": \"CBPlus\""
                        "}"
                    "]"
                "}"
            "}";

            res = calloc(sizeof(char), strlen(device_cfg) + 1);
            if (res) {
                strcpy(res, device_cfg);
            }
        }
        else if (strcasecmp (module->valuestring, "os") == 0) {
            static const char* os_cfg = "{"
                "\"retCode\": 0,"
                "\"data\": {"
                    "\"config\": ["
                        "{"
                            "\"module\": \"os\","
                            "\"key\": \"verCode\","
                            "\"value\": \"100\""
                        "},"
                        "{"
                            "\"module\": \"os\","
                            "\"key\": \"verName\","
                            "\"value\": \"2.0.0\""
                        "},"
                        "{"
                            "\"module\": \"os\","
                            "\"key\": \"timeZone\","
                            "\"value\": \"%s\""
                        "}"
                    "]"
                "}"
            "}";

            res = calloc(sizeof(char), strlen(os_cfg) + sizeof(DeviceState) + 1);
            if (res) {
                sprintf(res, os_cfg, device_state.timeZone);
            }
        }
        else if (strcasecmp (module->valuestring, "user") == 0) {
            static const char* user_cfg = "{"
                "\"retCode\": 0,"
                "\"data\": {"
                    "\"config\": ["
                        "{"
                            "\"module\": \"user\","
                            "\"key\": \"userId\","
                            "\"value\": \"1234567890\""
                        "},"
                        "{"
                            "\"module\": \"user\","
                            "\"key\": \"joobotId\","
                            "\"value\": \"13501001000\""
                        "},"
                        "{"
                            "\"module\": \"user\","
                            "\"key\": \"nickname\","
                            "\"value\": \"暖男\""
                        "},"
                        "{"
                            "\"module\": \"user\","
                            "\"key\": \"transferMode\","
                            "\"value\": %d"
                        "}"
                    "]"
                "}"
            "}";

            res = calloc(sizeof(char), strlen(user_cfg) + sizeof(DeviceState) + 1);
            if (res) {
                sprintf(res, user_cfg, device_state.transferMode);
            }
        }
        else if (strcasecmp (module->valuestring, "autoUploadCloud") == 0) {
            static const char* my_cfg = "{"
                "\"retCode\": 0,"
                "\"data\": {"
                    "\"config\": ["
                        "{"
                            "\"module\": \"autoUploadCloud\","
                            "\"key\": \"filter\","
                            "\"value\": %d"
                        "},"
                        "{"
                            "\"module\": \"autoUploadCloud\","
                            "\"key\": \"autoDel\","
                            "\"value\": %d"
                        "},"
                        "{"
                            "\"module\": \"autoUploadCloud\","
                            "\"key\": \"writeLoc\","
                            "\"value\": %d"
                        "}"
                    "]"
                "}"
            "}";

            res = calloc(sizeof(char), strlen(my_cfg) + sizeof(DeviceState) + 1);
            if (res) {
                sprintf(res, my_cfg,
                    device_state.auc_filter,
                    device_state.auc_autoDel,
                    device_state.auc_writeLoc);
            }
        }
        else if (strcasecmp (module->valuestring, "manualUploadCloud") == 0) {
            static const char* my_cfg = "{"
                "\"retCode\": 0,"
                "\"data\": {"
                    "\"config\": ["
                        "{"
                            "\"module\": \"manualUploadCloud\","
                            "\"key\": \"filter\","
                            "\"value\": %d"
                        "},"
                        "{"
                            "\"module\": \"manualUploadCloud\","
                            "\"key\": \"autoDel\","
                            "\"value\": %d"
                        "},"
                        "{"
                            "\"module\": \"manualUploadCloud\","
                            "\"key\": \"writeLoc\","
                            "\"value\": %d"
                        "}"
                    "]"
                "}"
            "}";

            res = calloc(sizeof(char), strlen(my_cfg) + sizeof(DeviceState) + 1);
            if (res) {
                sprintf(res, my_cfg,
                    device_state.muc_filter,
                    device_state.muc_autoDel,
                    device_state.muc_writeLoc);
            }
        }
        else if (strcasecmp (module->valuestring, "ftp") == 0) {
            static const char* my_cfg = "{"
                "\"retCode\": 0,"
                "\"data\": {"
                    "\"config\": ["
                        "{"
                            "\"module\": \"ftp\","
                            "\"key\": \"host\","
                            "\"value\": \"%s\""
                        "},"
                        "{"
                            "\"module\": \"ftp\","
                            "\"key\": \"port\","
                            "\"value\": \"%s\""
                        "},"
                        "{"
                            "\"module\": \"ftp\","
                            "\"key\": \"user\","
                            "\"value\": \"%s\""
                        "},"
                        "{"
                            "\"module\": \"ftp\","
                            "\"key\": \"password\","
                            "\"value\": \"%s\""
                        "}"
                    "]"
                "}"
            "}";

            res = calloc(sizeof(char), strlen(my_cfg) + sizeof(DeviceState) + 1);
            if (res) {
                sprintf(res, my_cfg,
                    device_state.ftpHost,
                    device_state.ftpPort,
                    device_state.ftpUser,
                    device_state.ftpPassword);
            }
        }
    }
    else if (strcmp(method, "setConfig") == 0) {
        static const char* my_res = "{"
            "\"retCode\": %d"
        "}";

        int ret_code = RC_BAD_PARAM;

        cJSON* module = cJSON_GetObjectItem (msg, "module");
        cJSON* key = cJSON_GetObjectItem (msg, "key");
        cJSON* value = cJSON_GetObjectItem (msg, "value");
        if (module == NULL || module->type != cJSON_String) {
            goto error;
        }

        if (strcasecmp(module->valuestring, "os") == 0) {
            if (strcmp(key->valuestring, "timeZone") == 0
                    && value->type == cJSON_String) {
                ret_code = set_device_time_zone(value->valuestring);
            }
            else {
                _ERR_PRINTF("%s: Bad key or not support for %s: %s\n",
                    __FUNCTION__, module->valuestring, key->valuestring);
            }
        }
        else if (strcasecmp(module->valuestring, "user") == 0) {
            if (strcmp(key->valuestring, "transferMode") == 0
                    && value->type == cJSON_String) {
                ret_code = set_device_transfer_mode(atoi(value->valuestring));
            }
            else {
                _ERR_PRINTF("%s: Bad key or not support for %s: %s\n",
                    __FUNCTION__, module->valuestring, key->valuestring);
            }
        }
        else if (strcasecmp(module->valuestring, "autoUploadCloud") == 0) {
            if (strcmp(key->valuestring, "filter") == 0
                    && value->type != cJSON_String) {
                ret_code = set_device_auc_filter(value->valueint);
            }
            else if (strcmp(key->valuestring, "autoDel") == 0
                    && value->type != cJSON_String) {
                ret_code = set_device_auc_autodel(value->valueint);
            }
            else if (strcmp(key->valuestring, "writeLoc") == 0
                    && value->type != cJSON_String) {
                ret_code = set_device_auc_writeloc(value->valueint);
            }
            else {
                _ERR_PRINTF("%s: Bad key or not support for %s: %s\n",
                    __FUNCTION__, module->valuestring, key->valuestring);
            }
        }
        else if (strcasecmp(module->valuestring, "manualUploadCloud") == 0) {
            if (strcmp(key->valuestring, "filter") == 0
                    && value->type != cJSON_String) {
                ret_code = set_device_muc_filter(value->valueint);
            }
            else if (strcmp(key->valuestring, "autoDel") == 0
                    && value->type != cJSON_String) {
                ret_code = set_device_muc_autodel(value->valueint);
            }
            else if (strcmp(key->valuestring, "writeLoc") == 0
                    && value->type != cJSON_String) {
                ret_code = set_device_muc_writeloc(value->valueint);
            }
            else {
                _ERR_PRINTF("%s: Bad key or not support for %s: %s\n",
                    __FUNCTION__, module->valuestring, key->valuestring);
            }
        }
        else {
            _ERR_PRINTF("%s: Bad module or not support for %s: %s\n",
                __FUNCTION__, method, module->valuestring);
        }

        res = calloc(sizeof(char), strlen(my_res) + 64);
        if (res) {
            sprintf(res, my_res, ret_code);
        }
    }
    else if (strcmp(method, "clear") == 0) {
        static const char* my_res = "{"
            "\"retCode\": 0"
        "}";

        res = strdup(my_res);
    }
    else if (strcmp(method, "setFtpConfig") == 0) {
        static const char* my_res = "{"
            "\"retCode\": %d"
        "}";

        int ret_code = RC_BAD_PARAM;

        cJSON* host = cJSON_GetObjectItem (msg, "host");
        cJSON* port = cJSON_GetObjectItem (msg, "port");
        cJSON* user = cJSON_GetObjectItem (msg, "user");
        cJSON* password = cJSON_GetObjectItem (msg, "password");

        if (host && port && user && password) {
            ret_code = set_ftp_config(host->valuestring, port->valuestring,
                        user->valuestring, password->valuestring);
        }

        res = calloc(sizeof(char), strlen(my_res) + 64);
        if (res) {
            sprintf(res, my_res, ret_code);
        }
    }
    else {
        _ERR_PRINTF("%s: Bad method: %s\n", __FUNCTION__, method);
    }

error:
    return res;
}

static char* on_firmware_method (const char* method, cJSON* msg)
{
    if (strcmp(method, "check") == 0) {
    }
    else if (strcmp(method, "download") == 0) {
    }
    else if (strcmp(method, "upgrade") == 0) {
    }
    else {
        _ERR_PRINTF("%s: Bad method: %s\n", __FUNCTION__, method);
    }

    return NULL;
}

static char* on_network_method (const char* method, cJSON* msg)
{
    char* res = NULL;

    if (strcmp(method, "getState") == 0) {
        static const char* my_state = "{"
            "\"retCode\": 0,"
            "\"data\": {"
                "\"currentSSID\": %s,"
                "\"globalState\": %d,"
                "\"networkInfo\": ["
                    "{"
                        "\"type\": 0,"
                        "\"state\": %d"
                    "},"
                    "{"
                        "\"type\": 1,"
                        "\"state\": %d"
                    "},"
                    "{"
                        "\"type\": 2,"
                        "\"state\": 6"
                    "},"
                    "{"
                        "\"type\": 3,"
                        "\"state\": 7"
                    "}"
                "]"
            "}"
        "}";

        int globalState = NETWORK_STATE_DISCONNECTED;
        if (device_state.lteState == NETWORK_STATE_CONNECTED ||
                device_state.wifiState == NETWORK_STATE_CONNECTED) {
            globalState = NETWORK_STATE_CONNECTED;
        }

        res = calloc(sizeof(char), strlen(my_state) + sizeof(DeviceState));
        if (res) {
            if (device_state.wifiState == NETWORK_STATE_CONNECTED) {
                char buff[LEN_WIFI_SSID + 5];
                memset(buff, 0, sizeof(buff));
                sprintf(buff, "\"%s\"", device_state.wifiSSID);
                sprintf(res, my_state, buff,
                    globalState, device_state.lteState, device_state.wifiState);
            }
            else {
                sprintf(res, my_state, "null",
                    globalState, device_state.lteState, device_state.wifiState);
            }
        }
    }
    else if (strcmp(method, "getApState") == 0) {
        static const char* my_state = "{"
            "\"retCode\": 0,"
            "\"data\": {"
                "\"state\": %d,"
                "\"ssid\": \"%s\","
                "\"password\": \"%s\""
            "}"
        "}";

        res = calloc(sizeof(char), strlen(my_state) + sizeof(DeviceState));
        if (res) {
            sprintf(res, my_state,
                    device_state.wifiAp,
                    device_state.wifiApSSID, device_state.wifiApPass);
        }
    }
    else if (strcmp(method, "scanWiFi") == 0) {
        static const char* my_result = "{"
            "\"retCode\": 0,"
            "\"data\": {"
                "\"currentSSID\": %s,"
                "\"scanResult\": ["
                    "{"
                        "\"ssid\": \"示例假热点，下同\","
                        "\"bssid\": \"f0:b4:29:24:18:03\","
                        "\"level\": \"-70\","
                        "\"capabilities\": \"[WPA-PSK-CCMP+TKIP][WPA2-PSK-CCMP+TKIP][WPS][ESS]\","
                        "\"frequency\": \"2442\""
                    "},"
                    "{"
                        "\"ssid\": \"Joobot\","
                        "\"bssid\": \"f0:b4:29:24:18:eb\","
                        "\"level\": \"-38\","
                        "\"capabilities\": \"[WPA-PSK-CCMP+TKIP][WPA2-PSK-CCMP+TKIP][WPS][ESS]\","
                        "\"frequency\": \"2457\""
                    "},"
                    "{"
                        "\"ssid\": \"FakeWiFi2\","
                        "\"bssid\": \"f0:b4:29:24:18:01\","
                        "\"level\": \"-38\","
                        "\"capabilities\": \"[WPA-PSK-CCMP+TKIP][WPA2-PSK-CCMP+TKIP][WPS][ESS]\","
                        "\"frequency\": \"2457\""
                    "},"
                    "{"
                        "\"ssid\": \"fake2\","
                        "\"bssid\": \"f0:b4:29:24:18:02\","
                        "\"level\": \"-70\","
                        "\"capabilities\": \"[WPA-PSK-CCMP+TKIP][WPA2-PSK-CCMP+TKIP][WPS][ESS]\","
                        "\"frequency\": \"2442\""
                    "},"
                    "{"
                        "\"ssid\": \"Fake5G\","
                        "\"bssid\": \"f0:b4:29:24:18:04\","
                        "\"level\": \"-58\","
                        "\"capabilities\": \"[WPA-PSK-CCMP+TKIP][WPA2-PSK-CCMP+TKIP][WPS][ESS]\","
                        "\"frequency\": \"5745\""
                    "},"
                    "{"
                        "\"ssid\": \"fake555aaa\","
                        "\"bssid\": \"f0:b4:29:24:18:05\","
                        "\"level\": \"-100\","
                        "\"capabilities\": \"[WPA-PSK-CCMP+TKIP][WPA2-PSK-CCMP+TKIP][WPS][ESS]\","
                        "\"frequency\": \"2442\""
                    "},"
                    "{"
                        "\"ssid\": \"fake666bbb\","
                        "\"bssid\": \"f0:b4:29:24:18:06\","
                        "\"level\": \"-88\","
                        "\"capabilities\": \"[WPA-PSK-CCMP+TKIP][WPA2-PSK-CCMP+TKIP][WPS][ESS]\","
                        "\"frequency\": \"2442\""
                    "},"
                    "{"
                        "\"ssid\": \"fake777ccc\","
                        "\"bssid\": \"f0:b4:29:24:18:07\","
                        "\"level\": \"-98\","
                        "\"capabilities\": \"[WPA-PSK-CCMP+TKIP][WPA2-PSK-CCMP+TKIP][WPS][ESS]\","
                        "\"frequency\": \"2442\""
                    "},"
                    "{"
                        "\"ssid\": \"fake888ddd\","
                        "\"bssid\": \"f0:b4:29:24:18:08\","
                        "\"level\": \"-58\","
                        "\"capabilities\": \"[WPA-PSK-CCMP+TKIP][WPA2-PSK-CCMP+TKIP][WPS][ESS]\","
                        "\"frequency\": \"2442\""
                    "},"
                    "{"
                        "\"ssid\": \"OpenWiFi\","
                        "\"bssid\": \"f0:b4:29:24:18:09\","
                        "\"level\": \"-38\","
                        "\"capabilities\": \"[ESS]\","
                        "\"frequency\": \"2442\""
                    "}"
                "]"
            "}"
        "}";

        res = calloc(sizeof(char), strlen(my_result) + sizeof(DeviceState));
        if (res) {
            if (device_state.wifiState == NETWORK_STATE_CONNECTED) {
                char buff[LEN_WIFI_SSID + 5];
                memset(buff, 0, sizeof(buff));
                sprintf(buff, "\"%s\"", device_state.wifiSSID);
                sprintf(res, my_result, buff);
            }
            else {
                sprintf(res, my_result, "null");
            }

            sleep(3);
        }
    }
    else if (strcmp(method, "connectWiFi") == 0) {
    }
    else if (strcmp(method, "enableAp") == 0) {
    }
    else if (strcmp(method, "disableAp") == 0) {
    }
    else {
        _ERR_PRINTF("%s: Bad method: %s\n", __FUNCTION__, method);
    }

    return res;
}

static char* on_bind_method (const char* method, cJSON* msg)
{
    if (strcmp(method, "getBindState") == 0) {
    }
    else if (strcmp(method, "getBindCode") == 0) {
    }
    else if (strcmp(method, "getBindCodeState") == 0) {
    }
    else {
        _ERR_PRINTF("%s: Bad method: %s\n", __FUNCTION__, method);
    }

    return NULL;
}

static char* on_am_method (const char* method, cJSON* msg)
{
    if (strcmp(method, "updateStatus") == 0) {
    }
    else if (strcmp(method, "cbplus.app.auto_upload_cloud") == 0) {
    }
    else if (strcmp(method, "cbplus.app.manual_upload_cloud") == 0) {
    }
    else {
        _ERR_PRINTF("%s: Bad method: %s\n", __FUNCTION__, method);
    }

    return NULL;
}

static char* on_muc_method (const char* method, cJSON* msg)
{
    if (strcmp(method, "updateStatus") == 0) {
    }
    else if (strcmp(method, "startTransfer") == 0) {
    }
    else if (strcmp(method, "stopTransfer") == 0) {
    }
    else {
        _ERR_PRINTF("%s: Bad method: %s\n", __FUNCTION__, method);
    }

    return NULL;
}

char *jbus_invoke(const char *path, const char *method, const char *message)
{
    char* res = NULL;
    cJSON* msg = cJSON_Parse (message);
    if (msg == NULL) {
        _ERR_PRINTF("%s: Bad message: %s\n",
                __FUNCTION__, message);
        goto error;
    }

    if (strcmp (path, "cbplus.config") == 0) {
        res = on_config_method(method, msg);
    }
    else if (strcmp (path, "cbplus.firmware") == 0) {
        res = on_firmware_method(method, msg);
    }
    else if (strcmp (path, "cbplus.network") == 0) {
        res = on_network_method(method, msg);
    }
    else if (strcmp (path, "cbplus.bind") == 0) {
        res = on_bind_method(method, msg);
    }
    else if (strcmp (path, "cbplus.am") == 0) {
        res = on_am_method(method, msg);
    }
    else if (strcmp (path, "cbplus.app.muc") == 0) {
        res = on_muc_method(method, msg);
    }
    else {
        _ERR_PRINTF("%s: Got an unknown command path: %s\n",
                __FUNCTION__, path);
    }

    if (msg)
        cJSON_Delete(msg);

error:
    return res;
}

void jbus_free(void *buf)
{
    if (buf)
        free(buf);
}

static const char* events [] = {
    "cbplus.event.NETWORK_STATE_CHANGED",
    "cbplus.event.BATTERY_CHANGED",
    "cbplus.event.WIFI_STATE_CHANGED",
    "cbplus.event.WIFI_RSSI_CHANGED",
    "cbplus.event.CELLULAR_DATA_STATE_CHANGED",
    "cbplus.event.CELLULAR_SIGNAL_CHANGED",
    "cbplus.event.CAMERA_CONNECTED",
    "cbplus.event.CAMERA_DISCONNECTED",
    "cbplus.event.CLOUD_STATE",
    "cbplus.event.UPLOAD_STATE",
    "cbplus.event.ORDER_STATE",
    "cbplus.event.FIRMWARE_DOWNLOAD",
    "cbplus.event.FIRMWARE_UPGRADE",
};

static int get_event_id(const char* event)
{
    int i;
    for (i = 0; i < TABLESIZE(events); i++) {
        if (strcmp (event, events[i]) == 0)
            return i;
    }

    return -1;
}

typedef char* (*event_generator_t)(void);

typedef struct _event_handler {
    event_generator_t generator;
    jbus_event_handler_t handler;
} event_handler;

static event_handler handlers [TABLESIZE(events)];

int jbus_add_event_handler(const char *event, jbus_event_handler_t handler)
{
    int eid = get_event_id (event);
    if (eid < 0)
        return JBUS_PARAM_ERROR;

    handlers[eid].handler = handler;
    return JBUS_SUCCESS;
}

static char* event_generator_network_state_changed(void)
{
    return NULL;
}

static char* event_generator_battery_changed(void)
{
    static const char* format = "{\"status\":%d, \"scale\": %d, \"level\": %d}";
    char* message;

    message = calloc (sizeof(char), 256);
    if (message) {
        sprintf(message, format, time(NULL)%5 + 1, 100, time(NULL)%100);
    }

    return message;
}

static char* event_generator_wifi_state_changed(void)
{
    return NULL;
}

static char* event_generator_wifi_rssi_changed(void)
{
    return NULL;
}

static char* event_generator_cellular_data_state_changed(void)
{
    return NULL;
}

static char* event_generator_cellular_signal_changed(void)
{
    return NULL;
}

static char* event_generator_camera_connected(void)
{
    return NULL;
}

static char* event_generator_camera_disconnected(void)
{
    return NULL;
}

static char* event_generator_cloud_state_changed(void)
{
    return NULL;
}

static char* event_generator_upload_state_changed(void)
{
    return NULL;
}

static char* event_generator_order_state_changed(void)
{
    return NULL;
}

static char* event_generator_firmware_download(void)
{
    return NULL;
}

static char* event_generator_firmware_upgrade(void)
{
    return NULL;
}

int jbus_connect(void)
{
    init_device_state();

    handlers[0].generator = event_generator_network_state_changed;
    handlers[1].generator = event_generator_battery_changed;
    handlers[2].generator = event_generator_wifi_state_changed;
    handlers[3].generator = event_generator_wifi_rssi_changed;
    handlers[4].generator = event_generator_cellular_data_state_changed;
    handlers[5].generator = event_generator_cellular_signal_changed;
    handlers[6].generator = event_generator_camera_connected;
    handlers[7].generator = event_generator_camera_disconnected;
    handlers[8].generator = event_generator_cloud_state_changed;
    handlers[9].generator = event_generator_upload_state_changed;
    handlers[10].generator = event_generator_order_state_changed;
    handlers[11].generator = event_generator_firmware_download;
    handlers[12].generator = event_generator_firmware_upgrade;

    return JBUS_SUCCESS;
}

void jbus_run(void)
{
    while (1) {
        int eid;

        usleep (500000);

        eid = time(NULL) % TABLESIZE(events);
        if (handlers[eid].handler) {
            char* message = handlers[eid].generator();
            if (message) {
                handlers[eid].handler(events[eid], message);
                free (message);
            }
        }
    }
}

void jbus_clear(void)
{
}

/* not used
int jbus_send_event(const char *event, const char *message)
{
    return JBUS_SUCCESS;
}

char *jbus_result(char *message)
{
    return NULL;
}

*/

