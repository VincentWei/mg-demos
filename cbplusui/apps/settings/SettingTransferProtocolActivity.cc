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
 * @file SettingTransferProtocolActivity.cc
 * @Synopsis The Time activity of Setting app.
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
#include "SettingTransferProtocolActivity.hh"
#include "MenuListPiece.hh"

REGISTER_ACTIVITY(SettingTransferProtocolActivity);

#define IDE_TRANSPROTO_JOOPIC        (IDE_MIN + 0)
#define IDE_TRANSPROTO_FTP           (IDE_MIN + 1)

static MENU_ENTRY _menu_entries [] = {
    { IDE_TRANSPROTO_JOOPIC,    TYPE_MENU_ENTRY_CHECKABLE, },
    { IDE_TRANSPROTO_FTP,       TYPE_MENU_ENTRY_CHECKABLE, },
};

static mMenuListPiece* _listpiece;

static BOOL mymain_onCommand (mMainWnd* self, int id, int nc, HWND hCtrl)
{
    if (id == IDC_CONTENT) {
        switch (nc + IDE_MIN) {
        case IDE_TRANSPROTO_JOOPIC:
            SYSCFGSERVICE->setTransferMode(SYS_TRANSFER_PROTOCOL_JOOPIC);
            ACTIVITYSTACK->pop ();
            break;

        case IDE_TRANSPROTO_FTP:
            SYSCFGSERVICE->setTransferMode(SYS_TRANSFER_PROTOCOL_FTP);
            ACTIVITYSTACK->pop ();
            break;

        default:
            break;
        }
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

        _menu_entries [IDE_TRANSPROTO_JOOPIC - IDE_MIN].name = _("Joopic Cloud");
        _menu_entries [IDE_TRANSPROTO_FTP - IDE_MIN].name = _("FTP");

        _menu_entries [IDE_TRANSPROTO_JOOPIC - IDE_MIN].value = SWITCH_OFF;
        _menu_entries [IDE_TRANSPROTO_FTP - IDE_MIN].value = SWITCH_OFF;
        if (SYSCFGSERVICE->getTransferProtocol() == SYS_TRANSFER_PROTOCOL_JOOPIC)
            _menu_entries [IDE_TRANSPROTO_JOOPIC - IDE_MIN].value = SWITCH_ON;
        else
            _menu_entries [IDE_TRANSPROTO_FTP - IDE_MIN].value = SWITCH_ON;

        _listpiece = createMenuListView ((mContainerCtrl*)ctrl_ctnr, &rc, _menu_entries, TABLESIZE (_menu_entries));
    }

    return TRUE;
}

static BOOL mymain_onClose (mMainWnd* self, int message)
{
    _MG_PRINTF ("SettingTransferProtocolActivity: mymain_onClose called.\n");

    return TRUE;
}

static BOOL mymain_onCreated (struct _NCS_CREATE_NOTIFY_INFO* info, mComponent* self, DWORD special_id)
{
    mWidget* ctrl_title = (mWidget*)(_c(self)->getChild (self, IDC_TITLE));
    mHotPiece* piece_title = (mHotPiece*)(ctrl_title->body);
    _c(piece_title)->setProperty (piece_title, NCSP_LABELPIECE_LABEL, (DWORD)_("Transfer Protocol"));
    ncsSetElement (ctrl_title, NCS_FGC_WINDOW, COLOR_TITLE);
    ncsSetFont (ctrl_title->hwnd, FONT_TITLE);
    return TRUE;
}

static LRESULT mymain_onIntent (mMainWnd* self, UINT msg, DWORD wparam, DWORD lparam)
{
    Intent* intent = (Intent*)wparam;
    if (intent)
        Intent::deleteIntent (intent);
    return 0;
}

static NCS_CREATE_NOTIFY_INFO create_notify_info = {
    onCreated: mymain_onCreated,
};

static NCS_EVENT_HANDLER mymain_handlers [] = {
    {MSG_CREATE, reinterpret_cast<void*>(mymain_onCreate)},
    {MSG_USER_APP_DATA, reinterpret_cast<void*>(mymain_onIntent)},
    {MSG_COMMAND, reinterpret_cast<void*>(mymain_onCommand)},
    {MSG_CLOSE, reinterpret_cast<void*>(mymain_onClose)},
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
    N_("Transfer Protocol"),
    NULL,
    NULL,
    mymain_handlers,
    _ctrl_templ,
    TABLESIZE(_ctrl_templ),
    0, 0, NULL, 0, 0,
    &create_notify_info,
    0
};

SettingTransferProtocolActivity::SettingTransferProtocolActivity() : NCSActivity (&mymain_templ)
{
    m_style = STYLE_PUSH;
}

SettingTransferProtocolActivity::~SettingTransferProtocolActivity()
{
}

