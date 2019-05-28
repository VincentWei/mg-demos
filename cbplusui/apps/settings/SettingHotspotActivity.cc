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
 * @file SettingHotspotActivity.cc
 * @Synopsis The activity for Remote Controll Via Self-Hotspot.
 * @author Vincent Wei
 * @version 1.0
 * @date 10/09/2018
 */

#include <math.h>

#include "global.h"
#include "utilities.h"
#include "misc.h"

#include "Activity.hh"
#include "ActivityStack.hh"
#include "NCSActivity.hh"

#include "resource.h"

#include "MenuListPiece.hh"
#include "MiscService.hh"
#include "SettingHotspotActivity.hh"

REGISTER_ACTIVITY(SettingHotspotActivity);

#define PAGE_NORMAL         0
#define PAGE_ERROR          1

#define IDC_HOTSPOT_NAME    301
#define IDC_HOTSPOT_PASSWD  302
#define IDC_START_HOTSPOT   303
#define IDC_STOP_HOTSPOT    304

#define IDC_ERROR           401
#define IDC_QUIT            402

#define ICON_SIZE_W         80
#define ICON_SIZE_H         60
#define ICON_PULLUP_H       30

#define MARGIN_H            10
#define MARGIN_V            20

#define SPACING_V           10

#define TEXT_HOTSPOT_NAME_H      30
#define TEXT_HOTSPOT_PASSWD_H    30
#define TEXT_HOTSPOT_RESTART_H   40
#define TEXT_NORMAL_H       30

static BOOL mypage_onCommand (mWidget* self, int id, int nc, HWND hCtrl)
{
    HWND hmainwin = GetMainWindowHandle (hCtrl);
    SettingHotspotActivity* act = (SettingHotspotActivity*)GetWindowAdditionalData (hmainwin);

    if (nc == NCSN_WIDGET_CLICKED) {
        if (id == IDC_START_HOTSPOT) {
            EnableWindow (hCtrl, FALSE);

            act->setAutoEscape (false);
            MISCSERVICE->startHotspot (hmainwin, true);

            mWidget* ctrl;
            ctrl = (mWidget*)(_c(self)->getChild (self, IDC_HOTSPOT_NAME));
            if (ctrl) {
                SetWindowText (ctrl->hwnd, _("Starting Hotspot…"));
            }

            ctrl = (mWidget*)(_c(self)->getChild (self, IDC_HOTSPOT_PASSWD));
            if (ctrl) {
                SetWindowText (ctrl->hwnd, _("Please wait a moment."));
            }
        }
        else if (id == IDC_STOP_HOTSPOT) {
            EnableWindow (hCtrl, FALSE);

            act->setAutoEscape (false);
            MISCSERVICE->stopHotspot (hmainwin);

            mWidget* ctrl;
            ctrl = (mWidget*)(_c(self)->getChild (self, IDC_HOTSPOT_NAME));
            if (ctrl) {
                SetWindowText (ctrl->hwnd, _("Stopping Hotspot…"));
            }

            ctrl = (mWidget*)(_c(self)->getChild (self, IDC_HOTSPOT_PASSWD));
            if (ctrl) {
                SetWindowText (ctrl->hwnd, _("Please wait a moment."));
            }
        }
    }

    return TRUE;
}

static NCS_WND_TEMPLATE _normal_ctrls [] = {
    {
        class_name: NCSCTRL_STATIC,
        id: IDC_HOTSPOT_NAME,
        x: MARGIN_H,
        y: MARGIN_V,
        w: ACTIVITY_W - MARGIN_H * 2,
        h: TEXT_HOTSPOT_NAME_H,
        style: WS_VISIBLE,
        ex_style: WS_EX_NONE,
        caption: "",
    },
    {
        class_name: NCSCTRL_STATIC,
        id: IDC_HOTSPOT_PASSWD,
        x: MARGIN_H,
        y: MARGIN_V + TEXT_HOTSPOT_NAME_H,
        w: ACTIVITY_W - MARGIN_H * 2,
        h: TEXT_HOTSPOT_PASSWD_H,
        style: WS_VISIBLE,
        ex_style: WS_EX_NONE,
        caption: "",
    },
    {
        class_name: NCSCTRL_BUTTON,
        id: IDC_START_HOTSPOT,
        x: MARGIN_H,
        y: MARGIN_V * 3 + TEXT_HOTSPOT_NAME_H + TEXT_HOTSPOT_PASSWD_H,
        w: ACTIVITY_W - MARGIN_H * 2,
        h: TEXT_HOTSPOT_RESTART_H,
        style: WS_VISIBLE | WS_DISABLED,
        ex_style: WS_EX_NONE,
        caption: "",
    },
    {
        class_name: NCSCTRL_BUTTON,
        id: IDC_STOP_HOTSPOT,
        x: MARGIN_H,
        y: MARGIN_V * 3 + TEXT_HOTSPOT_NAME_H + TEXT_HOTSPOT_PASSWD_H,
        w: ACTIVITY_W - MARGIN_H * 2,
        h: TEXT_HOTSPOT_RESTART_H,
        style: WS_VISIBLE,
        ex_style: WS_EX_NONE,
        caption: "",
    },
};

static void update_ctrls(mPage* page, const HOTSPOT_INFO* info)
{
    mWidget* ctrl;

    if (info && info->enabled) {
        char buff[256];

        ctrl = (mWidget*)(_c(page)->getChild (page, IDC_HOTSPOT_NAME));
        if (ctrl) {
            snprintf (buff, 255, _("WiFi: %s"), info->ssid);
            SetWindowText (ctrl->hwnd, buff);
        }

        ctrl = (mWidget*)(_c(page)->getChild (page, IDC_HOTSPOT_PASSWD));
        if (ctrl) {
            snprintf (buff, 255, _("Password: %s"), info->passwd);
            SetWindowText (ctrl->hwnd, buff);
        }

        ctrl = (mWidget*)(_c(page)->getChild (page, IDC_START_HOTSPOT));
        if (ctrl) {
            ShowWindow (ctrl->hwnd, SW_HIDE);
        }
        ctrl = (mWidget*)(_c(page)->getChild (page, IDC_STOP_HOTSPOT));
        if (ctrl) {
            SetWindowText (ctrl->hwnd, _("Stop Hotspot"));
            EnableWindow (ctrl->hwnd, TRUE);
            ShowWindow (ctrl->hwnd, SW_SHOWNORMAL);
        }
    }
    else if (info == NULL) {
        ctrl = (mWidget*)(_c(page)->getChild (page, IDC_HOTSPOT_NAME));
        if (ctrl) {
            SetWindowText (ctrl->hwnd, _("Failed to create WiFi hotspot!"));
        }

        ctrl = (mWidget*)(_c(page)->getChild (page, IDC_HOTSPOT_PASSWD));
        if (ctrl) {
            SetWindowText (ctrl->hwnd, _("Please try again."));
        }

        ctrl = (mWidget*)(_c(page)->getChild (page, IDC_STOP_HOTSPOT));
        if (ctrl) {
            ShowWindow (ctrl->hwnd, SW_HIDE);
        }
        ctrl = (mWidget*)(_c(page)->getChild (page, IDC_START_HOTSPOT));
        if (ctrl) {
            SetWindowText (ctrl->hwnd, _("Start Hotspot"));
            EnableWindow (ctrl->hwnd, TRUE);
            ShowWindow (ctrl->hwnd, SW_SHOWNORMAL);
        }
    }
    else if (!info->enabled) {
        ctrl = (mWidget*)(_c(page)->getChild (page, IDC_HOTSPOT_NAME));
        if (ctrl) {
            SetWindowText (ctrl->hwnd, _("Hotspot closed."));
        }

        ctrl = (mWidget*)(_c(page)->getChild (page, IDC_HOTSPOT_PASSWD));
        if (ctrl) {
            SetWindowText (ctrl->hwnd, _("You can try to start if need."));
        }

        ctrl = (mWidget*)(_c(page)->getChild (page, IDC_STOP_HOTSPOT));
        if (ctrl) {
            ShowWindow (ctrl->hwnd, SW_HIDE);
        }
        ctrl = (mWidget*)(_c(page)->getChild (page, IDC_START_HOTSPOT));
        if (ctrl) {
            SetWindowText (ctrl->hwnd, _("Start Hotspot"));
            EnableWindow (ctrl->hwnd, TRUE);
            ShowWindow (ctrl->hwnd, SW_SHOWNORMAL);
        }
    }
}

static void init_page_normal (mPage* self)
{
    for (size_t i = 0; i < TABLESIZE (_normal_ctrls); i++) {
        ncsCreateWindowIndirect (_normal_ctrls + i, self->hPanel);
    }

    HOTSPOT_INFO hotspot_info;
    if (MISCSERVICE->getHotspotStatus(&hotspot_info)) {
        update_ctrls(self, &hotspot_info);
    }
    else {
        update_ctrls(self, NULL);
    }
}

static NCS_PROP_ENTRY _props_message [] = {
    { NCSP_STATIC_AUTOWRAP, 1 },
    { NCSP_STATIC_ALIGN, NCS_ALIGN_LEFT },
    { NCSP_STATIC_VALIGN, NCS_VALIGN_TOP },
    { -1, 0 },
};

static NCS_WND_TEMPLATE _error_ctrls [] = {
    {
        class_name: NCSCTRL_STATIC,
        id: IDC_ERROR,
        x: MARGIN_H,
        y: MARGIN_V,
        w: ACTIVITY_W - MARGIN_H * 2,
        h: TEXT_NORMAL_H * 2,
        style: WS_VISIBLE,
        ex_style: WS_EX_NONE,
        caption: "Error occured!",
        props: _props_message,
    },
    {
        class_name: NCSCTRL_BUTTON,
        id: IDC_QUIT,
        x: 50,
        y: TEXT_NORMAL_H * 3,
        w: ACTIVITY_W - 50 - 50,
        h: ACTIVITY_CONTENT_ITEM_H,
        style: WS_VISIBLE | NCSS_NOTIFY,
        ex_style: WS_EX_NONE,
        caption: "",
    },
};

static void init_page_error (mPage* self)
{
    for (size_t i = 0; i < TABLESIZE (_error_ctrls); i++) {
        ncsCreateWindowIndirect (_error_ctrls + i, self->hPanel);
    }

    mWidget* ctrl;
    ctrl = (mWidget*)(_c(self)->getChild (self, IDC_ERROR));
    if (ctrl) {
    }

    ctrl = (mWidget*)(_c(self)->getChild (self, IDC_QUIT));
    if (ctrl) {
        SetWindowText (ctrl->hwnd, _("Quit"));
    }
}

static void mypage_onInitPage (mPage* self, DWORD add_data)
{
    switch (add_data) {
        case PAGE_NORMAL:
            init_page_normal (self);
            break;

        case PAGE_ERROR:
            init_page_error (self);
            break;

        default:
            break;
    }
}

static int mypage_onShowPage (mPage* self, HWND focusChild, int show_cmd)
{
    return 1;
}

static int mypage_onSheetCmd (mPage* self, DWORD wParam, DWORD lParam)
{
    return 0;
}

static NCS_EVENT_HANDLER mypage_handlers[] = {
    { MSG_INITPAGE, reinterpret_cast<void*>(mypage_onInitPage) },
    { MSG_SHOWPAGE, reinterpret_cast<void*>(mypage_onShowPage) },
    { MSG_COMMAND, reinterpret_cast<void*>(mypage_onCommand) },
    { MSG_SHEETCMD, reinterpret_cast<void*>(mypage_onSheetCmd) },
    { 0 , NULL }
};

static BOOL init_content (mComponent* self)
{
    /* create property pages */
    DLGTEMPLATE page_info = {
        WS_NONE,
        WS_EX_NONE,
        0, 0, 0, 0,
        "",
        0, 0,
        0, NULL,
        0
    };

    mPropSheet *propsheet = (mPropSheet*)(_c(self)->getChild (self, IDC_CONTENT));
    if (propsheet == NULL) {
        _ERR_PRINTF ("SettingHotspotActivity: failed to get content (property sheet) control.\n");
        return FALSE;
    }

    mPage* page;

    page_info.caption = "Normal";
    page_info.dwAddData = PAGE_NORMAL;
    page = _c(propsheet)->addPage (propsheet, &page_info, mypage_handlers);
    SetWindowBkColor (((mWidget*)page)->hwnd, PIXEL_black);

    page_info.caption = "Error";
    page_info.dwAddData = PAGE_ERROR;
    page = _c(propsheet)->addPage (propsheet, &page_info, mypage_handlers);
    SetWindowBkColor (((mWidget*)page)->hwnd, PIXEL_black);

    return TRUE;
}

static RES_NODE _myres_list [] = {
    {COMMON_IMG_CAMERA_USB,  RES_TYPE_IMAGE, 16, 0},
    {COMMON_IMG_CAMERA_WLAN, RES_TYPE_IMAGE, 16, 0},
    {NULL},
};

static BOOL load_res (void)
{
    unsigned u;

    u = loadResByTag (_myres_list, 0);
    if (u < (TABLESIZE (_myres_list) - 1)) {
        _ERR_PRINTF ("SettingHotspotActivity: failed to load resource.\n");
        return FALSE;
    }

    return TRUE;
}

static void unload_res (void)
{
    unloadResByTag (_myres_list, 0);
}

static BOOL mymain_onCreated (struct _NCS_CREATE_NOTIFY_INFO* info, mComponent* self, DWORD special_id)
{
    if (!load_res ()) {
        return FALSE;
    }

    mWidget* ctrl;

    ctrl = (mWidget*)(_c(self)->getChild (self, IDC_TITLE));
    ncsSetElement (ctrl, NCS_FGC_WINDOW, COLOR_TITLE);
    ncsSetFont (ctrl->hwnd, FONT_TITLE);
    SetWindowText (ctrl->hwnd, _("WiFi Hotspot"));

    init_content (self);
    return TRUE;
}

static LRESULT mymain_onIntent (mMainWnd* self, UINT msg, DWORD wparam, DWORD lparam)
{
    Intent* intent = (Intent*)wparam;
    if (intent)
        Intent::deleteIntent (intent);
    return 0;
}

static BOOL mymain_onClose (mMainWnd* self, int message)
{
    _MG_PRINTF ("SettingHotspotActivity: mymain_onClose called.\n");

    unload_res ();
    return TRUE;
}

static LRESULT mymain_onStateChanged (mMainWnd* self, UINT msg, DWORD wparam, DWORD lparam)
{
    switch (msg) {
    case MSG_USBDEV_CHANGED: {
        break;
    }

    default:
        break;
    }

    return 0;
}

static BOOL mymain_onCommand (mMainWnd* self, int id, int nc, HWND hCtrl)
{
#if 0
    mPropSheet *propsheet = (mPropSheet*)ncsGetChildObj (self->hwnd, IDC_CONTENT);
    int idx_page = (int)_c(propsheet)->getProperty (propsheet, NCSP_PRPSHT_ACTIVEPAGEIDX);
    _c(propsheet)->setProperty (propsheet, NCSP_PRPSHT_ACTIVEPAGEIDX, PAGE_ERROR - idx_page);

    SettingHotspotActivity* act;
    act = (SettingHotspotActivity*)GetWindowAdditionalData (self->hwnd);

    if (nc == NCSN_WIDGET_CLICKED && id == IDC_START_HOTSPOT) {
        mPropSheet *propsheet = (mPropSheet*)ncsGetChildObj (self->hwnd, IDC_CONTENT);
        mPage* page = (mPage*)(_c(propsheet)->getPageByIndex (propsheet, PAGE_NORMAL));
        mWidget* ctrl;

    }
#endif

    return TRUE;
}

static LRESULT mymain_onAsyncTaskFinished (mMainWnd* self, UINT msg, DWORD wparam, DWORD lparam)
{
    SettingHotspotActivity* act;
    act = (SettingHotspotActivity*)GetWindowAdditionalData (self->hwnd);

    mPropSheet *propsheet = (mPropSheet*)ncsGetChildObj (self->hwnd, IDC_CONTENT);
    mPage* page = (mPage*)(_c(propsheet)->getPageByIndex (propsheet, PAGE_NORMAL));

    if (wparam == TASKID_MISC_START_HOTSPOT) {
        HOTSPOT_INFO* info = (HOTSPOT_INFO*)lparam;

        update_ctrls(page, info);
        act->setAutoEscape (true);
    }
    else if (wparam == TASKID_MISC_STOP_HOTSPOT) {
        HOTSPOT_INFO* info = (HOTSPOT_INFO*)lparam;

        update_ctrls(page, info);
        act->setAutoEscape (true);
    }

    return TRUE;
}

static NCS_EVENT_HANDLER mymain_handlers [] = {
    {MSG_USER_APP_DATA, reinterpret_cast<void*>(mymain_onIntent)},
    {MSG_ASYNC_TASK_FINISHED, reinterpret_cast<void*>(mymain_onAsyncTaskFinished)},
    {MSG_COMMAND, reinterpret_cast<void*>(mymain_onCommand)},
    {MSG_USBDEV_CHANGED, reinterpret_cast<void*>(mymain_onStateChanged)},
    {MSG_WLANDEV_CHANGED, reinterpret_cast<void*>(mymain_onStateChanged)},
    {MSG_CLOSE, reinterpret_cast<void*>(mymain_onClose)},
    {0, NULL}
};

static NCS_CREATE_NOTIFY_INFO create_notify_info = {
    onCreated: mymain_onCreated,
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
    {
        class_name: NCSCTRL_PROPSHEET,
        id: IDC_CONTENT,
        x: ACTIVITY_CONTENT_X - 1,
        y: ACTIVITY_CONTENT_Y - 1,
        w: ACTIVITY_CONTENT_W + 2,
        h: ACTIVITY_CONTENT_H + 40,
        style: WS_VISIBLE | NCSS_PRPSHT_BOTTOM,
        ex_style: WS_EX_NONE,
        caption: "",
    },
};

static NCS_MNWND_TEMPLATE mymain_templ = {
    NCSCTRL_DIALOGBOX,
    1,
    ACTIVITY_X, ACTIVITY_Y, ACTIVITY_W, ACTIVITY_H,
    WS_NONE, WS_EX_NONE,
    N_("WiFi Hotspot"),
    NULL,
    NULL,
    mymain_handlers,
    _ctrl_templ,
    TABLESIZE(_ctrl_templ),
    0, 0, NULL, 0, 0,
    &create_notify_info,
    0
};

#if 0
int SettingHotspotActivity::onCancel ()
{
    if (messageScreen (m_hwnd, _("Please Confirm"), _("Exit the current work mode?"),
            MB_OKCANCEL | MS_AUTOCANCEL) == IDOK) {
        MISCSERVICE->stopHotspot (hwnd());
        showToastWindow (_("Please wait a moment."), _("Stopping WiFi hotspot…"), COLOR_TOAST_WARNING, FALSE);
        return 0;
    }

    return 1;
}
#endif

SettingHotspotActivity::SettingHotspotActivity () : NCSActivity (&mymain_templ, true)
{
    m_style = STYLE_ZOOM;
}

