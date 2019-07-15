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
 * @file SettingCheckFtpActivity.cc
 * @Synopsis The activity for check FTP settings.
 * @author Vincent Wei
 * @version 1.0
 * @date 28/04/2019
 */

#ifndef _GNU_SOURCE
#   define _GNU_SOURCE
#endif

#include "global.h"
#include "utilities.h"
#include "misc.h"

#include "Activity.hh"
#include "ActivityStack.hh"
#include "NCSActivity.hh"

#define _HAVE_SUBTITLE
#include "resource.h"

#include "SysConfService.hh"
#include "SettingCheckFtpActivity.hh"

#define PAGE_CHECK_FTP              0
#define PAGE_PROMPT_OK                 1
#define PAGE_PROMPT_ERROR           2

#define IDC_DESC                    300
#define IDC_GOT_ERROR               301

REGISTER_ACTIVITY(SettingCheckFtpActivity);

#define RES_GLOBAL_FTP              "images/ftp.png"

static RES_NODE _myres_list [] = {
    /* images on status bar */
    {RES_GLOBAL_FTP,     RES_TYPE_IMAGE, 16, 0},
    {GLOBAL_IMG_OK,         RES_TYPE_IMAGE, 16, 0},
    {GLOBAL_IMG_CANCEL,     RES_TYPE_IMAGE, 16, 0},

    /* terminator */
    {NULL}
};

static BOOL mymain_onCreate (mMainWnd* self, DWORD dwAddData)
{
    return TRUE;
}

static LRESULT mymain_onIntent (mMainWnd* self, UINT msg, DWORD wparam, DWORD lparam)
{
    Intent* intent = (Intent*)wparam;
    if (intent) {
        SYSCFGSERVICE->setFtpAccount(self->hwnd,
                intent->getString ("host").c_str(),
                intent->getString ("port").c_str(),
                intent->getString ("user").c_str(),
                intent->getString ("password").c_str());
        Intent::deleteIntent (intent);

        SettingCheckFtpActivity* act;
        act = (SettingCheckFtpActivity*)GetWindowAdditionalData (self->hwnd);
        act->setAutoEscape (false);
    }

    return 0;
}

static const char* _error_state [] = {
    N_("FTP operations are ok."),                       // SYS_FTP_ERR_OK
    N_("Failed to connect to the FTP server."),         // SYS_FTP_ERR_CONNECT
    N_("Failed to make directory on FTP server."),      // SYS_FTP_ERR_MKDIR
    N_("Failed to change directory on FTP server."),    // SYS_FTP_ERR_CD
    N_("Failed to upload file to FTP server."),         // SYS_FTP_ERR_PUT
    N_("Failed to delete file on FTP server."),         // SYS_FTP_ERR_DELETE
    N_("Failed to remove directory on FTP server."),    // SYS_FTP_ERR_RMDIR
    N_("Unknown error!"),                               // SYS_FTP_ERR_UNKNOWN
};

static LRESULT mymain_onAsyncTaskFinished (mMainWnd* self, UINT msg, DWORD wparam, DWORD lparam)
{
    int page_id;
    int status = *(int*)lparam;
    mPropSheet* propsheet = (mPropSheet*)ncsGetChildObj (self->hwnd, IDC_CONTENT);

    if (status == SYS_FTP_OK) {
        page_id = PAGE_PROMPT_OK;
    }
    else if (status < SYS_FTP_ERR_UNKNOWN) {
        page_id = PAGE_PROMPT_ERROR;
    }
    else {
        page_id = PAGE_PROMPT_ERROR;
        status = SYS_FTP_ERR_UNKNOWN;
    }

    mPage* page = (mPage*)(_c(propsheet)->getPageByIndex (propsheet, page_id));
    _c(propsheet)->setProperty (propsheet, NCSP_PRPSHT_ACTIVEPAGEIDX, page_id);
    mWidget* ctrl = (mWidget*)(_c(page)->getChild (page, IDC_DESC));
    if (ctrl) {
        SetWindowText (ctrl->hwnd, _(_error_state[status]));
    }

    return 0;
}

static BOOL mymain_onClose (mWidget* self, int message)
{
    unloadResByTag (_myres_list, 0);
    return TRUE;
}

static BOOL mypage_onCommand (mWidget* self, int id, int nc, HWND hCtrl)
{
    if (nc != NCSN_WIDGET_CLICKED) {
        return TRUE;
    }

    if (id == IDOK) {
        ACTIVITYSTACK->pop (0);
    }
    else if (id == IDC_GOT_ERROR) {
        ACTIVITYSTACK->pop (1);
    }

    return TRUE;
}

static void init_page_check_ftp (mPage* self)
{
    /* CBPlus image */
    NCS_PROP_ENTRY props_cbplus [] = {
        {NCSP_IMAGE_IMAGE, (DWORD)GetResource (Str2Key (RES_GLOBAL_FTP))},
        {-1, 0},
    };

    NCS_WND_TEMPLATE ctrl_templ_cbplus = {
        class_name: NCSCTRL_IMAGE,
        id: IDC_CONTENT,
        x: 0,
        y: ACTIVITY_CONTENT_MARGIN_V,
        w: ACTIVITY_CONTENT_W,
        h: 75 + ACTIVITY_CONTENT_MARGIN_V + ACTIVITY_CONTENT_MARGIN_V,
        style: WS_VISIBLE,
        ex_style: WS_EX_NONE,
        caption: "",
    };

    ctrl_templ_cbplus.props = props_cbplus;
    ncsCreateWindowIndirect (&ctrl_templ_cbplus, self->hPanel);

    /* loading gif */
    char gif_path [MAX_PATH + 1];
    strcpy (gif_path, GetResPath ());
    strcat (gif_path, "/");
    strcat (gif_path, RES_GLOBAL_LOADING_SM);

    NCS_PROP_ENTRY props_loading [] = {
        {NCSP_ANMT_GIFFILE, (DWORD)gif_path},
        {NCSP_ANMT_INTERVAL, (DWORD)10},
        {-1, 0},
    };

    NCS_WND_TEMPLATE ctrl_templ_loading = {
        class_name: NCSCTRL_ANIMATE,
        id: IDC_LOADING,
        x: 0,
        y: ctrl_templ_cbplus.y + ctrl_templ_cbplus.h,
        w: ACTIVITY_CONTENT_W,
        h: 40,
        style: WS_VISIBLE | NCSS_ANMT_AUTOLOOP | NCSS_ANMT_AUTOPLAY,
        ex_style: WS_EX_NONE,
        caption: "",
    };

    ctrl_templ_loading.props = props_loading;
    ncsCreateWindowIndirect (&ctrl_templ_loading, self->hPanel);
}

static void init_page_ftp_ok (mPage* self)
{
    /* Prompt text */
    NCS_PROP_ENTRY props_prompt [] = {
        { NCSP_STATIC_AUTOWRAP, 1 },
        { -1, 0},
    };

    NCS_WND_TEMPLATE ctrl_templ_prompt = {
        class_name: NCSCTRL_STATIC,
        id: IDC_STATIC,
        x: ACTIVITY_CONTENT_MARGIN_H,
        y: ACTIVITY_CONTENT_MARGIN_V,
        w: ACTIVITY_CONTENT_W - ACTIVITY_CONTENT_MARGIN_H - ACTIVITY_CONTENT_MARGIN_H,
        h: (ACTIVITY_CONTENT_H >> 1) - (ACTIVITY_CONTENT_MARGIN_V << 1),
        style: WS_VISIBLE,
        ex_style: WS_EX_NONE,
        caption: _("Your FTP settings are Ok!"),
    };

    ctrl_templ_prompt.props = props_prompt;
    ncsCreateWindowIndirect (&ctrl_templ_prompt, self->hPanel);

    /* ok button */
    NCS_WND_TEMPLATE ctrl_templ_ok = {
        class_name: NCSCTRL_BUTTON,
        id: IDOK,
        x: 50,
        y: ctrl_templ_prompt.y + ctrl_templ_prompt.h + ACTIVITY_CONTENT_MARGIN_V,
        w: ACTIVITY_W - 50 - 50,
        h: ACTIVITY_CONTENT_ITEM_H,
        style: WS_VISIBLE | NCSS_NOTIFY,
        ex_style: WS_EX_NONE,
        caption: _("Done"),
    };

    ncsCreateWindowIndirect (&ctrl_templ_ok, self->hPanel);
}

static void init_page_prompt_error (mPage* self)
{
    /* Prompt text */
    NCS_PROP_ENTRY props_prompt [] = {
        { NCSP_STATIC_AUTOWRAP, 1 },
        { -1, 0},
    };

    NCS_WND_TEMPLATE ctrl_templ_prompt = {
        class_name: NCSCTRL_STATIC,
        id: IDC_DESC,
        x: ACTIVITY_CONTENT_MARGIN_H,
        y: ACTIVITY_CONTENT_MARGIN_V,
        w: ACTIVITY_CONTENT_W - ACTIVITY_CONTENT_MARGIN_H - ACTIVITY_CONTENT_MARGIN_H,
        h: (ACTIVITY_CONTENT_H >> 1) - (ACTIVITY_CONTENT_MARGIN_V << 1),
        style: WS_VISIBLE,
        ex_style: WS_EX_NONE,
        caption: _("Failed to connect to FTP server. Please check your settings."),
    };

    ctrl_templ_prompt.props = props_prompt;
    ncsCreateWindowIndirect (&ctrl_templ_prompt, self->hPanel);

    /* got error button */
    NCS_WND_TEMPLATE ctrl_templ_ok = {
        class_name: NCSCTRL_BUTTON,
        id: IDC_GOT_ERROR,
        x: 50,
        y: ctrl_templ_prompt.y + ctrl_templ_prompt.h + ACTIVITY_CONTENT_MARGIN_V,
        w: ACTIVITY_W - 50 - 50,
        h: ACTIVITY_CONTENT_ITEM_H,
        style: WS_VISIBLE | NCSS_NOTIFY,
        ex_style: WS_EX_NONE,
        caption: _("I see"),
    };

    ncsCreateWindowIndirect (&ctrl_templ_ok, self->hPanel);
}

static void mypage_onInitPage (mPage* self, DWORD add_data)
{
    switch (add_data) {
        case PAGE_CHECK_FTP:
            init_page_check_ftp (self);
            break;
        case PAGE_PROMPT_OK:
            init_page_ftp_ok (self);
            break;
        case PAGE_PROMPT_ERROR:
            init_page_prompt_error (self);
            break;
        default: break;
    }
}

static int mypage_onShowPage (mPage* self, HWND focusChild, int show_cmd)
{
    DWORD page_id = GetWindowAdditionalData (self->hwnd);
    HWND hmain = GetMainWindowHandle (self->hwnd);
    SettingCheckFtpActivity* act;

    act = (SettingCheckFtpActivity*)GetWindowAdditionalData (hmain);
    if (show_cmd == SW_SHOW) {
        switch (page_id) {
        case PAGE_CHECK_FTP: {
            mAnimate* ctrl_loading = (mAnimate*)(_c(self)->getChild (self, IDC_LOADING));
            ncsAnimateStart (ctrl_loading);
            break;
        }

        case PAGE_PROMPT_OK:
        case PAGE_PROMPT_ERROR:
            act->setAutoEscape (true);
            break;

        default:
            break;
        }

        mWidget* subtitle = (mWidget*)ncsGetChildObj (hmain, IDC_SUBTITLE);
        if (subtitle)
            SetWindowText (subtitle->hwnd, GetWindowCaption (((mWidget*)self)->hwnd));
    }
    else {
        switch (page_id) {
        case PAGE_CHECK_FTP: {
            mAnimate* ctrl_loading = (mAnimate*)(_c(self)->getChild (self, IDC_LOADING));
            if (ctrl_loading)
                ncsAnimateStop (ctrl_loading);
            break;
        }

        default:
            break;
        }
    }

    return 1;
}

static int mypage_onSheetCmd (mPage* self, DWORD wParam, DWORD lParam)
{
    if (wParam == IDOK)
        return 1;
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
        _ERR_PRINTF ("SettingCheckFtpActivity: failed to get content (property sheet) control.\n");
        return FALSE;
    }

    mPage* page;

    page_info.caption = _("Checking FTP Settings…");
    page_info.dwAddData = PAGE_CHECK_FTP;
    page = _c(propsheet)->addPage (propsheet, &page_info, mypage_handlers);
    SetWindowBkColor (((mWidget*)page)->hwnd, PIXEL_black);

    page_info.caption = _("FTP Ok");
    page_info.dwAddData = PAGE_PROMPT_OK;
    page = _c(propsheet)->addPage (propsheet, &page_info, mypage_handlers);
    SetWindowBkColor (((mWidget*)page)->hwnd, PIXEL_black);

    page_info.caption = _("Wrong settings or bad network");
    page_info.dwAddData = PAGE_PROMPT_ERROR;
    page = _c(propsheet)->addPage (propsheet, &page_info, mypage_handlers);
    SetWindowBkColor (((mWidget*)page)->hwnd, PIXEL_black);
    return TRUE;
}

static BOOL mymain_onCreated (struct _NCS_CREATE_NOTIFY_INFO* info, mComponent* self, DWORD special_id)
{
    unsigned cnt;
    cnt = loadResByTag (_myres_list, 0);

    if (cnt < (TABLESIZE (_myres_list) - 1)) {
        _ERR_PRINTF ("SettingCheckFtpActivity: failed to load resource.\n");
        return FALSE;
    }

    mWidget* ctrl;

    ctrl = (mWidget*)(_c(self)->getChild (self, IDC_TITLE));
    ncsSetElement (ctrl, NCS_FGC_WINDOW, COLOR_TITLE);
    ncsSetFont (ctrl->hwnd, FONT_TITLE);
    SetWindowText (ctrl->hwnd, _("Check FTP"));

    ctrl = (mWidget*)(_c(self)->getChild (self, IDC_SUBTITLE));
    _c(ctrl)->setProperty (ctrl, NCSP_STATIC_VALIGN, (DWORD)NCS_VALIGN_TOP);
    ncsSetElement (ctrl, NCS_FGC_WINDOW, COLOR_SUBTITLE);
    ncsSetFont (ctrl->hwnd, FONT_SUBTITLE);
    SetWindowText (ctrl->hwnd, _("Checking…"));

    init_content (self);
    return TRUE;
}

static BOOL mymain_onCommand (mWidget* self, int id, int nc, HWND hCtrl)
{
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

static NCS_EVENT_HANDLER mymain_handlers [] = {
    { MSG_CREATE, reinterpret_cast<void*>(mymain_onCreate) },
    { MSG_USER_APP_DATA, reinterpret_cast<void*>(mymain_onIntent) },
    { MSG_COMMAND, reinterpret_cast<void*>(mymain_onCommand) },
    { MSG_ASYNC_TASK_FINISHED, reinterpret_cast<void*>(mymain_onAsyncTaskFinished) },
    { MSG_CLOSE, reinterpret_cast<void*>(mymain_onClose) },
    { 0, NULL}
};

static NCS_MNWND_TEMPLATE mymain_templ = {
    NCSCTRL_DIALOGBOX,
    1,
    ACTIVITY_X, ACTIVITY_Y, ACTIVITY_W, ACTIVITY_H,
    WS_NONE, WS_EX_NONE,
    N_("Check FTP"),
    NULL,
    NULL,
    mymain_handlers,
    _ctrl_templ,
    TABLESIZE(_ctrl_templ),
    0, 0, NULL, 0, 0,
    &create_notify_info,
    0
};

SettingCheckFtpActivity::SettingCheckFtpActivity () :
    NCSActivity (&mymain_templ)
{
    m_style = STYLE_PUSH;
}

