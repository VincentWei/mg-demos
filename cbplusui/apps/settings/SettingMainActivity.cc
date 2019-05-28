/*
** This file is a part of mg-demos package.
**
** Copyright (C) 2019 FMSoft (http://www.fmsoft.cn).
** Copyright (C) 2018 Beijing Joobot Technologies Co., Ltd.
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/
/*!============================================================================
 * @file SettingMainActivity.cc
 * @Synopsis The main activity of Setting app.
 * @author Vincent Wei
 * @version 1.0
 * @date 05/06/2018
 */

#include "global.h"
#include "utilities.h"
#include "misc.h"

#include "resource.h"

#include "Activity.hh"
#include "ActivityStack.hh"
#include "NCSActivity.hh"

#include "RuntimeConfService.hh"
#include "SysConfService.hh"
#include "NetworkService.hh"
#include "MiscService.hh"
#include "SettingMainActivity.hh"
#include "SettingTimeZoneActivity.hh"
#include "MenuListPiece.hh"

REGISTER_ACTIVITY(SettingMainActivity);

#define IDE_CELLULAR                (IDE_MIN + 0)
#define IDE_WIFI                    (IDE_MIN + 1)

#ifdef OEM_TPP
    #define IDE_SCREEN                  (IDE_MIN + 2)
    #define IDE_FIRMWARE                (IDE_MIN + 3)
    #define IDE_ABOUT                   (IDE_MIN + 4)
#else
    #define IDE_FTPSERVER               (IDE_MIN + 2)
    #define IDE_TRANSPROTO              (IDE_MIN + 3)
    #define IDE_FUNCTION                (IDE_MIN + 4)
    #define IDE_LANGUAGE                (IDE_MIN + 5)
    #define IDE_TIME                    (IDE_MIN + 6)
    #define IDE_SCREEN                  (IDE_MIN + 7)
    #define IDE_PAIR                    (IDE_MIN + 8)
    #define IDE_FIRMWARE                (IDE_MIN + 9)
    #define IDE_RESET                   (IDE_MIN + 10)
    #define IDE_ABOUT                   (IDE_MIN + 11)
#endif

static MENU_ENTRY _menu_entries [] = {
    { IDE_CELLULAR, TYPE_MENU_ENTRY_SWITCH, NULL },
    { IDE_WIFI,     TYPE_MENU_ENTRY_ACT,    "SettingWifiActivity" },
#ifdef OEM_TPP
    { IDE_SCREEN,   TYPE_MENU_ENTRY_ACT,    "SettingScreenActivity" },
    { IDE_FIRMWARE, TYPE_MENU_ENTRY_ACT,    "SettingFirmwareActivity" },
    { IDE_ABOUT,    TYPE_MENU_ENTRY_ACT,    "SettingAboutActivity" },
#else
    { IDE_FTPSERVER,    TYPE_MENU_ENTRY_ACT,    "SettingFtpServerActivity" },
    { IDE_TRANSPROTO,   TYPE_MENU_ENTRY_ACT,    "SettingTransferProtocolActivity" },
    { IDE_FUNCTION, TYPE_MENU_ENTRY_ACT,    "SettingFunctionActivity" },
    { IDE_LANGUAGE, TYPE_MENU_ENTRY_ACT,    "SettingLanguageActivity" },
    { IDE_TIME,     TYPE_MENU_ENTRY_ACT,    "SettingTimeActivity" },
    { IDE_SCREEN,   TYPE_MENU_ENTRY_ACT,    "SettingScreenActivity" },
    { IDE_PAIR,     TYPE_MENU_ENTRY_ACT,    "SettingPairActivity" },
    { IDE_FIRMWARE, TYPE_MENU_ENTRY_ACT,    "SettingFirmwareActivity" },
    { IDE_RESET,    TYPE_MENU_ENTRY_ACT,    "SettingResetActivity" },
    { IDE_ABOUT,    TYPE_MENU_ENTRY_ACT,    "SettingAboutActivity" },
#endif
};

static const char* get_protocol_name(int protocol)
{
    static const char* transfer_protocol_names [] = {
        N_("Unknown"),
        N_("Joopic Cloud"),
        N_("FTP"),
    };

    if (protocol >= 0 && protocol <= SYS_TRANSFER_PROTOCOL_MAX)
        return _(transfer_protocol_names[protocol + 1]);
    return _(transfer_protocol_names[0]);
}

static void init_menu_entries (void)
{
    const NETWORK_STATE* ns = NETWORKSERVICE->getState ();

    SYSCFGSERVICE->refreshUserInfo ();

    _menu_entries [IDE_CELLULAR - IDE_MIN].name = _("4G Network");
    if (ns->lte == NETWORK_STATE_DISABLED) {
        _menu_entries [IDE_CELLULAR - IDE_MIN].value = SWITCH_OFF;
    }
    else {
        _menu_entries [IDE_CELLULAR - IDE_MIN].value = SWITCH_ON;
    }

    _menu_entries [IDE_WIFI - IDE_MIN].name = _("WiFi Network");

    HOTSPOT_INFO hotspot_info;
    if (MISCSERVICE->getHotspotStatus(&hotspot_info) && hotspot_info.enabled) {
        _menu_entries [IDE_WIFI - IDE_MIN].value = _("Hotspot enabled");
    }
    else {
        if (ns->wifi == NETWORK_STATE_DISABLED) {
            _menu_entries [IDE_WIFI - IDE_MIN].value = _("Closed");
        }
        else if (ns->wifi == NETWORK_STATE_CONNECTING) {
            _menu_entries [IDE_WIFI - IDE_MIN].value = _("Connecting…");
        }
        else if (ns->wifi == NETWORK_STATE_CONNECTED) {
            _menu_entries [IDE_WIFI - IDE_MIN].value = ns->curr_ssid;
        }
        else {
            _menu_entries [IDE_WIFI - IDE_MIN].value = _("Not connected");
        }
    }

#ifndef OEM_TPP
    _menu_entries [IDE_FTPSERVER - IDE_MIN].name = _("FTP Server");

    const char * ftp_host = SYSCFGSERVICE->getFtpServerHostname();
    if (ftp_host)
        _menu_entries [IDE_FTPSERVER - IDE_MIN].value = ftp_host;
    else
        _menu_entries [IDE_FTPSERVER - IDE_MIN].value = _("Unset");

    _menu_entries [IDE_TRANSPROTO - IDE_MIN].name = _("Transfer Protocol");
    if (ftp_host) {
        _menu_entries [IDE_TRANSPROTO - IDE_MIN].value = get_protocol_name (SYSCFGSERVICE->getTransferProtocol());
        _menu_entries [IDE_TRANSPROTO - IDE_MIN].activity = "SettingTransferProtocolActivity";
    }
    else {
        _menu_entries [IDE_TRANSPROTO - IDE_MIN].value = get_protocol_name (SYS_TRANSFER_PROTOCOL_JOOPIC);
        _menu_entries [IDE_TRANSPROTO - IDE_MIN].activity = NULL;
    }

    _menu_entries [IDE_FUNCTION - IDE_MIN].name = _("Function");
    _menu_entries [IDE_FUNCTION - IDE_MIN].value = NULL;

    _menu_entries [IDE_LANGUAGE - IDE_MIN].name = _("Language");
    _menu_entries [IDE_LANGUAGE - IDE_MIN].value = RTCFGSERVICE->m_lang_settings->langs.at(RTCFGSERVICE->m_lang_settings->current).c_str();
    _menu_entries [IDE_LANGUAGE - IDE_MIN].intent_key = "firstBoot";
    _menu_entries [IDE_LANGUAGE - IDE_MIN].intent_value = "no";

    _menu_entries [IDE_TIME - IDE_MIN].name = _("Time");
    _menu_entries [IDE_TIME - IDE_MIN].value = GetTimeZoneName (RTCFGSERVICE->getTimeZone ());
#endif

    _menu_entries [IDE_SCREEN - IDE_MIN].name = _("Screen");
    _menu_entries [IDE_SCREEN - IDE_MIN].value = RTCFGSERVICE->getScreenLockTimeoutText ();

#ifndef OEM_TPP
    _menu_entries [IDE_PAIR - IDE_MIN].name = _("Pair");
    if (SYSCFGSERVICE->getBoundUserId () == NULL) {
        _menu_entries [IDE_PAIR - IDE_MIN].value = _("Not Paired");
    }
    else {
        _menu_entries [IDE_PAIR - IDE_MIN].value = _("Paired");
    }
    _menu_entries [IDE_PAIR - IDE_MIN].intent_key = "firstBoot";
    _menu_entries [IDE_PAIR - IDE_MIN].intent_value = "no";

    _menu_entries [IDE_RESET - IDE_MIN].name = _("Reset");
    _menu_entries [IDE_RESET - IDE_MIN].value = NULL;
#endif

    _menu_entries [IDE_FIRMWARE - IDE_MIN].name = _("Firmware");
    _menu_entries [IDE_FIRMWARE - IDE_MIN].value = SYSCFGSERVICE->getVersionNumber();

    _menu_entries [IDE_ABOUT - IDE_MIN].name = _("About");
    _menu_entries [IDE_ABOUT - IDE_MIN].value = SYSCFGSERVICE->getDeviceName();
}

static mMenuListPiece* _listpiece;
static mWidget* create_content (mMainWnd* self)
{
    NCS_WND_TEMPLATE ctrl_templ = {
        class_name: NCSCTRL_CONTAINERCTRL,
        id: IDC_CONTENT,
        x: ACTIVITY_CONTENT_X,
        y: ACTIVITY_CONTENT_Y,
        w: ACTIVITY_CONTENT_W,
        h: ACTIVITY_CONTENT_H,
        style: WS_VISIBLE | NCSS_NOTIFY,
        ex_style: WS_EX_NONE,
        caption: "",
    };

    mWidget* ctrl_ctnr = ncsCreateWindowIndirect (&ctrl_templ, self->hwnd);
    if (ctrl_ctnr) {
        RECT rc = {0, 0, ACTIVITY_CONTENT_W, ACTIVITY_CONTENT_H};
        _listpiece = createMenuListView ((mContainerCtrl*)ctrl_ctnr, &rc, _menu_entries, TABLESIZE (_menu_entries));
    }

    return ctrl_ctnr;
}

#if 0
static void destroy_content (mMainWnd* self)
{
    mWidget* ctrl_content = (mWidget*)(_c(self)->getChild (self, IDC_CONTENT));
    if (ctrl_content) {
        DestroyWindow (ctrl_content->hwnd);
    }
}

static void hide_loading (mMainWnd* self)
{
    mAnimate* ctrl_loading = (mAnimate*)(_c(self)->getChild (self, IDC_LOADING));
    if (ctrl_loading) {
        ShowWindow (ctrl_loading->hwnd, SW_HIDE);
    }
}

static void show_loading (mMainWnd* self)
{
    mAnimate* ctrl_loading = (mAnimate*)(_c(self)->getChild (self, IDC_LOADING));
    if (ctrl_loading) {
        ShowWindow (ctrl_loading->hwnd, SW_SHOW);
    }
}

static mWidget* create_loading (mMainWnd* self)
{
    char gif_path [MAX_PATH + 1];
    strcpy (gif_path, GetResPath ());
    strcat (gif_path, "/");
    strcat (gif_path, RES_GLOBAL_LOADING_SM);

    NCS_PROP_ENTRY props [] = {
        {NCSP_ANMT_GIFFILE, (DWORD)gif_path},
        {NCSP_ANMT_INTERVAL, (DWORD)10},
        {-1, 0},
    };

    NCS_WND_TEMPLATE ctrl_templ = {
        class_name: NCSCTRL_ANIMATE,
        id: IDC_LOADING,
        x: ACTIVITY_CONTENT_X,
        y: ACTIVITY_CONTENT_Y + 10,
        w: ACTIVITY_CONTENT_W,
        h: 28,
        style: WS_VISIBLE | NCSS_ANMT_AUTOPLAY | NCSS_ANMT_AUTOLOOP,
        ex_style: WS_EX_NONE,
        caption: "",
    };

    ctrl_templ.props = props;
    return ncsCreateWindowIndirect (&ctrl_templ, self->hwnd);
}
#endif

static BOOL mymain_onCommand (mMainWnd* self, int id, int nc, HWND hCtrl)
{
    if (id == IDC_CONTENT && nc < (int)TABLESIZE (_menu_entries)
            && _menu_entries[nc].type == TYPE_MENU_ENTRY_ACT
            && _menu_entries[nc].activity != NULL) {

        Intent* my_intent = NULL;
        if (_menu_entries[nc].intent_key && _menu_entries[nc].intent_value) {
            my_intent = Intent::newIntent ();
            my_intent->putExtra (_menu_entries[nc].intent_key, std::string (_menu_entries[nc].intent_value));
        }

        ACTIVITYSTACK->push (_menu_entries[nc].activity, my_intent);
    }

    return TRUE;
}

static LRESULT mymain_onSwitchStateChanged (mMainWnd* self, UINT msg, DWORD wparam, DWORD lparam)
{
    int row = (int)wparam;
    int event_id = (int)lparam;

    if (row == (IDE_CELLULAR - IDE_MIN)) {
        if (event_id == NCSN_SLIDESWITCHPIECE_OFF || event_id == NCSN_SLIDESWITCHPIECE_ON) {
            SettingMainActivity* act;
            act = (SettingMainActivity*)GetWindowAdditionalData (self->hwnd);
            mAnimate* ctrl_loading = (mAnimate*)(ncsGetChildObj(self->hwnd, IDC_LOADING));

            ncsAnimateStart (ctrl_loading);
            ShowWindow (((mWidget*)ctrl_loading)->hwnd, SW_SHOW);
            act->setAutoEscape (false);
            NETWORKSERVICE->enableLTE (self->hwnd, (event_id == NCSN_SLIDESWITCHPIECE_ON));
        }
    }

    return 0;
}

static LRESULT mymain_onAsyncTaskFinished (mMainWnd* self, UINT msg, DWORD wparam, DWORD lparam)
{
    SettingMainActivity* act;
    act = (SettingMainActivity*)GetWindowAdditionalData (self->hwnd);

    if (wparam == TASKID_NETWORK_ENABLE_LTE) {
        WORD cmd = LOWORD (lparam);
        WORD status = HIWORD (lparam);
        mAnimate* ctrl_loading = (mAnimate*)(ncsGetChildObj(self->hwnd, IDC_LOADING));

        act->setAutoEscape (true);
        ncsAnimateStop (ctrl_loading);
        ShowWindow (((mWidget*)ctrl_loading)->hwnd, SW_HIDE);

        if (status) { // When failed, we need to refresh the listview.
            if (cmd) {
                _menu_entries [IDE_CELLULAR - IDE_MIN].value = SWITCH_OFF;
                showToastWindow (NULL, _("Failed to enable LTE."), COLOR_TOAST_WARNING, TRUE);
            }
            else {
                _menu_entries [IDE_CELLULAR - IDE_MIN].value = SWITCH_ON;
                showToastWindow (NULL, _("Failed to disable LTE."), COLOR_TOAST_WARNING, TRUE);
            }

            _c(_listpiece)->reloadData (_listpiece);
        }
    }
    else {
        _DBG_PRINTF ("SettingMainActivity: Unknown task.\n");
    }

    return 0L;
}

static LRESULT mymain_onReady (mMainWnd* self, UINT msg, DWORD wparam, DWORD lparam)
{
    if (wparam == ACTIVITY_POPPED) {
        init_menu_entries ();
        _c(_listpiece)->reloadData (_listpiece);
    }

    return 0;
}

#if 0
static LRESULT mymain_onAsyncTaskFinished (mMainWnd* self, UINT msg, DWORD wparam, DWORD lparam)
{
    NETWORK_STATE* ns = (NETWORK_STATE*)lparam;

    _menu_entries [IDE_CELLULAR - IDE_MIN].name = _("4G Network");
    if (ns->lte == NETWORK_STATE_DISABLED) {
        _menu_entries [IDE_CELLULAR - IDE_MIN].value = SWITCH_OFF;
    }
    else {
        _menu_entries [IDE_CELLULAR - IDE_MIN].value = SWITCH_ON;
    }

    _menu_entries [IDE_WIFI - IDE_MIN].name = _("WLAN");
    if (ns->wifi == NETWORK_STATE_DISABLED) {
        _menu_entries [IDE_WIFI - IDE_MIN].value = _("Closed");
    }
    else if (ns->wifi == NETWORK_STATE_CONNECTED) {
        _menu_entries [IDE_WIFI - IDE_MIN].value = strncpy (_buff_curr_ssid, ns->curr_ssid, MAX_LEN_SSID);
    }
    else {
        _menu_entries [IDE_WIFI - IDE_MIN].value = _("Unset");
    }

    _menu_entries [IDE_FUNCTION - IDE_MIN].name = _("Function");
    _menu_entries [IDE_FUNCTION - IDE_MIN].value = _("Opened");

    _menu_entries [IDE_LANGUAGE - IDE_MIN].name = _("Language");
    _menu_entries [IDE_LANGUAGE - IDE_MIN].value = RTCFGSERVICE->m_lang_settings->langs.at(RTCFGSERVICE->m_lang_settings->current).c_str();
    _menu_entries [IDE_LANGUAGE - IDE_MIN].intent_key = "firstBoot";
    _menu_entries [IDE_LANGUAGE - IDE_MIN].intent_value = "no";

    _menu_entries [IDE_TIME - IDE_MIN].name = _("Time");
    _menu_entries [IDE_TIME - IDE_MIN].value = _("Auto");

    _menu_entries [IDE_SCREEN - IDE_MIN].name = _("Screen");
    _menu_entries [IDE_SCREEN - IDE_MIN].value = _("30 seconds");

    _menu_entries [IDE_PAIR - IDE_MIN].name = _("Pair");
    _menu_entries [IDE_PAIR - IDE_MIN].value = _("Paired");
    _menu_entries [IDE_PAIR - IDE_MIN].intent_key = "firstBoot";
    _menu_entries [IDE_PAIR - IDE_MIN].intent_value = "no";

    _menu_entries [IDE_FIRMWARE - IDE_MIN].name = _("Firmware");
    _menu_entries [IDE_FIRMWARE - IDE_MIN].value = SYSCFGSERVICE->getVersionNumber();

    _menu_entries [IDE_ABOUT - IDE_MIN].name = _("About");
    _menu_entries [IDE_ABOUT - IDE_MIN].value = SYSCFGSERVICE->getDeviceName();

    hide_loading (self);
    create_content (self);

    SettingMainActivity* act;
    act = (SettingMainActivity*)GetWindowAdditionalData (self->hwnd);
    act->setAutoEscape (true);
    return 0;
}
#endif

static BOOL mymain_onCreate (mMainWnd* self, DWORD dwAddData)
{
    /* init loading GIF */
    char gif_path [MAX_PATH + 1];
    strcpy (gif_path, GetResPath ());
    strcat (gif_path, "/");
    strcat (gif_path, RES_GLOBAL_LOADING_XM);

    mAnimate *animate = (mAnimate*)(_c(self)->getChild (self, IDC_LOADING));
    _c(animate)->setProperty (animate, NCSP_ANMT_GIFFILE, (DWORD)gif_path);
    _c(animate)->setProperty (animate, NCSP_ANMT_INTERVAL, (DWORD)10);

#if 0
    if (create_loading (self) == NULL) {
        _ERR_PRINTF ("SettingMainActivity: failed to create loading animation.\n");
        return FALSE;
    }
#endif

    return TRUE;
}

static BOOL mymain_onClose (mMainWnd* self, int message)
{
    _MG_PRINTF ("SettingMainActivity: mymain_onClose called.\n");
    return TRUE;
}

static BOOL mymain_onCreated (struct _NCS_CREATE_NOTIFY_INFO* info, mComponent* self, DWORD special_id)
{
    mWidget* ctrl_title = (mWidget*)(_c(self)->getChild (self, IDC_TITLE));
    SetWindowText (ctrl_title->hwnd, _("Settings"));
    ncsSetElement (ctrl_title, NCS_FGC_WINDOW, COLOR_TITLE);
    ncsSetFont (ctrl_title->hwnd, FONT_TITLE);

    init_menu_entries ();
    create_content ((mMainWnd*)self);

    return TRUE;
}

static NCS_CREATE_NOTIFY_INFO create_notify_info = {
    onCreated: mymain_onCreated,
};

static NCS_EVENT_HANDLER mymain_handlers [] = {
    {MSG_CREATE, reinterpret_cast<void*>(mymain_onCreate)},
    {MSG_ASYNC_TASK_FINISHED, reinterpret_cast<void*>(mymain_onAsyncTaskFinished)},
    {MSG_USER_APP_READY, reinterpret_cast<void*>(mymain_onReady)},
    {MSG_COMMAND, reinterpret_cast<void*>(mymain_onCommand)},
    {MSG_USER, reinterpret_cast<void*>(mymain_onSwitchStateChanged)},
    {MSG_CLOSE, reinterpret_cast<void*>(mymain_onClose)},
    {MSG_LBUTTONDOWN, reinterpret_cast<void*>(SpeedMeterMessageHandler)},
    {MSG_LBUTTONUP, reinterpret_cast<void*>(SpeedMeterMessageHandler)},
    {MSG_MOUSEMOVE, reinterpret_cast<void*>(SpeedMeterMessageHandler)},
    {0, NULL}
};

static NCS_WND_TEMPLATE _ctrl_templ[] =
{
    {
        class_name: NCSCTRL_STATIC,
        id: IDC_TITLE,
        x: ACTIVITY_TITLE_X + 5,
        y: ACTIVITY_TITLE_Y,
        w: ACTIVITY_TITLE_W - SIZE_LOADING_GIF_XM - 5 - 5,
        h: ACTIVITY_TITLE_H,
        style: WS_VISIBLE | NCSS_NOTIFY,
        ex_style: WS_EX_NONE,
        caption: "",
    },
    {
        class_name: NCSCTRL_ANIMATE,
        id: IDC_LOADING,
        x: ACTIVITY_TITLE_W - SIZE_LOADING_GIF_XM - 5 - 5,
        y: ACTIVITY_TITLE_Y,
        w: SIZE_LOADING_GIF_XM,
        h: ACTIVITY_TITLE_H,
        style: NCSS_ANMT_AUTOLOOP | NCSS_ANMT_AUTOPLAY,
        ex_style: WS_EX_NONE,
        caption: "",
    },
    {
        class_name: NCSCTRL_SEPARATOR,
        id: IDC_STATIC,
        x: ACTIVITY_SEPARATOR_X,
        y: ACTIVITY_SEPARATOR_Y,
        w: ACTIVITY_SEPARATOR_W,
        h: ACTIVITY_SEPARATOR_H,
        style: WS_VISIBLE,
        ex_style: WS_EX_NONE,
        caption: "",
    },
};

static NCS_MNWND_TEMPLATE mymain_templ = {
    NCSCTRL_DIALOGBOX,
    1,
    ACTIVITY_X, ACTIVITY_Y, ACTIVITY_W, ACTIVITY_H,
    WS_NONE, WS_EX_NONE,
    N_("Settings"),
    NULL,
    NULL,
    mymain_handlers,
    _ctrl_templ,
    TABLESIZE(_ctrl_templ),
    0, 0, NULL, 0, 0,
    &create_notify_info,
    0
};

SettingMainActivity::SettingMainActivity() : NCSActivity (&mymain_templ, true)
{
    MGNCS_INIT_CLASS (mMenuListPiece);
    m_style = STYLE_PUSH;
}

SettingMainActivity::~SettingMainActivity()
{
}

