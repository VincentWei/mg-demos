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
 * @file AUCSettingsActivity.cc
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

#include "AUCSettingsActivity.hh"
#include "MenuListPiece.hh"

#include "SysConfService.hh"

REGISTER_ACTIVITY(AUCSettingsActivity);

#define IDC_CLOSE 100

#define IDE_UPLOAD_FORMAT           (IDE_MIN + 0)
#define IDE_DESC_UPLOAD_FORMAT      (IDE_MIN + 1)
#define IDE_AUTO_DELETE             (IDE_MIN + 2)
#define IDE_DESC_AUTO_DELETE        (IDE_MIN + 3)
#define IDE_WRITE_LOCATION          (IDE_MIN + 4)
#define IDE_DESC_WRITE_LOCATION     (IDE_MIN + 5)

extern MENU_ENTRY g_menu_entries_upload_format [];

static MENU_ENTRY _menu_entries [] = {
    { (IDE_MIN + 0), TYPE_MENU_ENTRY_ACT,  "UploadFormatActivity", N_("Upload Format"), "JPEG" },
    { (IDE_MIN + 1), TYPE_MENU_ENTRY_DESC, NULL, N_("The file in the specified format will be uploaded to cloud.") },
    { (IDE_MIN + 2), TYPE_MENU_ENTRY_SWITCH, NULL, N_("Auto Delete"), SWITCH_ON },
    { (IDE_MIN + 3), TYPE_MENU_ENTRY_DESC, NULL, N_("The photo will be deleted automatically from your camera after the photo were uploaded cloud.") },
    { (IDE_MIN + 4), TYPE_MENU_ENTRY_SWITCH, NULL, N_("Write Location to Photos"), SWITCH_ON },
    { (IDE_MIN + 5), TYPE_MENU_ENTRY_DESC, NULL, N_("Write the GPS location data to the JPEG files before uploading the photos.") },
};

static mMenuListPiece* _listpiece;

static void _init_menu_entry_values (bool reload)
{
    const UPLOAD_OPTIONS* upload_opts = SYSCFGSERVICE->getAUCOptions (true);
    _menu_entries [IDE_UPLOAD_FORMAT - IDE_MIN].value = _(g_menu_entries_upload_format[upload_opts->filter - 1].name);

    if (upload_opts->autoDel)
        _menu_entries [IDE_AUTO_DELETE - IDE_MIN].value = SWITCH_ON;
    else
        _menu_entries [IDE_AUTO_DELETE - IDE_MIN].value = SWITCH_OFF;

    if (upload_opts->writeLoc)
        _menu_entries [IDE_WRITE_LOCATION - IDE_MIN].value = SWITCH_ON;
    else
        _menu_entries [IDE_WRITE_LOCATION - IDE_MIN].value = SWITCH_OFF;

    if (reload)
        _c(_listpiece)->reloadData (_listpiece);
}

static BOOL mymain_onCommand (mMainWnd* self, int id, int nc, HWND hCtrl)
{
    if (id == IDC_CONTENT && nc == 0) {
        const UPLOAD_OPTIONS* upload_opts = SYSCFGSERVICE->getAUCOptions (true);
        for (int i = 0; i < 4; i++) {
            if ((i + 1) == upload_opts->filter)
                g_menu_entries_upload_format [i].value = SWITCH_ON;
            else
                g_menu_entries_upload_format [i].value = SWITCH_OFF;
        }

        ACTIVITYSTACK->push (_menu_entries[nc].activity);
    }
    else if (id == IDC_CLOSE && nc == NCSN_WIDGET_CLICKED) {
        ACTIVITYSTACK->pop (0);
    }

    return TRUE;
}

static LRESULT mymain_onSwitchStateChanged (mMainWnd* self, UINT msg, DWORD wparam, DWORD lparam)
{
    int row = (int)wparam;
    int event_id = (int)lparam;

    if (row == (IDE_AUTO_DELETE - IDE_MIN)) {
        SYSCFGSERVICE->setAUCOptionAutoDel (event_id == NCSN_SLIDESWITCHPIECE_ON);
    }
    else if (row == (IDE_WRITE_LOCATION - IDE_MIN)) {
        SYSCFGSERVICE->setAUCOptionWriteLoc (event_id == NCSN_SLIDESWITCHPIECE_ON);
    }

    return 0;
}

static LRESULT mymain_onReady (mMainWnd* self, UINT msg, DWORD wparam, DWORD lparam)
{
    if (wparam == ACTIVITY_POPPED) {
        int filter = (int)lparam;
        if (filter > 0) {
            if (SYSCFGSERVICE->setAUCOptionFilter (filter)) {
                _menu_entries [IDE_UPLOAD_FORMAT - IDE_MIN].value = _(g_menu_entries_upload_format[filter - 1].name);
                _c(_listpiece)->reloadData (_listpiece);
            }
        }
    }

    return 0;
}

static BOOL mymain_onCreate (mMainWnd* self, DWORD dwAddData)
{
    return TRUE;
}

static BOOL mymain_onClose (mMainWnd* self, int message)
{
    _MG_PRINTF ("AUCSettingsActivity: mymain_onClose called.\n");

    return TRUE;
}

static BOOL mymain_onCreated (struct _NCS_CREATE_NOTIFY_INFO* info, mComponent* self, DWORD special_id)
{
    mWidget* ctrl;

    ctrl = (mWidget*)(_c(self)->getChild (self, IDC_CLOSE));
    if (ctrl)
        _c(ctrl)->setProperty (ctrl, NCSP_IMAGE_IMAGE, (DWORD)GetResource (Str2Key (GLOBAL_IMG_ARROW_DOWN)));

    ctrl = (mWidget*)(_c(self)->getChild (self, IDC_CONTENT));
    if (ctrl) {
        for (size_t i =0; i < TABLESIZE (_menu_entries); i++) {
            _menu_entries [i].name = _(_menu_entries [i].name);
        }

        _init_menu_entry_values (false);

        RECT rc = {0, 0, ACTIVITY_CONTENT_W, ACTIVITY_CONTENT_H};
        _listpiece = createMenuListView ((mContainerCtrl*)ctrl, &rc, _menu_entries, (int)TABLESIZE (_menu_entries));
    }
    return TRUE;
}

static NCS_CREATE_NOTIFY_INFO create_notify_info = {
    onCreated: mymain_onCreated,
};

static NCS_EVENT_HANDLER mymain_handlers [] = {
    {MSG_CREATE, reinterpret_cast<void*>(mymain_onCreate)},
    {MSG_COMMAND, reinterpret_cast<void*>(mymain_onCommand)},
    {MSG_USER, reinterpret_cast<void*>(mymain_onSwitchStateChanged)},
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
        class_name: NCSCTRL_IMAGE,
        id: IDC_CLOSE,
        x: 0,
        y: 0,
        w: ACTIVITY_TITLE_W,
        h: ACTIVITY_TITLE_H,
        style: WS_VISIBLE | NCSS_NOTIFY,
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
    }
};

static NCS_MNWND_TEMPLATE mymain_templ = {
    NCSCTRL_DIALOGBOX,
    1,
    ACTIVITY_X, ACTIVITY_Y, ACTIVITY_W, ACTIVITY_H,
    WS_NONE, WS_EX_NONE,
    N_("Upload Settings"),
    NULL,
    NULL,
    mymain_handlers,
    _ctrl_templ,
    TABLESIZE(_ctrl_templ),
    0, 0, NULL, 0, 0,
    &create_notify_info,
    0
};

AUCSettingsActivity::AUCSettingsActivity() : NCSActivity (&mymain_templ)
{
    m_style = STYLE_PUSH;
}

AUCSettingsActivity::~AUCSettingsActivity()
{
}

