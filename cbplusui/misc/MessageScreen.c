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
** Copyright (C) 2018 Beijing Joobot Technologies Inc.
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
 * @file MessageScreen.c
 * @Synopsis Message Screen
 * @author Vincent Wei
 * @version 1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <ctype.h>

#include "global.h"

#include "utilities.h"
#include "misc.h"

#define _FULL_SCREEN
#include "resource.h"

static BOOL my_onCommand (mMainWnd* self, int id, int nc, HWND hCtrl)
{
    if (nc == NCSN_WIDGET_CLICKED) {
        _c(self)->endDialog (self, id);
        return FALSE;
    }

    return TRUE;
}

static BOOL ms_onKeyDown (mMainWnd* self, int message, int code, DWORD key_status)
{
    if (code == SCANCODE_ESCAPE) {
        SendNotifyMessage (self->hwnd, MSG_COMMAND, MAKELONG (IDCANCEL, NCSN_WIDGET_CLICKED), 0);
    }

    return FALSE;
}

#define IDT_AUTOCANCEL 100
static void ms_onTimer (mMainWnd* self, LINT id, DWORD tick_count)
{
    if (id == IDT_AUTOCANCEL) {
        SendNotifyMessage (self->hwnd, MSG_COMMAND, MAKELONG (IDCANCEL, NCSN_WIDGET_CLICKED), 0);
        KillTimer (self->hwnd, IDT_AUTOCANCEL);
    }
}

static BOOL ms_onCreated (struct _NCS_CREATE_NOTIFY_INFO* info, mComponent* self, DWORD special_id)
{
    mWidget* ctrl_title = (mWidget*)(_c(self)->getChild (self, IDC_TITLE));
    ncsSetElement (ctrl_title, NCS_FGC_WINDOW, COLOR_TITLE);
    ncsSetFont (ctrl_title->hwnd, FONT_TITLE);

    if (special_id & MS_AUTOCANCEL) {
        SetTimer (((mMainWnd*)self)->hwnd, IDT_AUTOCANCEL, 500);
    }

    return TRUE;
}

static NCS_CREATE_NOTIFY_INFO create_notify_info = {
    onCreated: ms_onCreated,
};

static NCS_EVENT_HANDLER _ms_handlers[] = {
	{ MSG_KEYDOWN, ms_onKeyDown },
    { MSG_COMMAND, my_onCommand },
    { MSG_TIMER, ms_onTimer },
    { 0, NULL }
};

static NCS_PROP_ENTRY _props_content [] = {
    { NCSP_STATIC_AUTOWRAP, 1 },
    { NCSP_STATIC_ALIGN, NCS_ALIGN_LEFT },
    { NCSP_STATIC_VALIGN, NCS_VALIGN_TOP },
    { -1, 0},
};

static NCS_WND_TEMPLATE _ms_ctrls[] =
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
        class_name: NCSCTRL_STATIC,
        id: IDC_CONTENT,
        x: ACTIVITY_CONTENT_X + ACTIVITY_CONTENT_MARGIN_H,
        y: ACTIVITY_CONTENT_Y + ACTIVITY_CONTENT_MARGIN_V,
        w: ACTIVITY_CONTENT_W - (ACTIVITY_CONTENT_MARGIN_H * 2),
        h: ACTIVITY_CONTENT_H - 100,
        style: WS_VISIBLE,
        ex_style: WS_EX_NONE,
        caption: "",
        props: _props_content,
    },
    {
        class_name: NCSCTRL_IMAGE,
        id: IDOK,
        x: ACTIVITY_CONTENT_X,
        y: ACTIVITY_CONTENT_Y + ACTIVITY_CONTENT_H - 80,
        w: ACTIVITY_CONTENT_W,
        h: 60,
        style: WS_NONE,
        ex_style: WS_EX_NONE,
        caption: "",
    },
    {
        class_name: NCSCTRL_IMAGE,
        id: IDCANCEL,
        x: ACTIVITY_CONTENT_X + ACTIVITY_CONTENT_W/2,
        y: ACTIVITY_CONTENT_Y + ACTIVITY_CONTENT_H - 80,
        w: ACTIVITY_CONTENT_W/2,
        h: 60,
        style: WS_NONE,
        ex_style: WS_EX_NONE,
        caption: "",
    },
    {
        class_name: NCSCTRL_IMAGE,
        id: IDRETRY,
        x: ACTIVITY_CONTENT_X + ACTIVITY_CONTENT_W/2,
        y: ACTIVITY_CONTENT_Y + ACTIVITY_CONTENT_H - 80,
        w: ACTIVITY_CONTENT_W/2,
        h: 60,
        style: WS_NONE,
        ex_style: WS_EX_NONE,
        caption: "",
    },
};

static NCS_MNWND_TEMPLATE _ms_templ = {
    class_name: NCSCTRL_DIALOGBOX,
    id: 1,
    x: 0,
    y: 0,
    w: SCREEN_W,
    h: SCREEN_H,
    style: WS_NONE,
    ex_style: WS_EX_NONE | WS_EX_TOPMOST,
    caption: "Message Screen",
    handlers: _ms_handlers,
    ctrls: _ms_ctrls,
    count: TABLESIZE (_ms_ctrls),
    notify_info: &create_notify_info,
};

#define RES_GLOBAL_ICON_OK              "images/icon-ok.png"
#define RES_GLOBAL_ICON_CANCEL          "images/icon-cancel.png"
#define RES_GLOBAL_ICON_RETRY           "images/icon-retry.png"

static RES_NODE _myres_list [] = {
    {RES_GLOBAL_ICON_OK,            RES_TYPE_IMAGE, 16, 0},
    {RES_GLOBAL_ICON_CANCEL,        RES_TYPE_IMAGE, 16, 0},
    {RES_GLOBAL_ICON_RETRY,         RES_TYPE_IMAGE, 16, 0},

    /* terminator */
    {NULL}
};

int messageScreen (HWND hosting, const char* title, const char* text, DWORD style)
{
    int ret;

    loadResByTag (_myres_list, 0);

    NCS_PROP_ENTRY props_ok [] = {
        {NCSP_IMAGE_IMAGE, (DWORD)GetResource (Str2Key (RES_GLOBAL_ICON_OK))},
        {-1, 0},
    };

    NCS_PROP_ENTRY props_cancel [] = {
        {NCSP_IMAGE_IMAGE, (DWORD)GetResource (Str2Key (RES_GLOBAL_ICON_CANCEL))},
        {-1, 0},
    };

    NCS_PROP_ENTRY props_retry [] = {
        {NCSP_IMAGE_IMAGE, (DWORD)GetResource (Str2Key (RES_GLOBAL_ICON_RETRY))},
        {-1, 0},
    };

    _ms_ctrls [0].caption = title;
    _ms_ctrls [2].caption = text;
    _ms_ctrls [3].props = NULL;
    _ms_ctrls [4].props = NULL;
    _ms_ctrls [5].props = NULL;

    switch (style & MB_TYPEMASK) {
    case MB_OK:
        _ms_ctrls [3].style = WS_VISIBLE | NCSS_NOTIFY;
        _ms_ctrls [4].style = WS_NONE;
        _ms_ctrls [5].style = WS_NONE;

        _ms_ctrls [3].w = ACTIVITY_CONTENT_W;

        _ms_ctrls [3].props = props_ok;
        break;

    case MB_OKCANCEL:
        _ms_ctrls [3].style = WS_VISIBLE | NCSS_NOTIFY;
        _ms_ctrls [4].style = WS_VISIBLE | NCSS_NOTIFY;
        _ms_ctrls [5].style = WS_NONE;

        _ms_ctrls [3].w = ACTIVITY_CONTENT_W / 2;

        _ms_ctrls [3].props = props_ok;
        _ms_ctrls [4].props = props_cancel;
        break;

    case MB_RETRYCANCEL:
        _ms_ctrls [3].style = WS_NONE;
        _ms_ctrls [4].style = WS_VISIBLE | NCSS_NOTIFY;
        _ms_ctrls [5].style = WS_VISIBLE | NCSS_NOTIFY;

        _ms_ctrls [3].w = ACTIVITY_CONTENT_W / 2;

        _ms_ctrls [4].props = props_cancel;
        _ms_ctrls [5].props = props_retry;
        break;
    }

    _ms_templ.special_id = style;
    mDialogBox* mydlg = (mDialogBox *)ncsCreateMainWindowIndirect (&_ms_templ, hosting);
    ret = _c(mydlg)->doModal (mydlg, TRUE);

    unloadResByTag (_myres_list, 0);

    return ret;
}

