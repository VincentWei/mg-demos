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
 * @file SettingAboutActivity.cc
 * @Synopsis The Wifi activity of Setting app.
 * @author Vincent Wei
 * @version 1.0
 * @date 12/06/2018
 */

#include "global.h"
#include "utilities.h"
#include "misc.h"

#include "resource.h"

#include "Activity.hh"
#include "ActivityStack.hh"
#include "NCSActivity.hh"

#include "SysConfService.hh"
#include "NetworkService.hh"
#include "SettingAboutActivity.hh"
#include "MenuListPiece.hh"

REGISTER_ACTIVITY(SettingAboutActivity);

#ifndef OEM_TPP
    #define IDE_USER                    (IDE_MIN + 0)
    #define IDE_DEVNAME                 (IDE_MIN + 1)
    #define IDE_MODEL                   (IDE_MIN + 2)
    #define IDE_SERIALNO                (IDE_MIN + 3)
    #define IDE_VERSION                 (IDE_MIN + 4)
    #define IDE_COPYRIGHT               (IDE_MIN + 5)
#else
    #define IDE_DEVNAME                 (IDE_MIN + 0)
    #define IDE_MODEL                   (IDE_MIN + 1)
    #define IDE_SERIALNO                (IDE_MIN + 2)
    #define IDE_VERSION                 (IDE_MIN + 3)
#endif

static MENU_ENTRY _menu_entries [] = {
#ifndef OEM_TPP
    { IDE_USER,         TYPE_MENU_ENTRY_ACT, },
    { IDE_DEVNAME,      TYPE_MENU_ENTRY_ACT, },
    { IDE_MODEL,        TYPE_MENU_ENTRY_ACT, },
    { IDE_SERIALNO,     TYPE_MENU_ENTRY_ACT, },
    { IDE_VERSION,      TYPE_MENU_ENTRY_ACT, },
    { IDE_COPYRIGHT,    TYPE_MENU_ENTRY_ACT, },
#else
    { IDE_DEVNAME,      TYPE_MENU_ENTRY_ACT, },
    { IDE_MODEL,        TYPE_MENU_ENTRY_ACT, },
    { IDE_SERIALNO,     TYPE_MENU_ENTRY_ACT, },
    { IDE_VERSION,      TYPE_MENU_ENTRY_ACT, },
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

static BOOL mymain_onCreate (mMainWnd* self, DWORD dwAddData)
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

#ifndef OEM_TPP
        const char* user_nickname = SYSCFGSERVICE->getBoundUserNickname ();
        _menu_entries [IDE_USER - IDE_MIN].name = _("User");
        _menu_entries [IDE_USER - IDE_MIN].value = user_nickname?user_nickname:_("(Not Paired)");
#endif

        _menu_entries [IDE_DEVNAME - IDE_MIN].name = _("Device Name");
        _menu_entries [IDE_DEVNAME - IDE_MIN].value = SYSCFGSERVICE->getDeviceName ();
        _menu_entries [IDE_MODEL - IDE_MIN].name = _("Model");
        _menu_entries [IDE_MODEL - IDE_MIN].value = SYSCFGSERVICE->getDeviceModel ();
        _menu_entries [IDE_SERIALNO - IDE_MIN].name = _("Serial Number");
        _menu_entries [IDE_SERIALNO - IDE_MIN].value = SYSCFGSERVICE->getSerialNumber ();
        _menu_entries [IDE_VERSION - IDE_MIN].name = _("Version");
        _menu_entries [IDE_VERSION - IDE_MIN].value = SYSCFGSERVICE->getVersionNumber ();
#ifndef OEM_TPP
        _menu_entries [IDE_COPYRIGHT - IDE_MIN].name = _("Copyright");
        _menu_entries [IDE_COPYRIGHT - IDE_MIN].value = _("Joobot Inc.");
#endif
        createMenuListView ((mContainerCtrl*)ctrl_ctnr, &rc, _menu_entries, TABLESIZE (_menu_entries));
    }

    return TRUE;
}

static BOOL mymain_onClose (mMainWnd* self, int message)
{
    _MG_PRINTF ("SettingAboutActivity: mymain_onClose called.\n");

    return TRUE;
}

static BOOL mymain_onCreated (struct _NCS_CREATE_NOTIFY_INFO* info, mComponent* self, DWORD special_id)
{
    mWidget* ctrl_title = (mWidget*)(_c(self)->getChild (self, IDC_TITLE));
    mHotPiece* piece_title = (mHotPiece*)(ctrl_title->body);
    _c(piece_title)->setProperty (piece_title, NCSP_LABELPIECE_LABEL, (DWORD)_("About Device"));
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
    N_("About"),
    NULL,
    NULL,
    mymain_handlers,
    _ctrl_templ,
    TABLESIZE(_ctrl_templ),
    0, 0, NULL, 0, 0,
    &create_notify_info,
    0
};

SettingAboutActivity::SettingAboutActivity() : NCSActivity (&mymain_templ)
{
    m_style = STYLE_PUSH;
}

SettingAboutActivity::~SettingAboutActivity()
{
}

