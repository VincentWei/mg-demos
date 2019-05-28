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
 * @file SettingWifiListActivity.cc
 * @Synopsis The Wifi list activity of Setting app.
 * @author Vincent Wei
 * @version 1.0
 * @date 19/05/2018
 */

#include "global.h"
#include "utilities.h"
#include "misc.h"

#include "resource.h"

#include "Activity.hh"
#include "ActivityStack.hh"
#include "NCSActivity.hh"

#include "NetworkService.hh"
#include "WifiListPiece.hh"
#include "SettingWifiListActivity.hh"
#include "SettingWifiPasswordActivity.hh"

REGISTER_ACTIVITY(SettingWifiListActivity);

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

static BOOL is_loading_shown (mMainWnd* self)
{
    mAnimate* ctrl_loading = (mAnimate*)(_c(self)->getChild (self, IDC_LOADING));
    if (ctrl_loading) {
        return IsWindowVisible (ctrl_loading->hwnd);
    }

    return FALSE;
}

static void destroy_content (mMainWnd* self)
{
    mWidget* ctrl_content = (mWidget*)(_c(self)->getChild (self, IDC_CONTENT));
    if (ctrl_content) {
        DestroyWindow (ctrl_content->hwnd);
    }
}

static void reload_wifi_list (mMainWnd* self)
{
    hide_loading (self);
    destroy_content (self);

    SettingWifiListActivity* act;
    act = (SettingWifiListActivity*)GetWindowAdditionalData (self->hwnd);
    if (act->m_async_task) {
        delete act->m_async_task;
    }

    show_loading (self);
    act->m_async_task = NETWORKSERVICE->getWifiList (self->hwnd);
    act->setAutoEscape (false);
}

static BOOL mymain_onCommand (mMainWnd* self, int id, int nc, HWND hCtrl)
{
    if (id == IDC_TITLE && nc == NCSN_WIDGET_CLICKED) {
        if (!is_loading_shown (self))
            reload_wifi_list (self);
    }
    else if (id == IDC_CONTENT) {
        SettingWifiListActivity* act;
        act = (SettingWifiListActivity*)GetWindowAdditionalData (self->hwnd);
        const WifiHotspot* hotspot = act->m_async_task->m_info->hotspots->at(nc);

        if (!hotspot->is_open) {
            Intent* my_intent = Intent::newIntent ();
            my_intent->putExtra ("SSID", std::string (hotspot->ssid));
            ACTIVITYSTACK->push ("SettingWifiPasswordActivity", my_intent);
        }
        else {
            destroy_content (self);
            show_loading (self);
            NETWORKSERVICE->connectHotspot (self->hwnd, hotspot->ssid, "");
            act->setAutoEscape (false);
        }
    }

    return TRUE;
}

static LRESULT mymain_onReady (mMainWnd* self, UINT msg, DWORD wparam, DWORD lparam)
{
    SettingWifiListActivity* act;
    act = (SettingWifiListActivity*)GetWindowAdditionalData (self->hwnd);

    if (wparam == ACTIVITY_PUSHED) {
        act->m_async_task = NETWORKSERVICE->getWifiList (self->hwnd);
        act->setAutoEscape (false);
    }
    else if (wparam == ACTIVITY_POPPED) {
        WIFI_PASSWORD_INFO* res = (WIFI_PASSWORD_INFO*)lparam;

        if (res == NULL) {
            /* canceled */
            reload_wifi_list (self);
        }
        else {
            destroy_content (self);
            show_loading (self);

            NETWORKSERVICE->connectHotspot (self->hwnd, res->ssid, res->password);
            act->setAutoEscape (false);

            free (res->ssid);
            free (res->password);
            free (res);
        }
    }

    return 0;
}

static void create_wifi_listview (mContainerCtrl* ctrl_ctnr, const WIFI_HOTSPOTS_INFO* wifi_info)
{
    RECT rc = {0, 0, ACTIVITY_CONTENT_W, ACTIVITY_CONTENT_H};
    mWifiListPiece* wifiListPiece;
    mPanelPiece* panel;

    wifiListPiece = (mWifiListPiece *)NEWPIECEEX (mWifiListPiece, (DWORD)wifi_info);
    wifiListPiece->ctrl_ctnr = ctrl_ctnr;

    _c(wifiListPiece)->setRect (wifiListPiece, &rc);
    _c(wifiListPiece)->reloadData (wifiListPiece);

    panel = NEWPIECE (mPanelPiece);
    _c(panel)->addContent (panel, (mHotPiece*)wifiListPiece, 0, 0);
    _c(panel)->setRect (panel, &rc);

    _c(ctrl_ctnr)->setBody (ctrl_ctnr, (mHotPiece*)panel);

    /* set scrollbar color here */
    mHotPiece *scrollbar;
    scrollbar = wifiListPiece->m_scrollbar->piece;
    _c(scrollbar)->setProperty(scrollbar, NCSP_TRANROUND_BKCOLOR, COLOR_SCROLLBAR);
}

static mWidget* create_content (mMainWnd* self, const WIFI_HOTSPOTS_INFO* wifi_info)
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
        create_wifi_listview ((mContainerCtrl*)ctrl_ctnr, wifi_info);
    }

    return ctrl_ctnr;
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

static RES_NODE _myres_list [] = {
    /* images on status bar */
    {RES_NAME_WIFI_LOCK, RES_TYPE_IMAGE, 16, 0},
    {RES_NAME_WIFI_0,    RES_TYPE_IMAGE, 16, 0},
    {RES_NAME_WIFI_1,    RES_TYPE_IMAGE, 16, 0},
    {RES_NAME_WIFI_2,    RES_TYPE_IMAGE, 16, 0},
    {RES_NAME_WIFI_3,    RES_TYPE_IMAGE, 16, 0},
    {RES_NAME_WIFI_4,    RES_TYPE_IMAGE, 16, 0},

    /* terminator */
    {NULL}
};

static BOOL mymain_onCreate (mMainWnd* self, DWORD dwAddData)
{
    unsigned cnt;

    cnt = loadResByTag (_myres_list, 0);

    if (cnt < (TABLESIZE (_myres_list) - 1)) {
        _ERR_PRINTF ("SettingWifiListActivity: failed to load resource.\n");
        return FALSE;
    }

    if (create_loading (self) == NULL) {
        _ERR_PRINTF ("SettingWifiListActivity: failed to create loading animation.\n");
        return FALSE;
    }

    return TRUE;
}

static LRESULT mymain_onIntent (mMainWnd* self, UINT msg, DWORD wparam, DWORD lparam)
{
    Intent* intent = (Intent*)wparam;
    SettingWifiListActivity* act;
    act = (SettingWifiListActivity*)GetWindowAdditionalData (self->hwnd);

    if (strcasecmp (intent->getString ("firstBoot").c_str(), "yes") == 0) {
        act->m_firstBoot = true;
    }

    Intent::deleteIntent (intent);
    return 0;
}

static LRESULT mymain_onAsyncTaskFinished (mMainWnd* self, UINT msg, DWORD wparam, DWORD lparam)
{
    const char* mb_title = NULL;
    const char* mb_text = NULL;

    hide_loading (self);
    destroy_content (self);

    SettingWifiListActivity* act;
    act = (SettingWifiListActivity*)GetWindowAdditionalData (self->hwnd);

    if (!act->m_firstBoot)
        act->setAutoEscape (true);

    if (wparam == TASKID_NETWORK_WIFI_LIST) {
        mb_title = _("Hotspots List");

        WIFI_HOTSPOTS_INFO* info = (WIFI_HOTSPOTS_INFO*)lparam;
        switch (info->status) {
        case TS_WIFILIST_OK:
            create_content (self, info);
            break;

        case TS_WIFILIST_DISABLED:
            mb_text = _("WiFi disabled. Please turn on it in Network Settings.");
            break;

        case TS_WIFILIST_BADDATA:
            mb_text = _("Failed to scan WiFi hotspots, please try again.");
            break;

        case TS_WIFILIST_SYSERROR:
            mb_text = _("System error!");
            break;

        case TS_WIFILIST_UNKNOWN:
            mb_text = _("Unknown error. please retry.");
            break;

        default:
            break;
        }
    }
    else if (wparam == TASKID_NETWORK_CONN_WIFI) {
        WIFI_CONN_RESULT* info = (WIFI_CONN_RESULT*)lparam;

        mb_title = _("Connect WiFi");

        if (info->status == TS_CONN_WIFI_STATUS_OK) {
            switch (info->result) {
            case TS_CONN_WIFI_RES_CONNECTED:
                mb_text = NULL;
                break;

            case TS_CONN_WIFI_RES_BADPASSWD:
                mb_text = _("Password error, please retry.");
                break;

            case TS_CONN_WIFI_RES_NOTEXISTS:
                mb_text = _("Hotspot specified does not existe.");
                break;

            default:
                mb_text = _("Unknown error. please retry.");
                break;
            }
        }
        else {
            mb_text = _("System error!");
        }

        if (mb_text == NULL) {
            if (act->m_firstBoot) {
                Intent* my_intent = Intent::newIntent ();
                my_intent->putExtra ("firstBoot", std::string ("yes"));
                ACTIVITYSTACK->switchTo ("SettingCheckNetworkActivity", my_intent);
            }
            else {
                ACTIVITYSTACK->pop ();
            }
            return TRUE;
        }
    }

    /* we need return from this function asap */
    if (mb_text) {
        SendNotifyMessage (self->hwnd, MSG_USER, (WPARAM)mb_title, (LPARAM)mb_text);
    }

    return TRUE;
}

static LRESULT mymain_onUser (mMainWnd* self, UINT msg, DWORD wparam, DWORD lparam)
{
    int ret;
    SettingWifiListActivity* act;

    act = (SettingWifiListActivity*)GetWindowAdditionalData (self->hwnd);

    ret = messageScreen (self->hwnd, (const char*)wparam, (const char*)lparam, MB_RETRYCANCEL);
    if (ret == IDRETRY) {
        show_loading (self);

        if (act->m_async_task) {
            delete act->m_async_task;
        }
        act->m_async_task = NETWORKSERVICE->getWifiList (self->hwnd);
        act->setAutoEscape (false);
    }

    return 0;
}

static BOOL mymain_onClose (mMainWnd* self, int message)
{
    _MG_PRINTF ("SettingWifiListActivity: mymain_onClose called.\n");

    /* delete async task here */
    SettingWifiListActivity* act;
    act = (SettingWifiListActivity*)GetWindowAdditionalData (self->hwnd);
    if (act->m_async_task) {
        delete act->m_async_task;
        act->m_async_task = NULL;
    }

    unloadResByTag (_myres_list, 0);
    return TRUE;
}

static BOOL mymain_onCreated (struct _NCS_CREATE_NOTIFY_INFO* info, mComponent* self, DWORD special_id)
{
    mWidget* ctrl_title = (mWidget*)(_c(self)->getChild (self, IDC_TITLE));
    mHotPiece* piece_title = (mHotPiece*)(ctrl_title->body);
    _c(piece_title)->setProperty (piece_title, NCSP_LABELPIECE_LABEL, (DWORD)_("Set up WiFi"));
    ncsSetElement (ctrl_title, NCS_FGC_WINDOW, COLOR_TITLE);
    ncsSetFont (ctrl_title->hwnd, FONT_TITLE);
    return TRUE;
}

static NCS_CREATE_NOTIFY_INFO create_notify_info = {
    onCreated: mymain_onCreated,
};

static NCS_EVENT_HANDLER mymain_handlers [] = {
    {MSG_CREATE, reinterpret_cast<void*>(mymain_onCreate)},
    {MSG_USER_APP_DATA, reinterpret_cast<void*>(mymain_onIntent)},
    {MSG_ASYNC_TASK_FINISHED, reinterpret_cast<void*>(mymain_onAsyncTaskFinished)},
    {MSG_COMMAND, reinterpret_cast<void*>(mymain_onCommand)},
    {MSG_USER_APP_READY, reinterpret_cast<void*>(mymain_onReady)},
    {MSG_USER, reinterpret_cast<void*>(mymain_onUser)},
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
        x: ACTIVITY_TITLE_X,
        y: ACTIVITY_TITLE_Y,
        w: ACTIVITY_TITLE_W,
        h: ACTIVITY_TITLE_H,
        style: WS_VISIBLE | NCSS_NOTIFY,
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
    N_("WiFi Hotspots"),
    NULL,
    NULL,
    mymain_handlers,
    _ctrl_templ,
    TABLESIZE(_ctrl_templ),
    0, 0, NULL, 0, 0,
    &create_notify_info,
    0
};

SettingWifiListActivity::SettingWifiListActivity() : NCSActivity (&mymain_templ)
{
    MGNCS_INIT_CLASS (mWifiListPiece);

    m_style = STYLE_PUSH;

    m_firstBoot = false;

    m_async_task = NULL;
}

SettingWifiListActivity::~SettingWifiListActivity()
{
}

