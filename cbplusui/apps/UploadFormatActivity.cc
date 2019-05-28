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
 * @file UploadFormatActivity.cc
 * @Synopsis The upload format activity for Auto/Manula Upload Cloud apps.
 * @author Vincent Wei
 * @version 1.0
 * @date 14/06/2018
 */

#include "global.h"
#include "utilities.h"
#include "misc.h"

#include "resource.h"

#include "Activity.hh"
#include "ActivityStack.hh"
#include "NCSActivity.hh"

#include "MenuListPiece.hh"
#include "UploadFormatActivity.hh"

REGISTER_ACTIVITY(UploadFormatActivity);

MENU_ENTRY g_menu_entries_upload_format [] = {
    { (IDE_MIN + 0),   TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("JPEG"), SWITCH_ON },
    { (IDE_MIN + 1),   TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("Raw"), SWITCH_OFF },
    { (IDE_MIN + 2),   TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("Raw + JPEG"), SWITCH_OFF },
    { (IDE_MIN + 3),   TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("Inner JPEG in Raw"), SWITCH_OFF },
};

static BOOL mymain_onCommand (mMainWnd* self, int id, int nc, HWND hCtrl)
{
    if (id == IDC_CONTENT && nc < (int)(TABLESIZE (g_menu_entries_upload_format) - 1)
            && g_menu_entries_upload_format[nc].type == TYPE_MENU_ENTRY_CHECKABLE) {
        ACTIVITYSTACK->pop (nc + 1);
    }

    return TRUE;
}

static BOOL mymain_onCreate (mMainWnd* self, DWORD dwAddData)
{
    return TRUE;
}

static BOOL mymain_onClose (mMainWnd* self, int message)
{
    _MG_PRINTF ("UploadFormatActivity: mymain_onClose called.\n");

    return TRUE;
}

static BOOL mymain_onCreated (struct _NCS_CREATE_NOTIFY_INFO* info, mComponent* self, DWORD special_id)
{
    mWidget* ctrl;

    ctrl = (mWidget*)(_c(self)->getChild (self, IDC_TITLE));
    if (ctrl) {
        SetWindowText (ctrl->hwnd, _("Upload Format"));
        ncsSetElement (ctrl, NCS_FGC_WINDOW, COLOR_TITLE);
        ncsSetFont (ctrl->hwnd, FONT_TITLE);
    }

    ctrl = (mWidget*)(_c(self)->getChild (self, IDC_CONTENT));
    if (ctrl) {
        RECT rc = {0, 0, ACTIVITY_CONTENT_W, ACTIVITY_CONTENT_H};
        createMenuListView ((mContainerCtrl*)ctrl, &rc, g_menu_entries_upload_format, TABLESIZE (g_menu_entries_upload_format) - 1);
    }

    return TRUE;
}

static NCS_CREATE_NOTIFY_INFO create_notify_info = {
    onCreated: mymain_onCreated,
};

static NCS_EVENT_HANDLER mymain_handlers [] = {
    {MSG_CREATE, reinterpret_cast<void*>(mymain_onCreate)},
    {MSG_COMMAND, reinterpret_cast<void*>(mymain_onCommand)},
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
    {
        class_name: NCSCTRL_CONTAINERCTRL,
        id: IDC_CONTENT,
        x: ACTIVITY_CONTENT_X,
        y: ACTIVITY_CONTENT_Y,
        w: ACTIVITY_CONTENT_W,
        h: ACTIVITY_CONTENT_H,
        style: WS_VISIBLE | NCSS_NOTIFY,
        ex_style: WS_EX_NONE,
        caption: "",
    },
};

static NCS_MNWND_TEMPLATE mymain_templ = {
    NCSCTRL_DIALOGBOX,
    1,
    ACTIVITY_X, ACTIVITY_Y, ACTIVITY_W, ACTIVITY_H,
    WS_NONE, WS_EX_NONE,
    N_("Upload Period"),
    NULL,
    NULL,
    mymain_handlers,
    _ctrl_templ,
    TABLESIZE(_ctrl_templ),
    0, 0, NULL, 0, 0,
    &create_notify_info,
    0
};

UploadFormatActivity::UploadFormatActivity() : NCSActivity (&mymain_templ)
{
    m_style = STYLE_PUSH;
}

UploadFormatActivity::~UploadFormatActivity()
{
}

