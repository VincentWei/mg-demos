/*!============================================================================
 * @file SettingWifiActivity.cc 
 * @Synopsis The Wifi activity of Setting app. 
 * @author Vincent Wei
 * @version 1.0
 * @date 12/06/2018
 *
 *  Copyright (C) 2018 Beijing Joobot Technogolies Inc.
 */

#include "global.h"
#include "utilities.h"
#include "misc.h"

#include "resource.h"

#include "Activity.hh"
#include "ActivityStack.hh"
#include "NCSActivity.hh"

#include "NetworkService.hh"
#include "MiscService.hh"
#include "SettingWifiActivity.hh"
#include "MenuListPiece.hh"

REGISTER_ACTIVITY(SettingWifiActivity);

#define IDE_SWITCH_WLAN             (IDE_MIN + 0)
#define IDE_WIFI_LIST               (IDE_MIN + 1)
#ifndef OEM_TPP
    #define IDE_HOTSPOT                 (IDE_MIN + 2)
#endif

static MENU_ENTRY _menu_entries [] = {
    { IDE_SWITCH_WLAN, TYPE_MENU_ENTRY_SWITCH, NULL },
    { IDE_WIFI_LIST,   TYPE_MENU_ENTRY_ACT,    NULL, NULL, NULL },
#ifndef OEM_TPP
    { IDE_HOTSPOT,     TYPE_MENU_ENTRY_ACT,    "SettingHotspotActivity", NULL },
#endif
};

static BOOL mymain_onCommand (mMainWnd* self, int id, int nc, HWND hCtrl)
{
    if (id == IDC_CONTENT && nc < (int)TABLESIZE (_menu_entries)
            && _menu_entries[nc].type == TYPE_MENU_ENTRY_ACT
            && _menu_entries[nc].activity != NULL) {
        Intent* my_intent = Intent::newIntent ();
        my_intent->putExtra ("firstBoot", std::string ("no"));
        ACTIVITYSTACK->push (_menu_entries[nc].activity, my_intent);
    }

    return TRUE;
}

static LRESULT mymain_onSwitchStateChanged (mMainWnd* self, UINT msg, DWORD wparam, DWORD lparam)
{
    int row = (int)wparam;
    int event_id = (int)lparam;

    if (row == (IDE_SWITCH_WLAN - IDE_MIN)) {
        if (event_id == NCSN_SLIDESWITCHPIECE_OFF || event_id == NCSN_SLIDESWITCHPIECE_ON) {
            SettingWifiActivity* act;
            act = (SettingWifiActivity*)GetWindowAdditionalData (self->hwnd);
            mAnimate* ctrl_loading = (mAnimate*)(ncsGetChildObj(self->hwnd, IDC_LOADING));

            ncsAnimateStart (ctrl_loading);
            ShowWindow (((mWidget*)ctrl_loading)->hwnd, SW_SHOW);
            act->setAutoEscape (false);
            NETWORKSERVICE->enableWiFi (self->hwnd, (event_id == NCSN_SLIDESWITCHPIECE_ON));
        }
    }

    return 0;
}

static mMenuListPiece* _listpiece;
static HOTSPOT_INFO _hotspot_info;

static void init_menu_entries (BOOL reload)
{
    const NETWORK_STATE* ns;
    ns = NETWORKSERVICE->getState ();

    _menu_entries [IDE_SWITCH_WLAN - IDE_MIN].name = _("WLAN");
    _menu_entries [IDE_WIFI_LIST - IDE_MIN].value = NULL;
    _menu_entries [IDE_WIFI_LIST - IDE_MIN].activity = "SettingWifiListActivity";

    switch (ns->wifi) {
    case NETWORK_STATE_DISABLED:
        _menu_entries [IDE_SWITCH_WLAN - IDE_MIN].value = SWITCH_OFF;
        _menu_entries [IDE_WIFI_LIST - IDE_MIN].activity = NULL;
        _menu_entries [IDE_WIFI_LIST - IDE_MIN].name = _("Closed");
        break;
    case NETWORK_STATE_CONNECTED:
        _menu_entries [IDE_SWITCH_WLAN - IDE_MIN].value = SWITCH_ON;
        if (ns->curr_ssid == NULL || strcmp (ns->curr_ssid, "") == 0) {
            _menu_entries [IDE_WIFI_LIST - IDE_MIN].name = _("Connecting…");
        }
        else {
            _menu_entries [IDE_WIFI_LIST - IDE_MIN].name = ns->curr_ssid;
        }
        break;
    case NETWORK_STATE_CONNECTING:
        _menu_entries [IDE_SWITCH_WLAN - IDE_MIN].value = SWITCH_ON;
        _menu_entries [IDE_WIFI_LIST - IDE_MIN].name = _("Connecting…");
        break;
    default:
        _menu_entries [IDE_SWITCH_WLAN - IDE_MIN].value = SWITCH_ON;
        _menu_entries [IDE_WIFI_LIST - IDE_MIN].name = _("Not connected");
        break;
    }

#ifndef OEM_TPP
    _menu_entries [IDE_HOTSPOT - IDE_MIN].name = _("WiFi Hotspot");
    if (MISCSERVICE->getHotspotStatus(&_hotspot_info)
            && _hotspot_info.enabled) {
        _menu_entries [IDE_HOTSPOT - IDE_MIN].value = _hotspot_info.ssid;
    }
    else {
        _menu_entries [IDE_HOTSPOT - IDE_MIN].value = _("Not enabled");
    }
#endif

    if (reload)
        _c(_listpiece)->reloadData (_listpiece);
}

static LRESULT mymain_onWiFiChanged (mMainWnd* self, UINT msg, DWORD wparam, DWORD lparam)
{
    if (strcmp (_menu_entries [IDE_SWITCH_WLAN - IDE_MIN].value, SWITCH_ON) == 0) {
        init_menu_entries (TRUE);
    }

    return 0;
}

static LRESULT mymain_onAsyncTaskFinished (mMainWnd* self, UINT msg, DWORD wparam, DWORD lparam)
{
    SettingWifiActivity* act;
    act = (SettingWifiActivity*)GetWindowAdditionalData (self->hwnd);

    if (wparam == TASKID_NETWORK_ENABLE_WIFI) {
        WORD cmd = LOWORD (lparam);
        WORD status = HIWORD (lparam);
        if (status) { // When failed, we need to refresh the listview and show a toast.
            if (cmd) {
                showToastWindow (NULL, _("Failed to enable WLAN."), COLOR_TOAST_WARNING, TRUE);
            }
            else {
                showToastWindow (NULL, _("Failed to disable WLAN."), COLOR_TOAST_WARNING, TRUE);
            }
        }
        else {
            NETWORKSERVICE->getInstantState (self->hwnd);
            return 0L;
        }
    }
    else if (wparam == TASKID_NETWORK_STATE) {
        NETWORK_STATE* ns = (NETWORK_STATE*)lparam;
        /* save the new state to NETWORKSERVICE */
        NETWORKSERVICE->onStateChanged (ns);

        init_menu_entries (TRUE);
    }
    else {
        _DBG_PRINTF ("SettingWifiActivity: Unknown task.\n");
        return 0L;
    }

    mAnimate* ctrl_loading = (mAnimate*)(ncsGetChildObj(self->hwnd, IDC_LOADING));

    act->setAutoEscape (true);
    ncsAnimateStop (ctrl_loading);
    ShowWindow (((mWidget*)ctrl_loading)->hwnd, SW_HIDE);
    return 0L;
}

static LRESULT mymain_onReady (mMainWnd* self, UINT msg, DWORD wparam, DWORD lparam)
{
    if (wparam == ACTIVITY_PUSHED) {
    }
    else if (wparam == ACTIVITY_POPPED) {
        init_menu_entries (TRUE);
    }

    return 0;
}

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

        init_menu_entries (FALSE);
        _listpiece = createMenuListView ((mContainerCtrl*)ctrl_ctnr, &rc, _menu_entries, TABLESIZE (_menu_entries));
    }

    return TRUE;
}

static BOOL mymain_onClose (mMainWnd* self, int message)
{
    _MG_PRINTF ("SettingWifiActivity: mymain_onClose called.\n");

    return TRUE;
}

static BOOL mymain_onCreated (struct _NCS_CREATE_NOTIFY_INFO* info, mComponent* self, DWORD special_id)
{
    mWidget* ctrl_title = (mWidget*)(_c(self)->getChild (self, IDC_TITLE));
    mHotPiece* piece_title = (mHotPiece*)(ctrl_title->body);
    _c(piece_title)->setProperty (piece_title, NCSP_LABELPIECE_LABEL, (DWORD)_("WiFi Network"));
    ncsSetElement (ctrl_title, NCS_FGC_WINDOW, COLOR_TITLE);
    ncsSetFont (ctrl_title->hwnd, FONT_TITLE);
    return TRUE;
}

static NCS_CREATE_NOTIFY_INFO create_notify_info = {
    onCreated: mymain_onCreated,
};

static NCS_EVENT_HANDLER mymain_handlers [] = {
    {MSG_CREATE, reinterpret_cast<void*>(mymain_onCreate)},
    {MSG_COMMAND, reinterpret_cast<void*>(mymain_onCommand)},
    {MSG_USER, reinterpret_cast<void*>(mymain_onSwitchStateChanged)},
    {MSG_WIFI_CHANGED, reinterpret_cast<void*>(mymain_onWiFiChanged)},
    {MSG_ASYNC_TASK_FINISHED, reinterpret_cast<void*>(mymain_onAsyncTaskFinished)},
    {MSG_USER_APP_READY, reinterpret_cast<void*>(mymain_onReady)},
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
    N_("WiFi Network"),
    NULL,
    NULL,
    mymain_handlers,
    _ctrl_templ,
    TABLESIZE(_ctrl_templ),
    0, 0, NULL, 0, 0,
    &create_notify_info,
    0
};

SettingWifiActivity::SettingWifiActivity() : NCSActivity (&mymain_templ)
{ 
    m_style = STYLE_PUSH;
}

SettingWifiActivity::~SettingWifiActivity()
{
}

