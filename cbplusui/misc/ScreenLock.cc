/*!============================================================================
 * @file ScreenLock.c 
 * @Synopsis Soft keyboard
 * @author Vincent Wei
 * @version 1.0
 *
 * Copyright (C) 2018 Beijing Joobot Technologies Inc.
 * All rights reserved.
 */

#ifndef _GNU_SOURCE
#   define _GNU_SOURCE
#endif

#include <crypt.h>

#include "global.h"
#include "utilities.h"
#include "md5.h"
#include "misc.h"

#include "resource.h"

#include "Activity.hh"
#include "ActivityStack.hh"
#include "RuntimeConfService.hh"

#define PAGE_SAVER          0
#define PAGE_KEYPAD         1

#define IDC_TIMER           100

#define IDC_PROPSHEET       100

#define IDC_TIME            101
#define IDC_BATTERY_CAP     102
#define IDC_BATTERY_LEVEL   103
#define IDC_BATTERY_STATUS  104

#define IDC_DIGIT_MIN       200
#define IDC_DIGIT_MAX       209
#define IDC_DELETE          210
#define IDC_PROMPT          211

#define INPUTBOX_H          30

#define KEY_W               60
#define KEY_H               42
#define SPACE               4
#define MARGIN_H            ((240 - KEY_W*3 - SPACE*2) / 2)
#define MARGIN_V            10

#define KEY_X               MARGIN_H
#define KEY_Y               (INPUTBOX_H + MARGIN_V*2)

#define RES_BTN_DELETE      "images/screen-lock/del.png"

#define KEY_TYPE_BTN        0
#define KEY_TYPE_SPACE      1

#define INV_ENC_PASSWD      "na"
#define CIRCLE_UTF8         "●"

#define RES_BMPFONT_DIGITS      "images/screen-lock/bmpfont-digits.png"
#define RES_BMPFONT_COLON       "images/screen-lock/bmpfont-colon.png"
#define RES_BMPFONT_SPACE       "images/screen-lock/bmpfont-space.png"
#define RES_BATTERY             "images/screen-lock/battery.png"
#define RES_CHARGING            "images/screen-lock/charging.png"

typedef struct _SL_INFO {
    BOOL reset_passwd;
    BOOL passed;
    char curr_enc_passwd [LEN_ENC_PASSWD + 1];

    int  nr_digits;
    char passwd_inputed [8];
    char new_passwd_1 [8];
    char new_passwd_2 [8];

    BOOL show_circles;

    const char* prompt;

    DEVFONT *bmp_devfont;
    LOGFONT *bmp_logfont; 
} SL_INFO;

typedef struct _KEY {
    int type;
    int id;
    const char* res_name;
} KEY;

static KEY _digit_keys [] = {
    {KEY_TYPE_BTN,    IDC_DIGIT_MIN + 1,  "images/screen-lock/1.png"},
    {KEY_TYPE_BTN,    IDC_DIGIT_MIN + 2,  "images/screen-lock/2.png"},
    {KEY_TYPE_BTN,    IDC_DIGIT_MIN + 3,  "images/screen-lock/3.png"},

    {KEY_TYPE_BTN,    IDC_DIGIT_MIN + 4,  "images/screen-lock/4.png"},
    {KEY_TYPE_BTN,    IDC_DIGIT_MIN + 5,  "images/screen-lock/5.png"},
    {KEY_TYPE_BTN,    IDC_DIGIT_MIN + 6,  "images/screen-lock/6.png"},

    {KEY_TYPE_BTN,    IDC_DIGIT_MIN + 7,  "images/screen-lock/7.png"},
    {KEY_TYPE_BTN,    IDC_DIGIT_MIN + 8,  "images/screen-lock/8.png"},
    {KEY_TYPE_BTN,    IDC_DIGIT_MIN + 9,  "images/screen-lock/9.png"},

    {KEY_TYPE_SPACE,  0,  NULL},
    {KEY_TYPE_BTN,    IDC_DIGIT_MIN + 0,  "images/screen-lock/0.png"},
    {KEY_TYPE_BTN,    IDC_DELETE,         RES_BTN_DELETE},
};

static void show_circles_with_last_digit (HWND hwnd, int nr_digits, char digit)
{
    if (nr_digits < 1 || nr_digits > 4) {
        _ERR_PRINTF ("ScreenLock: bad number of digits: %d\n", nr_digits);
        return;
    }

    char display_buff [32];
    int len_circle_wchar = strlen (CIRCLE_UTF8);
    char* tmp = display_buff;
    for (int i = 0; i < nr_digits - 1; i++) {
        strcpy (tmp, CIRCLE_UTF8);
        tmp += len_circle_wchar;
    }
    *tmp = digit;
    tmp++;
    *tmp = '\0';

    SetWindowText (hwnd, display_buff);
}

static void show_circles (HWND hwnd, int nr_digits)
{
    if (nr_digits > 4) {
        _ERR_PRINTF ("ScreenLock: bad number of digits: %d\n", nr_digits);
        return;
    }

    char display_buff [32];
    int len_circle_wchar = strlen (CIRCLE_UTF8);
    char* tmp = display_buff;
    for (int i = 0; i < nr_digits; i++) {
        strcpy (tmp, CIRCLE_UTF8);
        tmp += len_circle_wchar;
    }
    *tmp = '\0';

    SetWindowText (hwnd, display_buff);
}

static void reset_sl_info (SL_INFO* sl_info)
{
    memset (sl_info->new_passwd_1, 0, sizeof(sl_info->new_passwd_1));
    memset (sl_info->new_passwd_2, 0, sizeof(sl_info->new_passwd_2));
    memset (sl_info->passwd_inputed, 0, sizeof(sl_info->passwd_inputed));
    sl_info->nr_digits = 0;
}

static BOOL check_password (const SL_INFO* sl_info)
{
    if (strcmp (sl_info->curr_enc_passwd, "na") == 0) {
        return TRUE;
    }

    struct crypt_data data;
    data.initialized = 0;
    char* enc_passwd = crypt_r (sl_info->passwd_inputed, sl_info->curr_enc_passwd, &data);
    if (strcmp (sl_info->curr_enc_passwd, enc_passwd) == 0) {
        return TRUE;
    }

    return FALSE;
}

static char hex_digits [] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
};

static void save_password (const char* passwd)
{
    struct crypt_data data;
    char salt [32];
    unsigned char md5_bin [16];
    char md5_hex [32];
    char str_time [16];

    sprintf (str_time, "%ld", time (NULL));
    mymd5 ((unsigned char*)str_time, md5_bin);
    for (int i = 0; i < 16; i++) {
        unsigned char byte = md5_bin [i];
        md5_hex [i*2] = hex_digits [(byte >> 4) & 0x0f];
        md5_hex [i*2+1] = hex_digits [byte & 0x0f];
    }
    md5_hex [16] = '\0';

    sprintf (salt, "$1$%s", md5_hex);
    data.initialized = 0;
    char* enc_passwd = crypt_r (passwd, salt, &data);

    RTCFGSERVICE->setScreenLockPassword (enc_passwd);
    RTCFGSERVICE->save ();
}

static BOOL mypage_onCommand (mWidget* self, int id, int nc, HWND hCtrl)
{
    HWND mainwnd = GetMainWindowHandle (hCtrl);
    SL_INFO* sl_info = (SL_INFO*)GetWindowAdditionalData (mainwnd);

    if (nc == NCSN_WIDGET_CLICKED) {
        mPropSheet *propsheet = (mPropSheet*)ncsGetChildObj (mainwnd, IDC_PROPSHEET);
        mPage* page = (mPage*)(_c(propsheet)->getPageByIndex (propsheet, PAGE_KEYPAD));
        mStatic* ctrl_prompt = (mStatic*)ncsObjFromHandle (GetDlgItem (page->hwnd, IDC_PROMPT));

        if (id >= IDC_DIGIT_MIN && id <= IDC_DIGIT_MAX) {
            if (sl_info->nr_digits < 4) {
                char digit = '0' + (id - IDC_DIGIT_MIN);
                sl_info->passwd_inputed [sl_info->nr_digits] = digit;
                sl_info->nr_digits ++;

                show_circles_with_last_digit (ctrl_prompt->hwnd, sl_info->nr_digits, digit);
                sl_info->show_circles = TRUE;
            }
        }
        else if (id == IDC_DELETE) {
            if (sl_info->nr_digits > 0) {
                sl_info->nr_digits --;
                sl_info->passwd_inputed [sl_info->nr_digits] = '\0';
                show_circles (ctrl_prompt->hwnd, sl_info->nr_digits);
                sl_info->show_circles = FALSE;
            }

            if (sl_info->nr_digits == 0 && sl_info->prompt) {
                SetWindowText (ctrl_prompt->hwnd, sl_info->prompt);
                sl_info->show_circles = FALSE;
            }
        }
    }

    return TRUE;
}

static void format_current_time (char* buff, int size, const char* fmt, time_t curr_time = 0)
{
    struct tm tm;
    if (curr_time == 0) {
        curr_time = time (NULL);
    }

    localtime_r (&curr_time, &tm);
    strftime (buff, size, fmt, &tm);
}

static BOOL update_time (HWND hwnd, LINT id, DWORD tick_count)
{
    char my_time [16];
    const char* format;
    time_t curr_time = time (NULL);

    if (curr_time % 2 == 0) {
        format = "%H:%M";
    }
    else {
        format = "%H %M";
    }

    format_current_time (my_time, sizeof (my_time), format, curr_time);
    SetWindowText (hwnd, my_time);

    return TRUE;
}

static BOOL init_battery_widgets (HWND hwnd)
{
    int battery_cap, battery_status;
    char buff [16];

    battery_status = statusBar_getBatteryStatus (&battery_cap);
    sprintf (buff, "%d%%", battery_cap);

    /* battery capacity */
    NCS_PROP_ENTRY props_battery_cap [] = {
        {NCSP_STATIC_ALIGN, NCS_ALIGN_RIGHT},
        {-1, 0},
    };
    NCS_WND_TEMPLATE ctrl_battery_cap = {
        class_name: NCSCTRL_STATIC,
        id: IDC_BATTERY_CAP,
        x: 0,
        y: 30,
        w: 100,
        h: 30,
        style: WS_VISIBLE,
        ex_style: WS_EX_NONE,
        caption: buff,
    };

    ctrl_battery_cap.props = props_battery_cap;
    mWidget* ctrl = ncsCreateWindowIndirect (&ctrl_battery_cap, hwnd);
    if (ctrl) {
        ncsSetFont (ctrl->hwnd, GLOBAL_FONT_XL);
    }
    else {
        _ERR_PRINTF ("screenLock: Failed to create battery capacity widget\n");
    }

    /* battery icon */
    NCS_PROP_ENTRY props_battery_icon [] = {
        {NCSP_IMAGE_IMAGE, (DWORD)GetResource (Str2Key (RES_BATTERY))},
        {-1, 0},
    };
    NCS_WND_TEMPLATE ctrl_battery_icon = {
        class_name: NCSCTRL_IMAGE,
        id: IDC_STATIC,
        x: 110,
        y: 30,
        w: 54,
        h: 30,
        style: WS_VISIBLE,
        ex_style: WS_EX_NONE,
        caption: "",
    };

    ctrl_battery_icon.props = props_battery_icon;
    ctrl = ncsCreateWindowIndirect (&ctrl_battery_icon, hwnd);
    if (ctrl == NULL) {
        _ERR_PRINTF ("screenLock: Failed to create battery icon widget\n");
    }

    NCS_PROP_ENTRY props_battery_level [] = {
        {NCSP_PROG_MAXPOS, 100},
        {NCSP_PROG_MINPOS, 0},
        {NCSP_PROG_LINESTEP, 10},
        {NCSP_PROG_CURPOS, (DWORD)battery_cap},
        {-1, 0},
    };
    NCS_WND_TEMPLATE ctrl_battery_level = {
        class_name: NCSCTRL_PROGRESSBAR,
        id: IDC_BATTERY_LEVEL,
        x: 113,
        y: 36,
        w: 44,
        h: 18,
        style: WS_VISIBLE,
        ex_style: WS_EX_NONE,
        caption: "",
    };
    ctrl_battery_level.props = props_battery_level;
    ctrl = ncsCreateWindowIndirect (&ctrl_battery_level, hwnd);
    if (ctrl) {
        if (battery_cap < 20) {
            ncsSetElement (ctrl, NCS_FGC_HILIGHT_ITEM, COLOR_STATE_ALARM);
        }
        else {
            ncsSetElement (ctrl, NCS_FGC_HILIGHT_ITEM, COLOR_STATE_NORMAL);
        }
    }
    else {
        _ERR_PRINTF ("screenLock: Failed to create battery level widget\n");
    }
    
    /* charging icon */
    NCS_PROP_ENTRY props_charging_icon [] = {
        {NCSP_IMAGE_IMAGE, (DWORD)GetResource (Str2Key (RES_CHARGING))},
        {-1, 0},
    };
    NCS_WND_TEMPLATE ctrl_charging_icon = {
        class_name: NCSCTRL_IMAGE,
        id: IDC_BATTERY_STATUS,
        x: 170,
        y: 30,
        w: 12,
        h: 30,
        style: WS_NONE,
        ex_style: WS_EX_NONE,
        caption: "",
    };

    ctrl_charging_icon.style = (battery_status == BATTERY_STATUS_CHARGING) ? WS_VISIBLE : WS_NONE;
    ctrl_charging_icon.props = props_charging_icon;
    ctrl = ncsCreateWindowIndirect (&ctrl_charging_icon, hwnd);
    if (ctrl == NULL) {
        _ERR_PRINTF ("screenLock: Failed to create charging icon widget\n");
    }

    return TRUE;
}

static void mypage_onInitPage (mWidget* self, DWORD add_data)
{
    SL_INFO* sl_info = (SL_INFO*)GetWindowAdditionalData (GetMainWindowHandle (self->hwnd));

    if (add_data == PAGE_SAVER) {
        NCS_WND_TEMPLATE ctrl_time = {
            class_name: NCSCTRL_STATIC,
            id: IDC_TIME,
            x: 0,
            y: (SCREEN_H - 60)/2,
            w: SCREEN_W,
            h: 60,
            style: WS_VISIBLE,
            ex_style: WS_EX_NONE,
            caption: "00:00",
        };

        char my_time [16];
        format_current_time (my_time, sizeof(my_time), "%H:%M");
        ctrl_time.caption = my_time;

        mWidget* ctrl;
        ctrl = ncsCreateWindowIndirect (&ctrl_time, ((mWidget*)self)->hwnd);
        if (ctrl) {
            SetWindowFont (ctrl->hwnd, sl_info->bmp_logfont);
        }

        init_battery_widgets (((mWidget*)self)->hwnd);

        Activity* act_top = ACTIVITYSTACK->top();
        if (act_top) {
            NCS_PROP_ENTRY props_act_name [] = {
                {NCSP_STATIC_ALIGN, NCS_ALIGN_RIGHT},
                {-1, 0},
            };

            NCS_WND_TEMPLATE ctrl_act_name = {
                class_name: NCSCTRL_STATIC,
                id: IDC_STATIC,
                x: 10,
                y: SCREEN_H - 40,
                w: SCREEN_W - 20,
                h: 40,
                style: WS_VISIBLE,
                ex_style: WS_EX_NONE,
            };

            ctrl_act_name.props = props_act_name;
            ctrl_act_name.caption = _(GetWindowCaption (act_top->hwnd()));
            ncsCreateWindowIndirect (&ctrl_act_name, ((mWidget*)self)->hwnd);
        }

        SetTimerEx (ctrl->hwnd, IDC_TIMER, 100, update_time);
    }
    else if (add_data == PAGE_KEYPAD) {
        NCS_WND_TEMPLATE ctrl_inputbox = {
            class_name: NCSCTRL_STATIC,
            id: IDC_PROMPT,
            x: SPACE,
            y: MARGIN_V,
            w: SCREEN_W - SPACE * 2,
            h: INPUTBOX_H,
            style: WS_VISIBLE,
            ex_style: WS_EX_NONE,
            caption: "",
        };

        if (sl_info->reset_passwd && (strcmp (sl_info->curr_enc_passwd, INV_ENC_PASSWD) == 0)) {
            ctrl_inputbox.caption = _("Input new password");
            sl_info->passed = TRUE;
        }
        else {
            ctrl_inputbox.caption = _("Input password");
        }
        sl_info->prompt = ctrl_inputbox.caption;

        mWidget* ctrl;
        ctrl = ncsCreateWindowIndirect (&ctrl_inputbox, ((mWidget*)self)->hwnd);
        if (ctrl)
            SetWindowBkColor (ctrl->hwnd, RGB2Pixel(HDC_SCREEN, 0x33, 0x33, 0x33));

        unsigned u;

        NCS_PROP_ENTRY props [] = {
            { NCSP_IMAGE_IMAGE, 0 },
            { 0, 0 }
        };

        NCS_WND_TEMPLATE ctrl_templ = {};
        ctrl_templ.class_name = NCSCTRL_IMAGE;
        ctrl_templ.style = WS_VISIBLE | NCSS_NOTIFY;
        ctrl_templ.ex_style = WS_EX_NONE;
        ctrl_templ.props = props;
        ctrl_templ.w = KEY_W;
        ctrl_templ.h = KEY_H;
        ctrl_templ.caption = "";

        for (u = 0; u < TABLESIZE (_digit_keys); u++) {
            if (_digit_keys[u].type == KEY_TYPE_SPACE) {
                continue;
            }

            props [0].value = (DWORD)GetResource (Str2Key (_digit_keys[u].res_name));

            ctrl_templ.id = _digit_keys[u].id;
            ctrl_templ.x = KEY_X + (KEY_W + SPACE) * (u % 3);
            ctrl_templ.y = KEY_Y + (KEY_H + SPACE) * (u / 3);

            ncsCreateWindowIndirect (&ctrl_templ, ((mWidget*)self)->hwnd);
        }

    }

    return;
}

static int mypage_onShowPage (mWidget* self, HWND hwnd, int show_cmd)
{
    return 1;
}

static int mypage_onSheetCmd (mWidget* self, DWORD wParam, DWORD lParam)
{
    if (wParam == IDOK)
        return 1;
    return 0;
}

static NCS_EVENT_HANDLER mypage_handlers[] = {
    { MSG_INITPAGE, reinterpret_cast<void*>(mypage_onInitPage) },
    { MSG_SHOWPAGE, reinterpret_cast<void*>(mypage_onShowPage) },
    { MSG_COMMAND, reinterpret_cast<void*>(mypage_onCommand) },
    { MSG_SHEETCMD, reinterpret_cast<void*>(mypage_onSheetCmd) },
    { 0 , NULL }
};

static RES_NODE img_res_list [] = {
    {RES_BMPFONT_DIGITS, RES_TYPE_IMAGE, 16, 0},
    {RES_BMPFONT_COLON, RES_TYPE_IMAGE, 16, 0},
    {RES_BMPFONT_SPACE, RES_TYPE_IMAGE, 16, 0},
    {RES_BATTERY, RES_TYPE_IMAGE, 16, 0},
    {RES_CHARGING, RES_TYPE_IMAGE, 16, 0},
    {NULL},
};

static BOOL load_res (void)
{
    unsigned u;
    RES_NODE res_list [] = {
        { NULL, RES_TYPE_IMAGE, 16, 0},
        { NULL},
    };

    for (u = 0; u < TABLESIZE (_digit_keys); u++) {
        if (_digit_keys[u].type == KEY_TYPE_BTN) {
            res_list [0].res_name = _digit_keys[u].res_name;
            if (loadResByTag (res_list, 0) == 0) {
                _ERR_PRINTF ("ScreenLock: failed to load resource: %s\n", res_list [0].res_name);
                return FALSE;
            }
        }
    }

    u = loadResByTag (img_res_list, 0);
    if (u < (TABLESIZE (img_res_list) - 1)) {
        return FALSE;
    }

    return TRUE;
}

static void unload_res (void)
{
    unsigned u;
    RES_NODE res_list [] = {
        { NULL, RES_TYPE_IMAGE, 16, 0},
        { NULL},
    };

    for (u = 0; u < TABLESIZE (_digit_keys); u++) {
        res_list [0].res_name = _digit_keys[u].res_name;
        unloadResByTag (res_list, 0);
    }

    unloadResByTag (img_res_list, 0);
}

static BOOL _sl_onCreated (struct _NCS_CREATE_NOTIFY_INFO* info, mComponent* self, DWORD special_id)
{
    DLGTEMPLATE page_info = {
        WS_NONE,
        WS_EX_NONE,
        0, 0, 0, 0,
        "",
        0, 0,
        0, NULL,
        0
    };

    mPropSheet *propsheet = (mPropSheet*)(_c(self)->getChild (self, IDC_PROPSHEET));
    if (propsheet == NULL) {
        _ERR_PRINTF ("ScreenLock: failed to get property sheet control.\n");
        return FALSE;
    }

    mPage* page;

    page_info.caption = "Saver";
    page_info.dwAddData = PAGE_SAVER;
    page = _c(propsheet)->addPage (propsheet, &page_info, mypage_handlers);
    SetWindowBkColor (((mWidget*)page)->hwnd, PIXEL_black);

    page_info.caption = "Keypad";
    page_info.dwAddData = PAGE_KEYPAD;
    page = _c(propsheet)->addPage (propsheet, &page_info, mypage_handlers);
    SetWindowBkColor (((mWidget*)page)->hwnd, PIXEL_black);

    return TRUE;
}

static BOOL _sl_onCreate (mMainWnd* self, DWORD dwAddData)
{
    SL_INFO* sl_info = (SL_INFO*)dwAddData;

    mPropSheet *propsheet = (mPropSheet*)ncsGetChildObj (self->hwnd, IDC_PROPSHEET);
    if (propsheet == NULL) {
        _ERR_PRINTF ("ScreenLock: failed to get property sheet control in MSG_CREATE message.\n");
        return FALSE;
    }

    if (sl_info->reset_passwd) {
        _c(propsheet)->setProperty (propsheet, NCSP_PRPSHT_ACTIVEPAGEIDX, PAGE_KEYPAD);
    }

    return TRUE;
}

static void _sl_onTimer (mMainWnd* self, LINT id, DWORD tick_count)
{
}

static LRESULT _sl_onIdle (mMainWnd* self, UINT message, WPARAM wparam, LPARAM lparam)
{
    HWND mainwnd = self->hwnd;
    SL_INFO* sl_info = (SL_INFO*)GetWindowAdditionalData (mainwnd);

    mPropSheet *propsheet = (mPropSheet*)ncsGetChildObj (mainwnd, IDC_PROPSHEET);
    mPage* page = (mPage*)(_c(propsheet)->getPageByIndex (propsheet, PAGE_KEYPAD));
    mStatic* ctrl_prompt = (mStatic*)ncsObjFromHandle (GetDlgItem (page->hwnd, IDC_PROMPT));

    if (sl_info->nr_digits > 0 && sl_info->show_circles) {
        show_circles (ctrl_prompt->hwnd, sl_info->nr_digits);
        sl_info->show_circles = FALSE;
    }

    if (sl_info->nr_digits == 4) {

        if (sl_info->reset_passwd) {

            if (sl_info->new_passwd_1 [0] == 0) {
                strncpy (sl_info->new_passwd_1, sl_info->passwd_inputed, 4);
            }
            else if (sl_info->new_passwd_2 [0] == 0) {
                strncpy (sl_info->new_passwd_2, sl_info->passwd_inputed, 4);
            }
            else {
                _MG_PRINTF ("ScreenLock: here is a BUG: %d\n", __LINE__);
            }

            if (sl_info->passed) {
                if (sl_info->new_passwd_2 [0]) {
                    if (strcmp (sl_info->new_passwd_1, sl_info->new_passwd_2)) {
                        reset_sl_info (sl_info);

                        sl_info->prompt = _("Not matched, input again");
                        SetWindowText (ctrl_prompt->hwnd, sl_info->prompt);
                        sl_info->show_circles = FALSE;
                    }
                    else {
                        /* encrypt and save new passwd here */
                        save_password (sl_info->new_passwd_1);
                        SendNotifyMessage (mainwnd, MSG_MNWND_ENDDIALOG, 0, 0);
                    }
                }
                else if (sl_info->new_passwd_1 [0]) {
                    memset (sl_info->passwd_inputed, 0, sizeof(sl_info->passwd_inputed));
                    sl_info->nr_digits = 0;

                    sl_info->prompt = _("Input new password again");
                    SetWindowText (ctrl_prompt->hwnd, sl_info->prompt);
                    sl_info->show_circles = FALSE;
                }

                return 0;
            }
        }

        /* check password here */
        if (check_password (sl_info)) {
            if (sl_info->reset_passwd) {
                sl_info->passed = TRUE;
                sl_info->prompt = _("Input new password");
                SetWindowText (ctrl_prompt->hwnd, sl_info->prompt);
                sl_info->show_circles = FALSE;
                reset_sl_info (sl_info);
            }
            else {
                SendNotifyMessage (mainwnd, MSG_MNWND_ENDDIALOG, 0, 0);
            }
        }
        else {
            reset_sl_info (sl_info);
            sl_info->prompt = _("Wrong password, input again");
            SetWindowText (ctrl_prompt->hwnd, sl_info->prompt);
            sl_info->show_circles = FALSE;
        }
    }

    return 0;
}

static BOOL _sl_onKeyDown (mMainWnd* self, UINT message, int code, DWORD key_status)
{
    SL_INFO* sl_info = (SL_INFO*)GetWindowAdditionalData (self->hwnd);
    mPropSheet *propsheet = (mPropSheet*)ncsGetChildObj (self->hwnd, IDC_PROPSHEET);

    int idx_page = (int)_c(propsheet)->getProperty (propsheet, NCSP_PRPSHT_ACTIVEPAGEIDX);
    if (idx_page == PAGE_KEYPAD) {
        if (sl_info->reset_passwd) {
            _c(self)->endDialog (self, 0);
        }
        else if (code == SCANCODE_ESCAPE)
            _c(propsheet)->setProperty (propsheet, NCSP_PRPSHT_ACTIVEPAGEIDX, PAGE_SAVER);
    }
    else {
        if (strcmp (sl_info->curr_enc_passwd, INV_ENC_PASSWD) == 0)
            _c(self)->endDialog (self, 0);
        else
            _c(propsheet)->setProperty (propsheet, NCSP_PRPSHT_ACTIVEPAGEIDX, PAGE_KEYPAD);
    }

    return TRUE;
}

static BOOL _sl_onLButtonDown (mMainWnd* self, int x, int y, DWORD keyStatus)
{
    SL_INFO* sl_info = (SL_INFO*)GetWindowAdditionalData (self->hwnd);
    mPropSheet *propsheet = (mPropSheet*)ncsGetChildObj (self->hwnd, IDC_PROPSHEET);

    int idx_page = (int)_c(propsheet)->getProperty (propsheet, NCSP_PRPSHT_ACTIVEPAGEIDX);
    if (idx_page == PAGE_SAVER) {
        if (strcmp (sl_info->curr_enc_passwd, INV_ENC_PASSWD) == 0)
            _c(self)->endDialog (self, 0);
        else {
            _c(propsheet)->setProperty (propsheet, NCSP_PRPSHT_ACTIVEPAGEIDX, PAGE_KEYPAD);
            return TRUE;
        }
    }

    return FALSE;
}

static LRESULT _sl_onBatteryChanged (mMainWnd* self, UINT message, WPARAM wparam, LPARAM lparam)
{
    int level = (int)wparam;
    int status = (int)lparam;

    if (status < 0 || status > BATTERY_STATUS_FULL || level < 0 || level > 100) {
        _ERR_PRINTF ("screenLock > _sl_onBatteryChanged: bad parameters: status (%d), level (%d)\n", status, level);
        return 0;
    }

    mPropSheet* propsheet = (mPropSheet*)ncsGetChildObj (self->hwnd, IDC_PROPSHEET);
    mPage* page = (mPage*)(_c(propsheet)->getPageByIndex (propsheet, PAGE_SAVER));
    mWidget* ctrl;

    ctrl = (mWidget*)ncsObjFromHandle (GetDlgItem (page->hwnd, IDC_BATTERY_CAP));
    if (ctrl) {
        char buff [16];
        sprintf (buff, "%d%%", level);
        SetWindowText (ctrl->hwnd, buff);
    }

    ctrl = (mWidget*)ncsObjFromHandle (GetDlgItem (page->hwnd, IDC_BATTERY_LEVEL));
    if (ctrl) {
        _c(ctrl)->setProperty (ctrl, NCSP_PROG_CURPOS, level);
        if (level < 20) {
            ncsSetElement (ctrl, NCS_FGC_HILIGHT_ITEM, COLOR_STATE_ALARM);
        }
        else {
            ncsSetElement (ctrl, NCS_FGC_HILIGHT_ITEM, COLOR_STATE_NORMAL);
        }
        InvalidateRect (ctrl->hwnd, NULL, TRUE);
    }

    ctrl = (mWidget*)ncsObjFromHandle (GetDlgItem (page->hwnd, IDC_BATTERY_STATUS));
    if (ctrl) {
        if (status == BATTERY_STATUS_CHARGING) {
            ShowWindow (ctrl->hwnd, SW_SHOW);
        }
        else {
            ShowWindow (ctrl->hwnd, SW_HIDE);
        }
    }

    return 0;
}

static NCS_CREATE_NOTIFY_INFO _sl_create_notify_info = {
    onCreated: _sl_onCreated,
};

static NCS_EVENT_HANDLER _sl_handlers[] = {
    { MSG_CREATE, reinterpret_cast<void*>(_sl_onCreate) },
    { MSG_TIMER, reinterpret_cast<void*>(_sl_onTimer) },
	{ MSG_KEYDOWN, reinterpret_cast<void*>(_sl_onKeyDown) },
    { MSG_LBUTTONDOWN, reinterpret_cast<void*>(_sl_onLButtonDown) },
    { MSG_IDLE, reinterpret_cast<void*>(_sl_onIdle) },
    { MSG_BATTERY_CHANGED, reinterpret_cast<void*>(_sl_onBatteryChanged) },
    { 0, NULL }
};

static NCS_WND_TEMPLATE _sl_ctrl_templ[] =
{
    {
        class_name: NCSCTRL_PROPSHEET,
        id: IDC_PROPSHEET,
        x: -1,
        y: -1,
        w: SCREEN_W + 2,
        h: SCREEN_H + 40,
        style: WS_VISIBLE | NCSS_PRPSHT_BOTTOM,
        ex_style: WS_EX_NONE,
        caption: "",
    },
};

//define the main window template
static NCS_MNWND_TEMPLATE _sl_templ = {
    NCSCTRL_DIALOGBOX, 
    1,
    0, 0, SCREEN_W, SCREEN_H,
    WS_NONE, WS_EX_TOPMOST | WS_EX_AUTOSECONDARYDC,
    "Screen Lock",
    NULL,
    NULL,
    _sl_handlers,
    _sl_ctrl_templ,
    TABLESIZE(_sl_ctrl_templ),
    0, 0, NULL, 0, 0,
    &_sl_create_notify_info,
    0
};

void screenLock (HWND hosting, BOOL reset)
{
    const char* passwd;
    SL_INFO sl_info = {reset, };

    passwd = RTCFGSERVICE->getScreenLockPassword ();
    if (passwd == NULL) {
        return;
    }

    if (!load_res ())
        return;

    if (strlen (passwd) < 16)
        strcpy (sl_info.curr_enc_passwd, INV_ENC_PASSWD);
    else
        strncpy (sl_info.curr_enc_passwd, passwd, LEN_ENC_PASSWD);

    sl_info.bmp_devfont = CreateBMPDevFont ("bmp-joopic-rrncnn-38-60-ISO8859-1", 
                (BITMAP*)GetResource (Str2Key (RES_BMPFONT_DIGITS)), "0", 10, 38);
    AddGlyphsToBMPFont (sl_info.bmp_devfont,
                (BITMAP*)GetResource (Str2Key (RES_BMPFONT_COLON)), ":", 1, 18);
    AddGlyphsToBMPFont (sl_info.bmp_devfont,
                (BITMAP*)GetResource (Str2Key (RES_BMPFONT_SPACE)), " ", 1, 18);

    sl_info.bmp_logfont = CreateLogFont (FONT_TYPE_NAME_BITMAP_BMP, "joopic",
                "ISO8859-1",
                FONT_WEIGHT_BOLD, FONT_SLANT_ITALIC,
                FONT_SETWIDTH_NORMAL, FONT_SPACING_CHARCELL,
                FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE,
                60, 0);

    _sl_templ.user_data = (DWORD)&sl_info;
    mDialogBox* mydlg = (mDialogBox *)ncsCreateMainWindowIndirect (&_sl_templ, hosting);
    _c(mydlg)->doModal (mydlg, TRUE);

    DestroyBMPFont (sl_info.bmp_devfont);
    DestroyLogFont (sl_info.bmp_logfont);

    unload_res ();

}

