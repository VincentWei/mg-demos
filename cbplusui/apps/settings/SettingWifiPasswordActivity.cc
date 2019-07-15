///////////////////////////////////////////////////////////////////////////////
//
//                          IMPORTANT NOTICE
//
// The following open source license statement does not apply to any
// entity in the Exception List published by FMSoft.
//
// For more information, please visit:
//
// https://www.fmsoft.cn/exception-list
//
//////////////////////////////////////////////////////////////////////////////
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
 * @file SettingWifiPasswordActivity.cc
 * @Synopsis The activity for WiFi password.
 * @author Vincent Wei
 * @version 1.0
 * @date 23/05/2018
 */

#include "global.h"
#include "utilities.h"
#include "misc.h"

#include "Activity.hh"
#include "ActivityStack.hh"
#include "NCSActivity.hh"

#define _HAVE_SUBTITLE
#include "resource.h"

#include "SettingWifiPasswordActivity.hh"

#define IDC_WIFI        101

REGISTER_ACTIVITY(SettingWifiPasswordActivity);

static BOOL mymain_onCreate (mMainWnd* self, DWORD dwAddData)
{
    return TRUE;
}

static LRESULT mymain_onIntent (mMainWnd* self, UINT msg, DWORD wparam, DWORD lparam)
{
    Intent* intent = (Intent*)wparam;
    const char* ssid = intent->getString ("SSID").c_str();

    mWidget* ctrl = (mWidget*)(_c(self)->getChild (self, IDC_SUBTITLE));
    SetWindowText (ctrl->hwnd, ssid);

    Intent::deleteIntent (intent);
    return 0;
}

#define RES_WIFI_LOGO         "images/select-wifi/wifi-logo.png"
static RES_NODE _myres_list [] = {
    {RES_WIFI_LOGO,       RES_TYPE_IMAGE, 16, 0},
    {NULL}
};

static BOOL mymain_onClose (mWidget* self, int message)
{
    _MG_PRINTF ("SettingWifiPasswordActivity: mymain_onClose called.\n");
    unloadResByTag (_myres_list, 0);

    //DestroyMainWindow (self->hwnd);
    return TRUE;
}

static BOOL mymain_onCreated (struct _NCS_CREATE_NOTIFY_INFO* info, mComponent* self, DWORD special_id)
{
    unsigned cnt = loadResByTag (_myres_list, 0);

    if (cnt < (TABLESIZE (_myres_list) - 1)) {
		_ERR_PRINTF ("SettingWifiPasswordActivity: failed to load resource.\n");
        return FALSE;
    }

    mWidget* ctrl;

    ctrl = (mWidget*)(_c(self)->getChild (self, IDC_TITLE));
    ncsSetElement (ctrl, NCS_FGC_WINDOW, COLOR_TITLE);
    ncsSetFont (ctrl->hwnd, FONT_TITLE);
    SetWindowText (ctrl->hwnd, _("Input Password"));

    ctrl = (mWidget*)(_c(self)->getChild (self, IDC_SUBTITLE));
    _c(ctrl)->setProperty (ctrl, NCSP_STATIC_VALIGN, (DWORD)NCS_VALIGN_TOP);
    ncsSetElement (ctrl, NCS_FGC_WINDOW, COLOR_SUBTITLE);
    ncsSetFont (ctrl->hwnd, FONT_SUBTITLE);

    ctrl = (mWidget*)(_c(self)->getChild (self, IDC_WIFI));
    _c(ctrl)->setProperty (ctrl, NCSP_IMAGE_IMAGE, (DWORD)GetResource (Str2Key (RES_WIFI_LOGO)));

    ctrl = (mWidget*)(_c(self)->getChild (self, IDC_CONTENT));
    SetWindowText (ctrl->hwnd, _("Tap WiFi logo to input password"));

    ctrl = (mWidget*)(_c(self)->getChild (self, IDC_LEFT_BUTTON));
    //_c(ctrl)->setProperty (ctrl, NCSP_STATIC_ALIGN, (DWORD)NCS_ALIGN_LEFT);
    //ncsSetElement (ctrl, NCS_FGC_WINDOW, COLOR_TEXT_BUTTON);
    //ncsSetFont (ctrl->hwnd, GLOBAL_FONT_SM);
    SetWindowText (ctrl->hwnd, _("Cancel"));

    ctrl = (mWidget*)(_c(self)->getChild (self, IDC_RIGHT_BUTTON));
    //_c(ctrl)->setProperty (ctrl, NCSP_STATIC_ALIGN, (DWORD)NCS_ALIGN_RIGHT);
    //ncsSetElement (ctrl, NCS_FGC_WINDOW, COLOR_TEXT_BUTTON);
    //ncsSetFont (ctrl->hwnd, GLOBAL_FONT_SM);
    SetWindowText (ctrl->hwnd, _("Join"));

    return TRUE;
}

static BOOL mymain_onCommand (mMainWnd* self, int id, int nc, HWND hCtrl)
{
    if (nc == NCSN_WIDGET_CLICKED) {
        switch (id) {
        case IDC_WIFI:
        case IDC_CONTENT: {
            char buff [256];
            memset(buff, 0, 256);
            statusBar_enableScreenLock(FALSE);
            softKeyboard (self->hwnd, buff, 8, 255);
            statusBar_enableScreenLock(TRUE);
            if (strlen (buff) > 0) {
                mWidget* ctrl = (mWidget*)(_c(self)->getChild (self, IDC_CONTENT));
                SetWindowText (ctrl->hwnd, buff);

                ctrl = (mWidget*)(_c(self)->getChild (self, IDC_RIGHT_BUTTON));
                EnableWindow (ctrl->hwnd, TRUE);
            }
            break;
        }

        case IDC_LEFT_BUTTON: {
            ACTIVITYSTACK->pop();
            break;
        }
        case IDC_RIGHT_BUTTON: {
            /* the caller is responsible to free the info */
            WIFI_PASSWORD_INFO* info = (WIFI_PASSWORD_INFO*)calloc (sizeof (WIFI_PASSWORD_INFO), 1);

            mWidget* ctrl = (mWidget*)(_c(self)->getChild (self, IDC_SUBTITLE));
            info->ssid = strdup (GetWindowCaption (ctrl->hwnd));

            ctrl = (mWidget*)(_c(self)->getChild (self, IDC_CONTENT));
            info->password = strdup (GetWindowCaption (ctrl->hwnd));
            ACTIVITYSTACK->pop ((DWORD)info);
            break;
        }
        default:
            break;
        }
    }

    return TRUE;
}

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
        style: WS_VISIBLE,
        ex_style: WS_EX_NONE,
        caption: "",
    },
    {
        class_name: NCSCTRL_STATIC,
        id: IDC_SUBTITLE,
        x: ACTIVITY_SUBTITLE_X,
        y: ACTIVITY_SUBTITLE_Y - 3,
        w: ACTIVITY_SUBTITLE_W,
        h: ACTIVITY_SUBTITLE_H,
        style: WS_VISIBLE,
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
        class_name: NCSCTRL_IMAGE,
        id: IDC_WIFI,
        x: ACTIVITY_CONTENT_X + ACTIVITY_CONTENT_MARGIN_H,
        y: ACTIVITY_CONTENT_Y + ACTIVITY_CONTENT_MARGIN_V,
        w: ACTIVITY_CONTENT_ITEM_W - ACTIVITY_CONTENT_MARGIN_H - ACTIVITY_CONTENT_MARGIN_H,
        h: ACTIVITY_CONTENT_ITEM_H * 2,
        style: WS_VISIBLE | NCSS_NOTIFY,
        ex_style: WS_EX_NONE,
        caption: "",
    },
    {
        class_name: NCSCTRL_STATIC,
        id: IDC_CONTENT,
        x: ACTIVITY_CONTENT_X + ACTIVITY_CONTENT_MARGIN_H,
        y: ACTIVITY_LEFT_BUTTON_Y - ACTIVITY_CONTENT_ITEM_H - ACTIVITY_CONTENT_MARGIN_V,
        w: ACTIVITY_CONTENT_ITEM_W - ACTIVITY_CONTENT_MARGIN_H - ACTIVITY_CONTENT_MARGIN_H,
        h: ACTIVITY_CONTENT_ITEM_H,
        style: WS_VISIBLE | NCSS_NOTIFY,
        ex_style: WS_EX_NONE,
        caption: "",
    },
    {
        class_name: NCSCTRL_BUTTON,
        id: IDC_LEFT_BUTTON,
        x: ACTIVITY_LEFT_BUTTON_X,
        y: ACTIVITY_LEFT_BUTTON_Y,
        w: ACTIVITY_LEFT_BUTTON_W,
        h: ACTIVITY_LEFT_BUTTON_H,
        style: WS_VISIBLE | NCSS_NOTIFY,
        ex_style: WS_EX_NONE,
        caption: "",
    },
    {
        class_name: NCSCTRL_BUTTON,
        id: IDC_RIGHT_BUTTON,
        x: ACTIVITY_RIGHT_BUTTON_X,
        y: ACTIVITY_RIGHT_BUTTON_Y,
        w: ACTIVITY_RIGHT_BUTTON_W,
        h: ACTIVITY_RIGHT_BUTTON_H,
        style: WS_VISIBLE | WS_DISABLED | NCSS_NOTIFY,
        ex_style: WS_EX_NONE,
        caption: "",
    },
};

static NCS_EVENT_HANDLER mymain_handlers [] = {
    {MSG_CREATE, reinterpret_cast<void*>(mymain_onCreate)},
    {MSG_USER_APP_DATA, reinterpret_cast<void*>(mymain_onIntent)},
    {MSG_COMMAND, reinterpret_cast<void*>(mymain_onCommand)},
    {MSG_CLOSE, reinterpret_cast<void*>(mymain_onClose)},
    {0, NULL}
};

static NCS_MNWND_TEMPLATE mymain_templ = {
    NCSCTRL_DIALOGBOX,
    1,
    ACTIVITY_X, ACTIVITY_Y, ACTIVITY_W, ACTIVITY_H,
    WS_NONE, WS_EX_NONE,
    N_("WiFi Pasword"),
    NULL,
    NULL,
    mymain_handlers,
    _ctrl_templ,
    TABLESIZE(_ctrl_templ),
    0, 0, NULL, 0, 0,
    &create_notify_info,
    0
};

SettingWifiPasswordActivity::SettingWifiPasswordActivity () :
    NCSActivity (&mymain_templ)
{
    m_style = STYLE_PUSH;
}

