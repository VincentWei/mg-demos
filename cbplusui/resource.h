/*!============================================================================
 * @file main.h 
 * @Synopsis header for resource definition 
 * @author DongKai
 * @version 1.0
 *  Company: Beijing FMSoft Technologies Co., Ltd.
 */

#ifndef  __RESOURCE_INC
#define  __RESOURCE_INC

#define SCREEN_W       240
#define SCREEN_H       240

#ifdef _FULL_SCREEN
    #define STATUSBAR_X    0
    #define STATUSBAR_Y    0
    #define STATUSBAR_W    SCREEN_W
    #define STATUSBAR_H    0
#else
    #define STATUSBAR_X    0
    #define STATUSBAR_Y    0
    #define STATUSBAR_W    SCREEN_W
    #define STATUSBAR_H    24
#endif

#define ACTIVITY_X     0
#define ACTIVITY_Y     STATUSBAR_H
#define ACTIVITY_W     SCREEN_W
#define ACTIVITY_H     (SCREEN_H - STATUSBAR_H)

#define ACTIVITY_TITLE_H            24
#ifdef _HAVE_SUBTITLE
    #define ACTIVITY_SUBTITLE_H     20
#else
    #define ACTIVITY_SUBTITLE_H     0
#endif

#define ACTIVITY_TITLE_X        0
#define ACTIVITY_TITLE_Y        0
#define ACTIVITY_TITLE_W        ACTIVITY_W

#define ACTIVITY_SUBTITLE_X     0
#define ACTIVITY_SUBTITLE_Y     ACTIVITY_TITLE_H
#define ACTIVITY_SUBTITLE_W     ACTIVITY_W

#define ACTIVITY_SEPARATOR_X        0
#define ACTIVITY_SEPARATOR_Y        (ACTIVITY_TITLE_H + ACTIVITY_SUBTITLE_H)
#define ACTIVITY_SEPARATOR_H        2
#define ACTIVITY_SEPARATOR_W        ACTIVITY_W

#define ACTIVITY_CONTENT_X          0
#define ACTIVITY_CONTENT_Y          (ACTIVITY_TITLE_H + ACTIVITY_SUBTITLE_H + ACTIVITY_SEPARATOR_H)
#define ACTIVITY_CONTENT_W          ACTIVITY_W
#define ACTIVITY_CONTENT_H          (ACTIVITY_H - ACTIVITY_TITLE_H - ACTIVITY_SUBTITLE_H - ACTIVITY_SEPARATOR_H)

#define ACTIVITY_LEFT_BUTTON_X      10
#define ACTIVITY_LEFT_BUTTON_H      30
#define ACTIVITY_LEFT_BUTTON_Y      (ACTIVITY_H - ACTIVITY_LEFT_BUTTON_H - 10)
#define ACTIVITY_LEFT_BUTTON_W      (ACTIVITY_W/2 - 20)

#define ACTIVITY_RIGHT_BUTTON_X     (ACTIVITY_W/2 + 10)
#define ACTIVITY_RIGHT_BUTTON_H     30
#define ACTIVITY_RIGHT_BUTTON_Y     (ACTIVITY_H - ACTIVITY_RIGHT_BUTTON_H - 10)
#define ACTIVITY_RIGHT_BUTTON_W     (ACTIVITY_W/2 - 20)

#define ACTIVITY_CONTENT_MARGIN_H   10
#define ACTIVITY_CONTENT_MARGIN_V   10

#define ACTIVITY_ITEM_WITH_BRIEF_H  50
#define ACTIVITY_ITEM_WITH_BRIEF_W  ACTIVITY_W

#define ACTIVITY_CONTENT_ITEM_H     40
#define ACTIVITY_CONTENT_ITEM_W     ACTIVITY_W

#define TOAST_X                     0
#define TOAST_Y                     (SCREEN_H - 80)
#define TOAST_W                     SCREEN_W
#define TOAST_H                     50

// Ids for common controls
#define IDC_TITLE           701
#define IDC_SUBTITLE        702
#define IDC_LOADING         703
#define IDC_CONTENT         704
#define IDC_LEFT_BUTTON     705
#define IDC_RIGHT_BUTTON    706

// environment keys
#define ENV_KEY_RES_PATH   "CBPLUS_UI_RES_PATH" 
#define ENV_KEY_CFG_PATH   "CBPLUS_UI_CFG_PATH" 
#define CFG_FILE_NAME      "cbplus-device-gui.cfg"

// resource name
// global fonts
#define GLOBAL_FONT_XS      "ttf-HanSans-srncnn-*-12-UTF-8"
#define GLOBAL_FONT_SM      "ttf-HanSans-srncnn-*-14-UTF-8"
#define GLOBAL_FONT_NM      "ttf-HanSans-srncnn-*-16-UTF-8"
#define GLOBAL_FONT_LG      "ttf-HanSans-srncnn-*-20-UTF-8"
#define GLOBAL_FONT_XL      "ttf-HanSans-srncnn-*-26-UTF-8"
#define GLOBAL_FONT_XXL     "ttf-HanSans-srncnn-*-36-UTF-8"

// fonts for title, subtitle, and toast
#define FONT_TITLE      GLOBAL_FONT_NM
#define FONT_SUBTITLE   GLOBAL_FONT_SM
#define FONT_TOAST      GLOBAL_FONT_SM

// global images
#define GLOBAL_IMG_ARROW_LEFT   "images/arrow-left.png"
#define GLOBAL_IMG_ARROW_RIGHT  "images/arrow-right.png"
#define GLOBAL_IMG_ARROW_UP     "images/arrow-up.png"
#define GLOBAL_IMG_ARROW_DOWN   "images/arrow-down.png"
#define GLOBAL_IMG_MARK_CHECK   "images/mark-check.png"
#define GLOBAL_IMG_OK           "images/icon-ok.png"
#define GLOBAL_IMG_CANCEL       "images/icon-cancel.png"

// common images
#define COMMON_IMG_CLOUD        "images/icon-cloud.png"
#define COMMON_IMG_FTP          "images/icon-ftp.png"
#define COMMON_IMG_CAMERA_USB   "images/icon-camera-usb.png"
#define COMMON_IMG_CAMERA_WLAN  "images/icon-camera-wlan.png"

#define RES_GLOBAL_LOADING_NM   "images/loading-nm.gif"
#define RES_GLOBAL_LOADING_SM   "images/loading-sm.gif"
#define RES_GLOBAL_LOADING_XM   "images/loading-xm.gif"

#define SIZE_LOADING_GIF_NM     120
#define SIZE_LOADING_GIF_SM     28
#define SIZE_LOADING_GIF_XM     16

// colors
#define COLOR_TITLE         0xFF6496C7L
#define COLOR_SUBTITLE      0xFFDDDDDDL
#define COLOR_TEXT_BUTTON   0xFF6496C7L
#define COLOR_SCROLL_ITEM   0xFF000000L
#define COLOR_SCROLLBAR     0xFF6496C7L

#define COLOR_STATE_NORMAL  0xFF16F235L
#define COLOR_STATE_ALARM   0xFF0C0CF5L

#define COLOR_TOAST_BK      0xFFE8E8E8L
#define COLOR_TOAST_DEF     0xFF000000L
#define COLOR_TOAST_INFO    0xFF333333L
#define COLOR_TOAST_WARNING 0xFF6496C7L
#define COLOR_TOAST_ERROR   0xFF0C0CF5L

// system-wide user-defined messages
#define MSG_USBDEV_CHANGED      (MSG_USER + 101)
#define MSG_WLANDEV_CHANGED     (MSG_USER + 102)
#define MSG_BLEDEV_CHANGED      (MSG_USER + 103)
#define MSG_WIFI_CHANGED        (MSG_USER + 104)
#define MSG_BATTERY_CHANGED     (MSG_USER + 105)
    #define BATTERY_STATUS_UNKNOWN      1
    #define BATTERY_STATUS_CHARGING     2
    #define BATTERY_STATUS_DISCHARGING  3
    #define BATTERY_STATUS_NOT_CHARGING 4
    #define BATTERY_STATUS_FULL         5

#define MSG_CLOUD_CHANGED       (MSG_USER + 211)

#define MSG_UPLOAD_CHANGED      (MSG_USER + 212)
    #define UPLOAD_STATUS_IDLE      0
    #define UPLOAD_STATUS_READING   1
    #define UPLOAD_STATUS_UPLOADING 2
    #define UPLOAD_STATUS_FAILED    3
    #define UPLOAD_STATUS_UNKNOWN   4

#define MSG_ORDER_CHANGED       (MSG_USER + 213)
    #define ORDER_STATUS_OK         0
    #define ORDER_STATUS_UNKNOWN    1
    #define ORDER_STATUS_NONE       2

#define MSG_TOUPLOAD_CHANGED    (MSG_USER + 214)
#define MSG_UPLOADED_CHANGED    (MSG_USER + 215)

#define MSG_FIRMWARE_DOWNLOAD_STATE     (MSG_USER + 251)
#define MSG_FIRMWARE_UPGRADE_STATE      (MSG_USER + 252)

#ifdef __cplusplus
extern "C" {
#endif

void statusBar_onScreenLockTimeoutChanged (int timeout);
void statusBar_enableScreenLock (BOOL enable);

int statusBar_getBatteryStatus (int* cap);

#define SBINFO_DEV_CAMERA_VENDOR    1
#define SBINFO_DEV_CAMERA_MODEL     2
#define SBINFO_DEV_SSID             3
const char* statusBar_getDevInfo (int devId);

#define SBINFO_SPACE_FREE           1
#define SBINFO_SPACE_TOTAL          2
float statusBar_getSpaceInfo (int spaceId);

#ifdef __cplusplus
}
#endif

#endif   /* ----- #ifndef __RESOURCE_INC  ----- */

