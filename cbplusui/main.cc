/*!============================================================================
 * @file main.cc
 * @Synopsis
 * @author Vincent Wei
 * @version 1.0
 * @date 17/05/2018
 *
 * Copyright (C) 2018 Beijing Joobot Technologies Co., Ltd.
 */

#include <pthread.h>
#include <semaphore.h>
#include <jbus.h>

#include "global.h"
#include "utilities.h"

#include "Activity.hh"
#include "ActivityStack.hh"

#include "MyGetText.hh"
#include "cJSON.h"

#include "resource.h"

#include "misc/misc.h"
#include "apps/RuntimeConfService.hh"
#include "apps/SysConfService.hh"
#include "apps/NetworkService.hh"
#include "apps/MiscService.hh"
#include "apps/MenuListPiece.hh"

#define RES_SB_FORMAT_4G    "images/status-bar/4g-%d.png"
#define RES_SB_4G_0         "images/status-bar/4g-0.png"
#define RES_SB_4G_1         "images/status-bar/4g-1.png"
#define RES_SB_4G_2         "images/status-bar/4g-2.png"
#define RES_SB_4G_3         "images/status-bar/4g-3.png"
#define RES_SB_4G_4         "images/status-bar/4g-4.png"
#define RES_SB_4G_5         "images/status-bar/4g-5.png"

#define RES_SB_FORMAT_WIFI  "images/status-bar/wifi-%d.png"
#define RES_SB_WIFI_0       "images/status-bar/wifi-0.png"
#define RES_SB_WIFI_1       "images/status-bar/wifi-1.png"
#define RES_SB_WIFI_2       "images/status-bar/wifi-2.png"
#define RES_SB_WIFI_3       "images/status-bar/wifi-3.png"
#define RES_SB_WIFI_4       "images/status-bar/wifi-4.png"
#define RES_SB_WIFI_HOTSPOT "images/status-bar/wifi-hotspot.png"

#define RES_SB_FORMAT_GPS   "images/status-bar/gps-%d.png"
#define RES_SB_GPS_0        "images/status-bar/gps-0.png"
#define RES_SB_GPS_1        "images/status-bar/gps-1.png"
#define RES_SB_GPS_2        "images/status-bar/gps-2.png"
#define RES_SB_GPS_3        "images/status-bar/gps-3.png"

#define RES_SB_LOCK         "images/status-bar/lock.png"

#define RES_SB_FORMAT_BLEDEV    "images/status-bar/ble-dev-%s.png"
#define RES_SB_BLE_DEV_NONE     "images/status-bar/ble-dev-none.png"
#define RES_SB_BLE_DEV_PTZ      "images/status-bar/ble-dev-ptz.png"
#define RES_SB_BLE_DEV_LMETER   "images/status-bar/ble-dev-lmeter.png"
#define RES_SB_BLE_DEV_UNKNOWN  "images/status-bar/ble-dev-unknown.png"

#define RES_SB_FORMAT_USBDEV    "images/status-bar/usb-dev-%s.png"
#define RES_SB_USB_DEV_NONE     "images/status-bar/usb-dev-none.png"
#define RES_SB_USB_DEV_DISK     "images/status-bar/usb-dev-disk.png"
#define RES_SB_USB_DEV_CAM      "images/status-bar/usb-dev-camera.png"
#define RES_SB_USB_DEV_CAM_WARN "images/status-bar/usb-dev-camera-warn.png"
#define RES_SB_USB_DEV_UNKNOWN  "images/status-bar/usb-dev-unknown.png"

#define RES_SB_FORMAT_WIFIDEV       "images/status-bar/wifi-dev-%s.png"
#define RES_SB_WIFI_DEV_NONE        "images/status-bar/wifi-dev-none.png"
#define RES_SB_WIFI_DEV_CAM         "images/status-bar/wifi-dev-camera.png"
#define RES_SB_WIFI_DEV_CAM_WARN    "images/status-bar/wifi-dev-camera-warn.png"
#define RES_SB_WIFI_DEV_UNKNOWN     "images/status-bar/wifi-dev-unknown.png"

#define RES_SB_FORMAT_BAT   "images/status-bar/battery-%d.png"
#define RES_SB_BAT_0        "images/status-bar/battery-0.png"
#define RES_SB_BAT_1        "images/status-bar/battery-1.png"
#define RES_SB_BAT_2        "images/status-bar/battery-2.png"
#define RES_SB_BAT_3        "images/status-bar/battery-3.png"
#define RES_SB_BAT_4        "images/status-bar/battery-4.png"
#define RES_SB_BAT_5        "images/status-bar/battery-5.png"

typedef struct _STATUS_BAR_INFO {
    int level_4g;
    int level_wifi;
    int level_gps;
    int level_battery;
    char dev_ble [16];
    char dev_usb [16];
    char dev_wifi [16];
    char str_time [16];

    mHotPiece* piece_4g;
    mHotPiece* piece_wifi;
    mHotPiece* piece_gps;
    mHotPiece* piece_time;
    mHotPiece* piece_dev_ble;
    mHotPiece* piece_dev_wifi;
    mHotPiece* piece_dev_usb;
    mHotPiece* piece_battery;

    RECT rc_4g;
    RECT rc_wifi;
    RECT rc_gps;
    RECT rc_time;
    RECT rc_dev_ble;
    RECT rc_dev_wifi;
    RECT rc_dev_usb;
    RECT rc_battery;

    char* curr_ssid;
    char* camera_vendor;
    char* camera_model;

    int screen_lock_timeout;
    int screen_lock_enabled;

    float free_space;
    float total_space;

    int battery_capacity;
    int battery_status;
} STATUS_BAR_INFO;

static STATUS_BAR_INFO sbInfo;
static mWidget *widgetStatusBar;

#define SCREEN_IDLE_TIMER_ID    1

void statusBar_onScreenLockTimeoutChanged (int timeout)
{
    sbInfo.screen_lock_timeout = timeout * 100;
    ResetTimerEx (widgetStatusBar->hwnd, SCREEN_IDLE_TIMER_ID, sbInfo.screen_lock_timeout, NULL);
}

void statusBar_enableScreenLock (BOOL enable)
{
    sbInfo.screen_lock_enabled = enable;
}

int statusBar_getBatteryStatus (int* cap)
{
    if (cap)
        *cap = sbInfo.battery_capacity;
    return sbInfo.battery_status;
}

const char* statusBar_getDevInfo (int devId)
{
    switch (devId) {
    case SBINFO_DEV_CAMERA_VENDOR:
        return sbInfo.camera_vendor;
    case SBINFO_DEV_CAMERA_MODEL:
        return sbInfo.camera_model;
    case SBINFO_DEV_SSID:
        return sbInfo.curr_ssid;
    }

    return NULL;
}

float statusBar_getSpaceInfo (int spaceId)
{
    switch (spaceId) {
    case SBINFO_SPACE_FREE:
        return sbInfo.free_space;
    case SBINFO_SPACE_TOTAL:
        return sbInfo.total_space;
    }

    return 0.0f;
}

#define MSG_WIFI_STATE_CHANGED          (MSG_USER + 1)
    #define WIFI_DATA_UNKNOWN       -1
    #define WIFI_DATA_CONNECTED     0
    #define WIFI_DATA_CONNECTING    1
    #define WIFI_DATA_SUSPENDED     2
    #define WIFI_DATA_DISCONNECTED  4
    #define WIFI_DATA_CLOSED        6

#define MSG_RSSI_CHANGED                (MSG_USER + 2)

#define MSG_CELLULAR_STATE_CHANGED      (MSG_USER + 3)
    #define CELLULAR_DATA_UNKNOWN       -1

    /** Data connection state: Disconnected. IP traffic not available. */
    #define CELLULAR_DATA_DISCONNECTED  0

    /** Data connection state: Currently setting up a data connection. */
    #define CELLULAR_DATA_CONNECTING    1

    /** Data connection state: Connected. IP traffic should be available. */
    #define CELLULAR_DATA_CONNECTED     2

    /** Data connection state: Suspended. The connection is up, but IP
    * traffic is temporarily unavailable. For example, in a 2G network,
    * data activity may be suspended when a voice call arrives. */
    #define CELLULAR_DATA_SUSPENDED     3

#define MSG_CELLULAR_SIGNAL_CHANGED     (MSG_USER + 4)
    #define SIGNAL_STRENGTH_NONE_OR_UNKNOWN     0
    #define SIGNAL_STRENGTH_POOR                1
    #define SIGNAL_STRENGTH_MODERATE            2
    #define SIGNAL_STRENGTH_GOOD                3
    #define SIGNAL_STRENGTH_GREAT               4

#define MSG_CAMERA_CHANGED              (MSG_USER + 5)

static bool get_json_value_int (const cJSON* json, const char* field, int* value)
{
    cJSON* item = cJSON_GetObjectItem (json, field);
    if (item == NULL || item->type != cJSON_Number) {
        return false;
    }

    *value = item->valueint;
    return true;
}

static char* get_json_value_string (const cJSON* json, const char* field)
{
    cJSON* item = cJSON_GetObjectItem (json, field);
    if (item == NULL || item->type != cJSON_String) {
        return NULL;
    }

    return item->valuestring;
}

static void on_network_changed (const char *event, const char *message)
{
    NETWORKSERVICE->onStateChanged (message);
}

static int normalize_battery_level (int level, int min, int max)
{
    if (min == max) {
        max = min + 1;
    }

    if (level < min)
        level = min;
    else if (level > max)
        level = max;

    return (level - min) * max / (max - min);
}

static void on_battery_changed (const char *event, const char *message)
{
    cJSON* root = cJSON_Parse (message);
    if (root == NULL) {
        _ERR_PRINTF ("CBPlusGUI>on_battery_changed: Failed calling cJSON_Parse.\n");
        return;
    }

    int status, scale, level;
    if (!get_json_value_int (root, "status", &status)) {
        _ERR_PRINTF ("CBPlusGUI>on_battery_changed: no status field or not number.\n");
        goto error;
    }

    if (!get_json_value_int (root, "scale", &scale)) {
        _ERR_PRINTF ("CBPlusGUI>on_battery_changed: no scale field or not number.\n");
        goto error;
    }

    if (!get_json_value_int (root, "level", &level)) {
        _ERR_PRINTF ("CBPlusGUI>on_battery_changed: no level field or not number.\n");
        goto error;
    }

    level = normalize_battery_level (level, 0, scale);
    sbInfo.battery_capacity = level;
    sbInfo.battery_status = status;

    BroadcastMessage (MSG_BATTERY_CHANGED, level, status);

error:
    cJSON_Delete (root);
}

static LRESULT statusBar_onBatteryChanged (mWidget* self, LINT message, WPARAM wparam, LPARAM lparam)
{
    int level = (int)wparam;
    int status = (int)lparam;

    if (status == BATTERY_STATUS_CHARGING) {
        level = 5;
    }
    else {
        level = level / 20;
        if (level > 4) level = 4;
    }

    if (level != sbInfo.level_battery) {
        char res_name [128];

        sbInfo.level_battery = level;
        sprintf (res_name, RES_SB_FORMAT_BAT, sbInfo.level_battery);

        _c(sbInfo.piece_battery)->setProperty (sbInfo.piece_battery, NCSP_IMAGEPIECE_IMAGE, (DWORD)GetResource (Str2Key (res_name)));
        InvalidateRect (self->hwnd, &sbInfo.rc_battery, TRUE);
    }

    return 0;
}

static void on_wifi_state_changed (const char *event, const char *message)
{
    int state;
    char* ssid;
    Activity* act_top = ACTIVITYSTACK->top();

    cJSON* root = cJSON_Parse (message);
    if (root == NULL) {
        _ERR_PRINTF ("CBPlusGUI>on_wifi_state_changed: Failed calling cJSON_Parse.\n");
        return;
    }

    if (!get_json_value_int (root, "state", &state)) {
        _ERR_PRINTF ("CBPlusGUI>on_wifi_state_changed: no state field or not number.\n");
        goto error;
    }

    ssid = get_json_value_string (root, "ssid");
    NETWORKSERVICE->onWiFiStateChanged (state, ssid);

    SendNotifyMessage (widgetStatusBar->hwnd, MSG_WIFI_STATE_CHANGED, state, 0);

    if (act_top) {
        SendNotifyMessage (act_top->hwnd(), MSG_WIFI_CHANGED, state, 0);
    }

error:
    cJSON_Delete (root);
}

static LRESULT statusBar_onWiFiStateChanged (mWidget* self, LINT message, WPARAM wparam, LPARAM lparam)
{
    int state = (int)wparam;
    int level = sbInfo.level_wifi;

    _DBG_PRINTF ("CBPlusGUI>statusBar_onWiFiStateChanged: %d.\n", state);
    if (state == WIFI_DATA_CLOSED) {
        HOTSPOT_INFO hotspot_info;
        if (MISCSERVICE->getHotspotStatus(&hotspot_info) && hotspot_info.enabled) {
            _c(sbInfo.piece_wifi)->setProperty (sbInfo.piece_wifi, NCSP_IMAGEPIECE_IMAGE,
                    (DWORD)GetResource (Str2Key (RES_SB_WIFI_HOTSPOT)));
            InvalidateRect (self->hwnd, &sbInfo.rc_wifi, TRUE);
            return 0;
        }
    }

    if (state != WIFI_DATA_CONNECTED) {
        level = 0;
    }

    if (level != sbInfo.level_wifi) {
        char res_name [128];

        sbInfo.level_wifi = level;
        sprintf (res_name, RES_SB_FORMAT_WIFI, sbInfo.level_wifi);

        _c(sbInfo.piece_wifi)->setProperty (sbInfo.piece_wifi, NCSP_IMAGEPIECE_IMAGE, (DWORD)GetResource (Str2Key (res_name)));
        InvalidateRect (self->hwnd, &sbInfo.rc_wifi, TRUE);
    }

    return 0;
}

#define MIN_RSSI -100
#define MAX_RSSI -55

static int signal_rssi_to_level (int rssi)
{
    if (rssi <= MIN_RSSI)
        return 0;
    else if (rssi >= MAX_RSSI)
        return 100;

    return (int)((rssi - MIN_RSSI) * 100.0 / (MAX_RSSI - MIN_RSSI));
}

static void on_wifi_rssi_changed (const char *event, const char *message)
{
    cJSON* root = cJSON_Parse (message);
    if (root == NULL) {
        _ERR_PRINTF ("CBPlusGUI>on_wifi_rssi_changed: Failed calling cJSON_Parse.\n");
        return;
    }

    int rssi, level;
    if (!get_json_value_int (root, "rssi", &rssi)) {
        _ERR_PRINTF ("CBPlusGUI>on_wifi_rssi_changed: no rssi field or not number.\n");
        goto error;
    }

    level = signal_rssi_to_level (rssi);
    SendNotifyMessage (widgetStatusBar->hwnd, MSG_RSSI_CHANGED, level, 0);

error:
    cJSON_Delete (root);
}

static LRESULT statusBar_onRSSIChanged (mWidget* self, LINT message, WPARAM wparam, LPARAM lparam)
{
    int level = (int)wparam;

    level = level / 25;
    if (level > 3) level = 3;
    level += 1;

    if (level != sbInfo.level_wifi) {
        char res_name [128];

        sbInfo.level_wifi = level;
        sprintf (res_name, RES_SB_FORMAT_WIFI, sbInfo.level_wifi);

        _c(sbInfo.piece_wifi)->setProperty (sbInfo.piece_wifi, NCSP_IMAGEPIECE_IMAGE, (DWORD)GetResource (Str2Key (res_name)));
        InvalidateRect (self->hwnd, &sbInfo.rc_wifi, TRUE);
    }

    return 0;
}

static void on_cellular_state_changed (const char *event, const char *message)
{
    cJSON* root = cJSON_Parse (message);
    if (root == NULL) {
        _ERR_PRINTF ("CBPlusGUI>on_cellular_state_changed: Failed calling cJSON_Parse.\n");
        return;
    }

    int state;
    if (!get_json_value_int (root, "state", &state)) {
        _ERR_PRINTF ("CBPlusGUI>on_cellular_state_changed: no state field or not number.\n");
        goto error;
    }

    SendNotifyMessage (widgetStatusBar->hwnd, MSG_CELLULAR_STATE_CHANGED, state, 0);

error:
    cJSON_Delete (root);
}

static LRESULT statusBar_onCellularStateChanged (mWidget* self, LINT message, WPARAM wparam, LPARAM lparam)
{
    int state = (int)wparam;
    int level = sbInfo.level_4g;

    if (state != CELLULAR_DATA_CONNECTED) {
        level = 0;
    }

    if (level != sbInfo.level_4g) {
        char res_name [128];

        sbInfo.level_4g = level;
        sprintf (res_name, RES_SB_FORMAT_4G, sbInfo.level_4g);

        _c(sbInfo.piece_4g)->setProperty (sbInfo.piece_4g, NCSP_IMAGEPIECE_IMAGE, (DWORD)GetResource (Str2Key (res_name)));
        InvalidateRect (self->hwnd, &sbInfo.rc_4g, TRUE);
    }

    return 0;
}

static void on_cellular_signal_changed (const char *event, const char *message)
{
    cJSON* root = cJSON_Parse (message);
    if (root == NULL) {
        _ERR_PRINTF ("CBPlusGUI>on_cellular_signal_changed: Failed calling cJSON_Parse.\n");
        return;
    }

    int level;
    if (!get_json_value_int (root, "level", &level)) {
        _ERR_PRINTF ("CBPlusGUI>on_cellular_signal_changed: no level field or not number.\n");
        goto error;
    }

    SendNotifyMessage (widgetStatusBar->hwnd, MSG_CELLULAR_SIGNAL_CHANGED, level, 0);

error:
    cJSON_Delete (root);
}

static LRESULT statusBar_onCellularSignalChanged (mWidget* self, LINT message, WPARAM wparam, LPARAM lparam)
{
    int level = (int)(wparam/25) + 1;

    if (level != sbInfo.level_4g) {
        char res_name [128];

        sbInfo.level_4g = level;
        sprintf (res_name, RES_SB_FORMAT_4G, sbInfo.level_4g);

        _c(sbInfo.piece_4g)->setProperty (sbInfo.piece_4g, NCSP_IMAGEPIECE_IMAGE, (DWORD)GetResource (Str2Key (res_name)));
        InvalidateRect (self->hwnd, &sbInfo.rc_4g, TRUE);
    }

    return 0;
}

static void on_camera_connected (const char *event, const char *message)
{
    cJSON* root = cJSON_Parse (message);
    if (root == NULL) {
        _ERR_PRINTF ("CBPlusGUI>%s: Failed calling cJSON_Parse.\n",  __FUNCTION__);
        return;
    }

    char* vendor = get_json_value_string (root, "vendor");
    char* model = get_json_value_string (root, "model");
    if (vendor && model) {
        vendor = strdup (vendor);
        model = strdup (model);
        SendNotifyMessage (widgetStatusBar->hwnd, MSG_CAMERA_CHANGED, (WPARAM)vendor, (LPARAM)model);
    }

    cJSON_Delete (root);
}

static void on_camera_disconnected (const char *event, const char *message)
{
    SendNotifyMessage (widgetStatusBar->hwnd, MSG_CAMERA_CHANGED, 0, 0);
}

static LRESULT statusBar_onCameraChanged (mWidget* self, LINT message, WPARAM wparam, LPARAM lparam)
{
    char* vendor = (char*)wparam;
    char* model = (char*)lparam;

    if (sbInfo.camera_vendor)
        free (sbInfo.camera_vendor);
    if (sbInfo.camera_model)
        free (sbInfo.camera_model);

    char res_name [128];
    if (vendor == NULL && model == NULL) {
        /* disconnected */

        strcpy (sbInfo.dev_usb, "none");
        sbInfo.camera_vendor = NULL;
        sbInfo.camera_model = NULL;
    }
    else if (vendor && model) {
        strcpy (sbInfo.dev_usb, "camera");
        sbInfo.camera_vendor = vendor;
        sbInfo.camera_model = model;
    }
    else {
        strcpy (sbInfo.dev_usb, "unknown");
        sbInfo.camera_vendor = NULL;
        sbInfo.camera_model = NULL;

        if (vendor) free (vendor);
        if (model) free (model);
    }

    sprintf (res_name, RES_SB_FORMAT_USBDEV, sbInfo.dev_usb);
    _c(sbInfo.piece_dev_usb)->setProperty (sbInfo.piece_dev_usb, NCSP_IMAGEPIECE_IMAGE, (DWORD)GetResource (Str2Key (res_name)));
    InvalidateRect (self->hwnd, &sbInfo.rc_dev_usb, TRUE);

    Activity* act_top = ACTIVITYSTACK->top();
    if (act_top) {
        SendNotifyMessage (act_top->hwnd(), MSG_USBDEV_CHANGED, (WPARAM)sbInfo.camera_vendor, (LPARAM)sbInfo.camera_model);
    }
    return 0;
}

static void on_cloud_state_changed (const char *event, const char *message)
{
    cJSON* root = cJSON_Parse (message);
    if (root == NULL) {
        _ERR_PRINTF ("CBPlusGUI>%s: Failed calling cJSON_Parse.\n",  __FUNCTION__);
        return;
    }

    char* freeSpace;
    char* totalSpace;
    int uploadedNr;
    char* uploadedSize;
    int waitUploadNr;
    char* waitUploadSize;
    Activity* act_top = ACTIVITYSTACK->top();

    char* vendor = get_json_value_string (root, "vendor");
    char* model = get_json_value_string (root, "model");
    if (vendor && model && (sbInfo.camera_vendor == NULL || sbInfo.camera_model)) {
        vendor = strdup (vendor);
        model = strdup (model);
        SendNotifyMessage (widgetStatusBar->hwnd, MSG_CAMERA_CHANGED, (WPARAM)vendor, (LPARAM)model);
    }

    freeSpace = get_json_value_string (root, "freeSpace");
    totalSpace = get_json_value_string (root, "totalSpace");
    if (freeSpace == NULL || totalSpace == NULL) {
        _ERR_PRINTF ("CBPlusGUI>on_cloud_state_changed: no freeSpace/totalSpace field or not String.\n");
        goto error;
    }

    sbInfo.free_space = strtof (freeSpace, NULL);
    sbInfo.total_space = strtof (totalSpace, NULL);
    if (act_top) {
        /* NOTE: memcpy instead of casting */
        WPARAM wparam;
        LPARAM lparam;
        memcpy (&wparam, &sbInfo.free_space, sizeof (float));
        memcpy (&lparam, &sbInfo.total_space, sizeof (float));
        SendNotifyMessage (act_top->hwnd(), MSG_CLOUD_CHANGED, wparam, lparam);
    }

    if (!get_json_value_int (root, "waitUpload", &waitUploadNr)) {
        _ERR_PRINTF ("CBPlusGUI>on_cloud_state_changed: no waitUpload field or not Number.\n");
        goto error;
    }
    waitUploadSize = get_json_value_string (root, "waitUploadSize");
    if (waitUploadSize == NULL) {
        _ERR_PRINTF ("CBPlusGUI>on_cloud_state_changed: no waitUploadSize field or not String.\n");
        goto error;
    }
    if (act_top) {
        float f1 = strtof (waitUploadSize, NULL);

        LPARAM lparam;
        memcpy (&lparam, &f1, sizeof (float));
        _DBG_PRINTF ("CBPlusGUI>on_cloud_state_changed: send MSG_TOUPLOAD_CHANGED to top act.\n");
        SendNotifyMessage (act_top->hwnd(), MSG_TOUPLOAD_CHANGED, (WPARAM)waitUploadNr, lparam);
    }

    if (!get_json_value_int (root, "uploaded", &uploadedNr)) {
        _ERR_PRINTF ("CBPlusGUI>on_cloud_state_changed: no uploaded field or not Number.\n");
        goto error;
    }
    uploadedSize = get_json_value_string (root, "uploadedSize");
    if (uploadedSize == NULL) {
        _ERR_PRINTF ("CBPlusGUI>on_cloud_state_changed: no uploadedSize field or not String.\n");
        goto error;
    }
    if (act_top) {
        float f1 = strtof (uploadedSize, NULL);

        LPARAM lparam;
        memcpy (&lparam, &f1, sizeof(float));
        _DBG_PRINTF ("CBPlusGUI>on_cloud_state_changed: send MSG_TOUPLOAD_CHANGED to top act.\n");
        SendNotifyMessage (act_top->hwnd(), MSG_UPLOADED_CHANGED, (WPARAM)uploadedNr, lparam);
    }

error:
    cJSON_Delete (root);
}

static void on_upload_state_changed (const char *event, const char *message)
{
    cJSON* root = cJSON_Parse (message);
    if (root == NULL) {
        _ERR_PRINTF ("CBPlusGUI>%s: Failed calling cJSON_Parse.\n",  __FUNCTION__);
        return;
    }

    Activity* act_top = ACTIVITYSTACK->top();

    char* state = NULL;
    int status = UPLOAD_STATUS_UNKNOWN;
    char* name = NULL;
    state = get_json_value_string (root, "state");
    if (state) {
        if (strcasecmp(state, "idle") == 0) {
            status = UPLOAD_STATUS_IDLE;
        }
        else if (strcasecmp(state, "reading") == 0) {
            status = UPLOAD_STATUS_READING;
        }
        else if (strcasecmp(state, "uploading") == 0) {
            status = UPLOAD_STATUS_UPLOADING;
        }
        else if (strcasecmp(state, "failed") == 0) {
            status = UPLOAD_STATUS_FAILED;
        }

        name = get_json_value_string (root, "name");
        if (name && name[0])
            name = strdup (name);
        else
            name = NULL;
    }
    else {
        _ERR_PRINTF ("CBPlusGUI>on_upload_state_changed: no state defined.\n");
        goto error;
    }

    if (act_top) {
        SendNotifyMessage (act_top->hwnd(), MSG_UPLOAD_CHANGED, (WPARAM)status, (LPARAM)name);
    }

error:
    cJSON_Delete (root);
}

static void on_order_state_changed (const char *event, const char *message)
{
    cJSON* root = cJSON_Parse (message);
    if (root == NULL) {
        _ERR_PRINTF ("CBPlusGUI>%s: Failed calling cJSON_Parse.\n",  __FUNCTION__);
        return;
    }

    Activity* act_top = ACTIVITYSTACK->top();

    int status;
    char* order = NULL;
    if (!get_json_value_int (root, "status", &status)) {
        _ERR_PRINTF ("CBPlusGUI>on_order_state_changed: no status defined.\n");
        goto error;
    }

    if (status == ORDER_STATUS_OK) {
        order = get_json_value_string (root, "order");
        if (order)
            order = strdup (order);
    }

    if (act_top) {
        SendNotifyMessage (act_top->hwnd(), MSG_ORDER_CHANGED, (WPARAM)status, (LPARAM)order);
    }

error:
    cJSON_Delete (root);
}

static void in_downloading_firmware (const char *event, const char *message)
{
    cJSON* root = cJSON_Parse (message);
    if (root == NULL) {
        _ERR_PRINTF ("CBPlusGUI>%s: Failed calling cJSON_Parse.\n",  __FUNCTION__);
        return;
    }

    Activity* act_top = ACTIVITYSTACK->top();
    int progress = 0, state = 0;
    if (!get_json_value_int (root, "progress", &progress)) {
        _ERR_PRINTF ("CBPlusGUI>in_downloading_firmware: no progress field or not number.\n");
        goto error;
    }

    if (!get_json_value_int (root, "state", &state)) {
        _ERR_PRINTF ("CBPlusGUI>in_downloading_firmware: no state field or not number.\n");
        goto error;
    }

    if (act_top) {
        SendNotifyMessage (act_top->hwnd(), MSG_FIRMWARE_DOWNLOAD_STATE, (WPARAM)progress, (LPARAM)state);
    }

error:
    cJSON_Delete (root);
}

static void in_upgrading_firmware (const char *event, const char *message)
{
    cJSON* root = cJSON_Parse (message);
    if (root == NULL) {
        _ERR_PRINTF ("CBPlusGUI>%s: Failed calling cJSON_Parse.\n",  __FUNCTION__);
        return;
    }

    Activity* act_top = ACTIVITYSTACK->top();
    int progress = 0, state = 0;
    if (!get_json_value_int (root, "progress", &progress)) {
        _ERR_PRINTF ("CBPlusGUI>in_upgrading_firmware: no progress field or not number.\n");
        goto error;
    }

    if (!get_json_value_int (root, "state", &state)) {
        _ERR_PRINTF ("CBPlusGUI>in_upgrading_firmware: no state field or not number.\n");
        goto error;
    }

    if (act_top) {
        SendNotifyMessage (act_top->hwnd(), MSG_FIRMWARE_UPGRADE_STATE, (WPARAM)progress, (LPARAM)state);
    }

error:
    cJSON_Delete (root);
}

#define IDC_FIRST_BOOT_SETTINGS         100
#define IDC_UBUS_FAILED                 101

static void* event_listener (void* data)
{
    jbus_add_event_handler ("cbplus.event.NETWORK_STATE_CHANGED", on_network_changed);
    jbus_add_event_handler ("cbplus.event.BATTERY_CHANGED", on_battery_changed);
    jbus_add_event_handler ("cbplus.event.WIFI_STATE_CHANGED", on_wifi_state_changed);
    jbus_add_event_handler ("cbplus.event.WIFI_RSSI_CHANGED", on_wifi_rssi_changed);
    jbus_add_event_handler ("cbplus.event.CELLULAR_DATA_STATE_CHANGED", on_cellular_state_changed);
    jbus_add_event_handler ("cbplus.event.CELLULAR_SIGNAL_CHANGED", on_cellular_signal_changed);
    jbus_add_event_handler ("cbplus.event.CAMERA_CONNECTED", on_camera_connected);
    jbus_add_event_handler ("cbplus.event.CAMERA_DISCONNECTED", on_camera_disconnected);
    jbus_add_event_handler ("cbplus.event.CLOUD_STATE", on_cloud_state_changed);
    jbus_add_event_handler ("cbplus.event.UPLOAD_STATE", on_upload_state_changed);
    jbus_add_event_handler ("cbplus.event.ORDER_STATE", on_order_state_changed);
    jbus_add_event_handler ("cbplus.event.FIRMWARE_DOWNLOAD", in_downloading_firmware);
    jbus_add_event_handler ("cbplus.event.FIRMWARE_UPGRADE", in_upgrading_firmware);

    int ret = jbus_connect ();

    sem_t* wait = (sem_t*) data;
    sem_post (wait);

    if (ret) {
        _ERR_PRINTF ("CBPlusGUI>event_listener: Failed calling jbus_connect.\n");
        SendNotifyMessage (widgetStatusBar->hwnd, MSG_COMMAND, IDC_UBUS_FAILED, 0);
    }
    else {
        /* enter event loop */
        jbus_run ();
        jbus_clear ();
    }

    return NULL;
}

static pthread_t start_event_listener (void)
{
    pthread_t thread_event;
    sem_t wait;

    sem_init (&wait, 0, 0);
    pthread_create (&thread_event, NULL, event_listener, &wait);
    pthread_detach (thread_event);
    sem_wait (&wait);
    sem_destroy (&wait);

    return thread_event;
}

static HWND _hwnd_splash;
static void get_system_config (HWND hwnd)
{
    _hwnd_splash = hwnd;
    NETWORKSERVICE->getInstantState (GetHosting (hwnd));
}

static LRESULT statusBar_onAsyncTaskFinished (mMainWnd* self, UINT msg, DWORD wparam, DWORD lparam)
{
    switch (wparam) {
    case TASKID_NETWORK_STATE: {
        NETWORK_STATE* ns = (NETWORK_STATE*)lparam;
        NETWORKSERVICE->onStateChanged (ns);
        SendNotifyMessage (_hwnd_splash, MSG_CLOSE, 0, 0);
        break;
    }
    case TASKID_SYSCONF_GETALL: {
        _DBG_PRINTF ("CBPlusGUI>main: async task SYSCONF_GETALL finished: %x\n", lparam);
        break;
    }
    case TASKID_MISC_START_FUNC: {
        _DBG_PRINTF ("CBPlusGUI>main: async task MISC_START_FUNC finished: %x\n", lparam);
        break;
    }
    case TASKID_MISC_STOP_FUNC: {
        _DBG_PRINTF ("CBPlusGUI>main: async task MISC_STOP_FUNC finished: %x\n", lparam);
        break;
    }
    default:
        _DBG_PRINTF ("CBPlusGUI>main: Unhandled async task: %d\n", wparam);
        break;
    }

    return 0;
}

static BOOL statusBar_onCommand (mWidget* self, int id, int nc, HWND hCtrl)
{
    switch (id) {
        case IDC_FIRST_BOOT_SETTINGS: {
            Intent* my_intent = Intent::newIntent ();
            my_intent->putExtra ("firstBoot", std::string ("yes"));
            ACTIVITYSTACK->push ("SettingLanguageActivity", my_intent);
            break;
        }

        case IDC_UBUS_FAILED: {
            messageScreen (self->hwnd, _("System Error"), _("Failed to connect uBus. Please check the system configuration."), MB_OK);
            exit (0);
            break;
        }
    }

    return TRUE;
}

/* global resource */
static RES_NODE globalResList [] =
{
    /* global fonts */
    {GLOBAL_FONT_XS, RES_TYPE_FONT, 0, 0},
    {GLOBAL_FONT_SM, RES_TYPE_FONT, 0, 0},
    {GLOBAL_FONT_NM, RES_TYPE_FONT, 0, 0},
    {GLOBAL_FONT_LG, RES_TYPE_FONT, 0, 0},
    {GLOBAL_FONT_XL, RES_TYPE_FONT, 0, 0},
    {GLOBAL_FONT_XXL, RES_TYPE_FONT, 0, 0},

    /* global images */
    {GLOBAL_IMG_ARROW_LEFT,     RES_TYPE_IMAGE, 16, 0},
    {GLOBAL_IMG_ARROW_RIGHT,    RES_TYPE_IMAGE, 16, 0},
    {GLOBAL_IMG_ARROW_UP,       RES_TYPE_IMAGE, 16, 0},
    {GLOBAL_IMG_ARROW_DOWN,     RES_TYPE_IMAGE, 16, 0},
    {GLOBAL_IMG_MARK_CHECK,     RES_TYPE_IMAGE, 16, 0},

    /* terminator */
    {NULL}
};

static RES_NODE statusBarResList [] = {
    /* images on status bar */
    {RES_SB_4G_0,       RES_TYPE_IMAGE, 16, 0},
    {RES_SB_4G_1,       RES_TYPE_IMAGE, 16, 0},
    {RES_SB_4G_2,       RES_TYPE_IMAGE, 16, 0},
    {RES_SB_4G_3,       RES_TYPE_IMAGE, 16, 0},
    {RES_SB_4G_4,       RES_TYPE_IMAGE, 16, 0},
    {RES_SB_4G_5,       RES_TYPE_IMAGE, 16, 0},
    {RES_SB_WIFI_0,     RES_TYPE_IMAGE, 16, 0},
    {RES_SB_WIFI_1,     RES_TYPE_IMAGE, 16, 0},
    {RES_SB_WIFI_2,     RES_TYPE_IMAGE, 16, 0},
    {RES_SB_WIFI_3,     RES_TYPE_IMAGE, 16, 0},
    {RES_SB_WIFI_4,     RES_TYPE_IMAGE, 16, 0},
    {RES_SB_WIFI_HOTSPOT,       RES_TYPE_IMAGE, 16, 0},
    {RES_SB_GPS_0,      RES_TYPE_IMAGE, 16, 0},
    {RES_SB_GPS_1,      RES_TYPE_IMAGE, 16, 0},
    {RES_SB_GPS_2,      RES_TYPE_IMAGE, 16, 0},
    {RES_SB_GPS_3,      RES_TYPE_IMAGE, 16, 0},
    {RES_SB_LOCK,       RES_TYPE_IMAGE, 16, 0},
    {RES_SB_BLE_DEV_NONE,       RES_TYPE_IMAGE, 16, 0},
    {RES_SB_BLE_DEV_PTZ,        RES_TYPE_IMAGE, 16, 0},
    {RES_SB_BLE_DEV_LMETER,     RES_TYPE_IMAGE, 16, 0},
    {RES_SB_BLE_DEV_UNKNOWN,    RES_TYPE_IMAGE, 16, 0},
    {RES_SB_USB_DEV_NONE,       RES_TYPE_IMAGE, 16, 0},
    {RES_SB_USB_DEV_DISK,       RES_TYPE_IMAGE, 16, 0},
    {RES_SB_USB_DEV_CAM,        RES_TYPE_IMAGE, 16, 0},
    {RES_SB_USB_DEV_CAM_WARN,   RES_TYPE_IMAGE, 16, 0},
    {RES_SB_USB_DEV_UNKNOWN,    RES_TYPE_IMAGE, 16, 0},
    {RES_SB_WIFI_DEV_NONE,      RES_TYPE_IMAGE, 16, 0},
    {RES_SB_WIFI_DEV_CAM,       RES_TYPE_IMAGE, 16, 0},
    {RES_SB_WIFI_DEV_CAM_WARN,  RES_TYPE_IMAGE, 16, 0},
    {RES_SB_WIFI_DEV_UNKNOWN,       RES_TYPE_IMAGE, 16, 0},
    {RES_SB_BAT_0,    RES_TYPE_IMAGE, 16, 0},
    {RES_SB_BAT_1,    RES_TYPE_IMAGE, 16, 0},
    {RES_SB_BAT_2,    RES_TYPE_IMAGE, 16, 0},
    {RES_SB_BAT_3,    RES_TYPE_IMAGE, 16, 0},
    {RES_SB_BAT_4,    RES_TYPE_IMAGE, 16, 0},
    {RES_SB_BAT_5,    RES_TYPE_IMAGE, 16, 0},

    /* terminator */
    {NULL}
};

#define IDT_SB_TIME 100

static void format_current_time (char* buff, int size, const char* fmt)
{
    struct tm tm;
    time_t curr_time = time (NULL);

    localtime_r (&curr_time, &tm);
    strftime (buff, size, fmt, &tm);
}

static void init_sb_info (void)
{
    sbInfo.level_4g = 0;
    sbInfo.level_wifi = 0;
    sbInfo.level_gps = 0;
    sbInfo.level_battery = 3;

    sbInfo.camera_vendor = NULL;
    sbInfo.camera_model = NULL;

    strcpy (sbInfo.dev_ble, "none");
    strcpy (sbInfo.dev_usb, "none");
    strcpy (sbInfo.dev_wifi, "none");

    format_current_time (sbInfo.str_time, sizeof(sbInfo.str_time), "%H:%M");
}

static void term_sb_info (void)
{
    if (sbInfo.camera_vendor)
        free (sbInfo.camera_vendor);
    if (sbInfo.camera_model)
        free (sbInfo.camera_model);
}

static BOOL create_sb_pieces (mMainWnd* self, mContainerCtrl* ctrl_ctnr)
{
    char res_name [128];
    mPanelPiece* piece_panel = NULL;

    /* piece for 4g signal */
    SetRect (&sbInfo.rc_4g, 0, 0, 40, STATUSBAR_H);
    sprintf (res_name, RES_SB_FORMAT_4G, sbInfo.level_4g);

    sbInfo.piece_4g = (mHotPiece*)NEWPIECE (mImagePiece);
    _c(sbInfo.piece_4g)->setRect (sbInfo.piece_4g, &sbInfo.rc_4g);
    _c(sbInfo.piece_4g)->setProperty (sbInfo.piece_4g, NCSP_IMAGEPIECE_IMAGE, (DWORD)GetResource (Str2Key (res_name)));

    /* piece for WiFi signal */
    SetRect (&sbInfo.rc_wifi, 0, 0, 20, STATUSBAR_H);
    sprintf (res_name, RES_SB_FORMAT_WIFI, sbInfo.level_wifi);

    sbInfo.piece_wifi = (mHotPiece*)NEWPIECE (mImagePiece);
    _c(sbInfo.piece_wifi)->setRect (sbInfo.piece_wifi, &sbInfo.rc_wifi);
    _c(sbInfo.piece_wifi)->setProperty (sbInfo.piece_wifi, NCSP_IMAGEPIECE_IMAGE, (DWORD)GetResource (Str2Key (res_name)));

    /* piece for GPS signal */
    SetRect (&sbInfo.rc_gps, 0, 0, 20, STATUSBAR_H);
    sprintf (res_name, RES_SB_FORMAT_GPS, sbInfo.level_gps);

    sbInfo.piece_gps = (mHotPiece*)NEWPIECE (mImagePiece);
    _c(sbInfo.piece_gps)->setRect (sbInfo.piece_gps, &sbInfo.rc_gps);
    _c(sbInfo.piece_gps)->setProperty (sbInfo.piece_gps, NCSP_IMAGEPIECE_IMAGE, (DWORD)GetResource (Str2Key (res_name)));

    /* time piece */
    sbInfo.piece_time = (mHotPiece*)NEWPIECE (mLabelPiece);
    SetRect (&sbInfo.rc_time, 0, 0, 60, STATUSBAR_H);
    _c(sbInfo.piece_time)->setRect (sbInfo.piece_time, &sbInfo.rc_time);
    _c(sbInfo.piece_time)->setProperty (sbInfo.piece_time, NCSP_LABELPIECE_LABEL, (DWORD)sbInfo.str_time);
    _c(sbInfo.piece_time)->setProperty (sbInfo.piece_time, NCSP_LABELPIECE_ALIGN, (DWORD)NCS_ALIGN_CENTER);
    _c(sbInfo.piece_time)->setProperty (sbInfo.piece_time, NCSP_LABELPIECE_AUTOWRAP, 0);

    /* piece for BLE device */
    SetRect (&sbInfo.rc_dev_ble, 0, 0, 18, STATUSBAR_H);
    sprintf (res_name, RES_SB_FORMAT_BLEDEV, sbInfo.dev_ble);

    sbInfo.piece_dev_ble = (mHotPiece*)NEWPIECE (mImagePiece);
    _c(sbInfo.piece_dev_ble)->setRect (sbInfo.piece_dev_ble, &sbInfo.rc_dev_ble);
    _c(sbInfo.piece_dev_ble)->setProperty (sbInfo.piece_dev_ble, NCSP_IMAGEPIECE_IMAGE, (DWORD)GetResource (Str2Key (res_name)));

    /* piece for WiFi device */
    SetRect (&sbInfo.rc_dev_wifi, 0, 0, 18, STATUSBAR_H);
    sprintf (res_name, RES_SB_FORMAT_WIFIDEV, sbInfo.dev_wifi);

    sbInfo.piece_dev_wifi = (mHotPiece*)NEWPIECE (mImagePiece);
    _c(sbInfo.piece_dev_wifi)->setRect (sbInfo.piece_dev_wifi, &sbInfo.rc_dev_wifi);
    _c(sbInfo.piece_dev_wifi)->setProperty (sbInfo.piece_dev_wifi, NCSP_IMAGEPIECE_IMAGE, (DWORD)GetResource (Str2Key (res_name)));

    /* piece for USB device */
    SetRect (&sbInfo.rc_dev_usb, 0, 0, 24, STATUSBAR_H);
    sprintf (res_name, RES_SB_FORMAT_USBDEV, sbInfo.dev_usb);

    sbInfo.piece_dev_usb = (mHotPiece*)NEWPIECE (mImagePiece);
    _c(sbInfo.piece_dev_usb)->setRect (sbInfo.piece_dev_usb, &sbInfo.rc_dev_usb);
    _c(sbInfo.piece_dev_usb)->setProperty (sbInfo.piece_dev_usb, NCSP_IMAGEPIECE_IMAGE, (DWORD)GetResource (Str2Key (res_name)));

    /* piece for battery */
    SetRect (&sbInfo.rc_battery, 0, 0, 24, STATUSBAR_H);
    sprintf (res_name, RES_SB_FORMAT_BAT, sbInfo.level_battery);

    sbInfo.piece_battery = (mHotPiece*)NEWPIECE (mImagePiece);
    _c(sbInfo.piece_battery)->setRect (sbInfo.piece_battery, &sbInfo.rc_battery);
    _c(sbInfo.piece_battery)->setProperty (sbInfo.piece_battery, NCSP_IMAGEPIECE_IMAGE, (DWORD)GetResource (Str2Key (res_name)));

    RECT rc_panel;
    piece_panel = NEWPIECE (mPanelPiece);
    SetRect (&rc_panel, 0, 0, STATUSBAR_W, STATUSBAR_H);
    _c(piece_panel)->setRect (piece_panel, &rc_panel);

    int offset_x = 2;
    _c(piece_panel)->addContent (piece_panel, (mHotPiece*)sbInfo.piece_4g, offset_x, 0);
    OffsetRect (&sbInfo.rc_4g, offset_x, 0);

    offset_x = 2 + RECTW(sbInfo.rc_4g) + 2;
    _c(piece_panel)->addContent (piece_panel, (mHotPiece*)sbInfo.piece_wifi, offset_x, 0);
    OffsetRect (&sbInfo.rc_wifi, offset_x, 0);

    offset_x = 2 + RECTW(sbInfo.rc_4g) + 2 + RECTW(sbInfo.rc_wifi) + 2;
    _c(piece_panel)->addContent (piece_panel, (mHotPiece*)sbInfo.piece_gps, offset_x, 0);
    OffsetRect (&sbInfo.rc_gps, offset_x, 0);

    offset_x = (STATUSBAR_W - 60)/2;
    _c(piece_panel)->addContent (piece_panel, (mHotPiece*)sbInfo.piece_time, offset_x, 0);
    OffsetRect (&sbInfo.rc_time, offset_x, 0);

    offset_x = STATUSBAR_W - RECTW(sbInfo.rc_battery) - 2;
    _c(piece_panel)->addContent (piece_panel, (mHotPiece*)sbInfo.piece_battery, offset_x, 0);
    OffsetRect (&sbInfo.rc_battery, offset_x, 0);

    offset_x = STATUSBAR_W - RECTW(sbInfo.rc_battery) - 2 - RECTW(sbInfo.rc_dev_usb) - 2;
    _c(piece_panel)->addContent (piece_panel, (mHotPiece*)sbInfo.piece_dev_usb, offset_x, 0);
    OffsetRect (&sbInfo.rc_dev_usb, offset_x, 0);

    offset_x = STATUSBAR_W - RECTW(sbInfo.rc_battery) - 2 - RECTW(sbInfo.rc_dev_usb) - 2
                - RECTW(sbInfo.rc_dev_wifi) - 2;
    _c(piece_panel)->addContent (piece_panel, (mHotPiece*)sbInfo.piece_dev_wifi, offset_x, 0);
    OffsetRect (&sbInfo.rc_dev_wifi, offset_x, 0);

    offset_x = STATUSBAR_W - RECTW(sbInfo.rc_battery) - 2 - RECTW(sbInfo.rc_dev_usb) - 2
                - RECTW(sbInfo.rc_dev_wifi) - 2 - RECTW(sbInfo.rc_dev_ble) - 2;
    _c(piece_panel)->addContent (piece_panel, (mHotPiece*)sbInfo.piece_dev_ble, offset_x, 0);
    OffsetRect (&sbInfo.rc_dev_ble, offset_x, 0);

    _c(ctrl_ctnr)->setBody (ctrl_ctnr, (mHotPiece*)piece_panel);

    return TRUE;
}

static BOOL statusBar_onCreate (mMainWnd* self, DWORD dwAddData)
{
    unsigned cnt;
    mContainerCtrl* ctrl_ctnr;

    cnt = loadResByTag (statusBarResList, 0);

    if (cnt < (TABLESIZE (statusBarResList) - 1)) {
        _ERR_PRINTF ("CBPlusGUI>main: failed to load resource for status bar.\n");
        return FALSE;
    }

    ctrl_ctnr = (mContainerCtrl*)ncsCreateWindow (NCSCTRL_CONTAINERCTRL,
            "",
            WS_VISIBLE, WS_EX_NONE,
            0,
            0, 0, STATUSBAR_W, STATUSBAR_H - 1,
            self->hwnd,
            NULL, NULL, NULL, 0);

    init_sb_info ();
    create_sb_pieces (self, ctrl_ctnr);

    SetTimer (self->hwnd, IDT_SB_TIME, 100);

    return TRUE;
}

static void statusBar_onTimer (mWidget* self, LINT id, DWORD tick_count)
{
    if (id == IDT_SB_TIME) {
        char str_time [16];

        format_current_time (str_time, sizeof (str_time), "%H:%M");
        if (strcmp (str_time, sbInfo.str_time)) {
            strcpy (sbInfo.str_time, str_time);
            _c(sbInfo.piece_time)->setProperty (sbInfo.piece_time, NCSP_LABELPIECE_LABEL, (DWORD)sbInfo.str_time);
            InvalidateRect (self->hwnd, &sbInfo.rc_time, TRUE);
        }
    }
    else if (id == SCREEN_IDLE_TIMER_ID) {
        KillTimer (self->hwnd, id);
        if (sbInfo.screen_lock_enabled)
            screenLock (self->hwnd, FALSE);
        SetTimerEx (self->hwnd, SCREEN_IDLE_TIMER_ID, sbInfo.screen_lock_timeout, NULL);
    }
}

static void statusBar_onClose (mWidget* self, UINT message)
{
    term_sb_info ();

    KillTimer (self->hwnd, IDT_SB_TIME);

    while (TRUE) {
        if (ACTIVITYSTACK->depth () == 0) {
            break;
        }

        ACTIVITYSTACK->pop ();
    }
}

static void statusBar_onDestroy (mWidget* self, UINT message)
{
    unloadResByTag (statusBarResList, 0);
    PostQuitMessage (self->hwnd);
}

extern "C" int switch_back_light();

static BOOL statusBar_onKeyDown (mMainWnd* self, UINT message, int code, DWORD key_status)
{

    if (code != SCANCODE_POWER) {
        return FALSE;
    }

    switch_back_light();
    return TRUE;
}


static NCS_EVENT_HANDLER _statusbar_handlers [] = {
    {MSG_CREATE, reinterpret_cast<void*>(statusBar_onCreate)},
    {MSG_ASYNC_TASK_FINISHED, reinterpret_cast<void*>(statusBar_onAsyncTaskFinished)},
    {MSG_COMMAND, reinterpret_cast<void*>(statusBar_onCommand)},
    {MSG_BATTERY_CHANGED, reinterpret_cast<void*>(statusBar_onBatteryChanged)},
    {MSG_WIFI_STATE_CHANGED, reinterpret_cast<void*>(statusBar_onWiFiStateChanged)},
    {MSG_RSSI_CHANGED, reinterpret_cast<void*>(statusBar_onRSSIChanged)},
    {MSG_CELLULAR_STATE_CHANGED, reinterpret_cast<void*>(statusBar_onCellularStateChanged)},
    {MSG_CELLULAR_SIGNAL_CHANGED, reinterpret_cast<void*>(statusBar_onCellularSignalChanged)},
    {MSG_CAMERA_CHANGED, reinterpret_cast<void*>(statusBar_onCameraChanged)},
    {MSG_TIMER, reinterpret_cast<void*>(statusBar_onTimer)},
    {MSG_CLOSE, reinterpret_cast<void*>(statusBar_onClose)},
    {MSG_DESTROY, reinterpret_cast<void*>(statusBar_onDestroy)},
    {MSG_KEYUP, reinterpret_cast<void*>(statusBar_onKeyDown) },
    {0, NULL}
};

static HPACKAGE load_res_package (void)
{
    char res_path [MAX_PATH + 1];

    const char *etc_value = NULL;
    if ((etc_value = getenv (ENV_KEY_RES_PATH))) {
        int len = strlen (etc_value);
        if (etc_value [len - 1] == '/') {
            strcpy (res_path, etc_value);
        }
        else {
            strcpy (res_path, etc_value);
            strcat (res_path, "/");
        }
    }
    else {
        strcpy (res_path, "./res/");
    }

    SetResPath (res_path);
    strcat (res_path, "ui.res");
    return ncsLoadResPackage (res_path);

}

static NCS_WND_TEMPLATE _ctrl_templ[] =
{
    {
        class_name: NCSCTRL_SEPARATOR,
        id: IDC_STATIC,
        x: 0,
        y: STATUSBAR_H - 1,
        w: STATUSBAR_W,
        h: 1,
        style: WS_VISIBLE,
        ex_style: WS_EX_NONE,
        caption: "",
    },
};

static NCS_MNWND_TEMPLATE _statusbar_templ = {
    NCSCTRL_MAINWND,
    1,
    0, 0, SCREEN_W, STATUSBAR_H,
    WS_DISABLED, WS_EX_AUTOSECONDARYDC,
    "Status Bar",
    NULL,
    NULL,
    _statusbar_handlers,
    _ctrl_templ,
    TABLESIZE(_ctrl_templ),
    0, 0, NULL, 0, 0,
    NULL,
    0
};


int key_hook(void* context, HWND dst_wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    if (msg == MSG_KEYUP && wparam == SCANCODE_POWER && widgetStatusBar) {
        PostMessage(widgetStatusBar->hwnd, msg, wparam, lparam);
        return HOOK_STOP;
    }

    return HOOK_GOON;
}

int main (int argc, const char* argv[])
{
    HPACKAGE hPackage = HPACKAGE_NULL;
    MSG Msg;

    if (InitGUI (argc, argv) != 0) {
        _ERR_PRINTF ("CBPlusGUI>main: failed to call InitGUI.\n");
        return 1;
    }

    RegisterKeyMsgHook(NULL, key_hook);

#ifdef _MGRM_PROCESSES
    JoinLayer(NAME_DEF_LAYER , "CBPlus", 0, 0);
#endif

    /* init mGNCS and mGNCS4Touch */
    ncsInitialize ();
    ncs4TouchInitialize ();

    MGNCS_INIT_CLASS (mMenuListPiece);

    DO_REGISTER_ACTIVITY (LauncherActivity);
    DO_REGISTER_ACTIVITY (SettingMainActivity);
    DO_REGISTER_ACTIVITY (SettingLanguageActivity);
    DO_REGISTER_ACTIVITY (SettingCheckNetworkActivity);
    DO_REGISTER_ACTIVITY (SettingCheckNetworkPromptActivity);
    DO_REGISTER_ACTIVITY (SettingWifiActivity);
    DO_REGISTER_ACTIVITY (SettingWifiListActivity);
    DO_REGISTER_ACTIVITY (SettingWifiPasswordActivity);
    DO_REGISTER_ACTIVITY (SettingFunctionActivity);
    DO_REGISTER_ACTIVITY (SettingTimeActivity);
    DO_REGISTER_ACTIVITY (SettingTimeZoneActivity);
    DO_REGISTER_ACTIVITY (SettingScreenActivity);
    DO_REGISTER_ACTIVITY (SettingScreenLockTimeActivity);
    DO_REGISTER_ACTIVITY (SettingFirmwareActivity);
    DO_REGISTER_ACTIVITY (SettingPairActivity);
    DO_REGISTER_ACTIVITY (SettingAboutActivity);
    DO_REGISTER_ACTIVITY (SettingResetActivity);
    DO_REGISTER_ACTIVITY (SettingFtpServerActivity);
    DO_REGISTER_ACTIVITY (SettingCheckFtpActivity);
    DO_REGISTER_ACTIVITY (SettingTransferProtocolActivity);
    DO_REGISTER_ACTIVITY (SettingHotspotActivity);

    hPackage = load_res_package ();
    if (hPackage == HPACKAGE_NULL) {
        _ERR_PRINTF ("CBPlusGUI>main: failed to load resource package.\n");
        return 2;
    }

    /* load resource */
    unsigned cnt = loadResByTag (globalResList, 0);
    if (cnt < (TABLESIZE (globalResList) - 1)) {
        _ERR_PRINTF ("CBPlusGUI>main: failed to load global resource.\n");
        return 3;
    }

    if (!RTCFGSERVICE->load ()) {
        _ERR_PRINTF ("CBPlusGUI>main: failed when loading run-time conf.\n");
        return FALSE;
    }
    RTCFGSERVICE->getLanguageSettings ();
    RTCFGSERVICE->getTimeZone ();

    SetDefaultWindowElementRenderer ("flat");
    ncsSetSystemRenderer ("flat");

    WNDCLASS wnd_class;
    wnd_class.spClassName = NCSCTRL_DIALOGBOX;
    wnd_class.opMask = COP_BKCOLOR;
    wnd_class.iBkColor = PIXEL_black;
    SetWindowClassInfo (&wnd_class);

    wnd_class.spClassName = NCSCTRL_MAINWND;
    SetWindowClassInfo (&wnd_class);

    widgetStatusBar = ncsCreateMainWindowIndirect (&_statusbar_templ, HWND_DESKTOP);
    SET_GLOBAL_HOSTING (widgetStatusBar->hwnd);

    mGEffInit ();

    SYSCFGSERVICE->getAll (widgetStatusBar->hwnd);

    splashScreen (widgetStatusBar->hwnd, get_system_config);

    start_event_listener ();

    createToastWindow (widgetStatusBar->hwnd);

    ShowWindow (widgetStatusBar->hwnd, SW_SHOW);

    ACTIVITYSTACK->push ("LauncherActivity");

    sbInfo.screen_lock_timeout = RTCFGSERVICE->getScreenLockTimeout () * 100;
    sbInfo.screen_lock_enabled = TRUE;
    SetTimerEx (widgetStatusBar->hwnd, SCREEN_IDLE_TIMER_ID, sbInfo.screen_lock_timeout, NULL);
    while (GetMessage (&Msg, widgetStatusBar->hwnd)) {
        TranslateMessage (&Msg);

        if ((Msg.message >= MSG_FIRSTMOUSEMSG && Msg.message <= MSG_LASTMOUSEMSG) ||
                (Msg.message >= MSG_FIRSTKEYMSG && Msg.message <= MSG_LASTKEYMSG)) {
            ResetTimerEx (widgetStatusBar->hwnd, SCREEN_IDLE_TIMER_ID, sbInfo.screen_lock_timeout, NULL);
        }

        DispatchMessage (&Msg);
    }
    KillTimer (widgetStatusBar->hwnd, SCREEN_IDLE_TIMER_ID);
    DestroyMainWindow (widgetStatusBar->hwnd);

    mGEffDeinit ();

    releaseRefMemDC32b ();
    unloadResByTag (globalResList, 0);
    ncsUnloadResPackage (hPackage);

    ncs4TouchUninitialize ();
    ncsUninitialize ();

    TerminateGUI (0);
    return 0;
}

