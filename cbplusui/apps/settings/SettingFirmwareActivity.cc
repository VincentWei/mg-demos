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
 * @file SettingFirmwareActivity.cc
 * @Synopsis The activity for WiFi password.
 * @author Vincent Wei
 * @version 1.0
 * @date 23/05/2018
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

#include "FirmwareService.hh"
#include "SettingFirmwareActivity.hh"

#define PAGE_CHECK_FIRMWARE         0
#define PAGE_CONFIRM_DOWNLOAD       1
#define PAGE_DOWNLOAD_FIRMWARE      2
#define PAGE_CONFIRM_UPGRADE        3
#define PAGE_UPGRADE_FIRMWARE       4
#define PAGE_PROMPT_UPTODATE        5
#define PAGE_PROMPT_LOWBATTERY      6
#define PAGE_PROMPT_ERROR           7

#define IDC_DESC                    300
#define IDC_PROGRESS                301
#define IDC_STATE                   302

REGISTER_ACTIVITY(SettingFirmwareActivity);

#define RES_GLOBAL_CBPLUS           "images/cbplus.png"

static RES_NODE _myres_list [] = {
    /* images on status bar */
    {RES_GLOBAL_CBPLUS,     RES_TYPE_IMAGE, 16, 0},
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
    if (intent)
        Intent::deleteIntent (intent);
    return 0;
}

#define FWS_OK                      0
#define FWS_ERR_NETWORK             1
#define FWS_ERR_DISK_FULL           2
#define FWS_ERR_LOW_BATTERY         3
#define FWS_ERR_NO_EXT_POWER        4
#define FWS_ERR_UPTODATE            5

static LRESULT mymain_onAsyncTaskFinished (mMainWnd* self, UINT msg, DWORD wparam, DWORD lparam)
{
    FIRMWARE_INFO* info = (FIRMWARE_INFO*)lparam;
    mPropSheet* propsheet = (mPropSheet*)ncsGetChildObj (self->hwnd, IDC_CONTENT);

    if (info->status == FWS_OK) {
        switch (wparam) {
        case TASKID_FIRMWARE_CHECK: {
            mPage* page = (mPage*)(_c(propsheet)->getPageByIndex (propsheet, PAGE_CONFIRM_DOWNLOAD));
            mWidget* ctrl = (mWidget*)(_c(page)->getChild (page, IDC_DESC));
            if (ctrl) {
                char* buff;
                char buff_size [16];
                humanSize (buff_size, 15, (float)info->size);
                if (asprintf (&buff, "V%s-%d (%s)\n%s", info->verName, info->verCode, buff_size, info->verDesc) >= 0) {
                    SetWindowText (ctrl->hwnd, buff);
                    free (buff);
                }
            }
            _c(propsheet)->setProperty (propsheet, NCSP_PRPSHT_ACTIVEPAGEIDX, PAGE_CONFIRM_DOWNLOAD);
            return 0;
        }

        case TASKID_FIRMWARE_DOWNLOAD:
        case TASKID_FIRMWARE_UPGRADE:
            _MG_PRINTF ("SettingFirmwareActivity: do nothing for task: %d.\n", (int)wparam);
            return 0;
        }
    }

    switch (info->status) {
    case FWS_ERR_UPTODATE:
        _c(propsheet)->setProperty (propsheet, NCSP_PRPSHT_ACTIVEPAGEIDX, PAGE_PROMPT_UPTODATE);
        break;

    case FWS_ERR_LOW_BATTERY:
        _c(propsheet)->setProperty (propsheet, NCSP_PRPSHT_ACTIVEPAGEIDX, PAGE_PROMPT_LOWBATTERY);
        break;

    default:
        _c(propsheet)->setProperty (propsheet, NCSP_PRPSHT_ACTIVEPAGEIDX, PAGE_PROMPT_ERROR);
        break;
    }

    return 0;
}

static BOOL mymain_onClose (mWidget* self, int message)
{
    _MG_PRINTF ("SettingFirmwareActivity: mymain_onClose called.\n");
    unloadResByTag (_myres_list, 0);
    return TRUE;
}

static BOOL mypage_onCommand (mWidget* self, int id, int nc, HWND hCtrl)
{
    HWND hmainwin = GetMainWindowHandle (hCtrl);
    mPropSheet *propsheet = (mPropSheet*)ncsGetChildObj (hmainwin, IDC_CONTENT);

    if (nc != NCSN_WIDGET_CLICKED) {
        return TRUE;
    }

    if (id == IDOK) {
        ACTIVITYSTACK->pop ();
        return TRUE;
    }

    int idx_page = _c(propsheet)->getProperty (propsheet, NCSP_PRPSHT_ACTIVEPAGEIDX);
    if (idx_page == PAGE_CONFIRM_DOWNLOAD) {
        if (id == IDYES) {
            _c(propsheet)->setProperty (propsheet, NCSP_PRPSHT_ACTIVEPAGEIDX, PAGE_DOWNLOAD_FIRMWARE);
            FIRMWARESERVICE->downloadFirmware (self->hwnd, NULL);
        }
        else if (id == IDNO) {
            ACTIVITYSTACK->pop ();
        }
    }
    else if (idx_page == PAGE_CONFIRM_UPGRADE) {
        if (id == IDYES) {
            _c(propsheet)->setProperty (propsheet, NCSP_PRPSHT_ACTIVEPAGEIDX, PAGE_UPGRADE_FIRMWARE);
            FIRMWARESERVICE->upgradeFirmware (self->hwnd, NULL);
        }
        else if (id == IDNO) {
            ACTIVITYSTACK->pop ();
        }
    }

    return TRUE;
}

static void init_page_check_firmware (mPage* self)
{
    /* CBPlus image */
    NCS_PROP_ENTRY props_cbplus [] = {
        {NCSP_IMAGE_IMAGE, (DWORD)GetResource (Str2Key (RES_GLOBAL_CBPLUS))},
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

static void init_page_confirm_download (mPage* self)
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
        caption: "",
    };

    ctrl_templ_prompt.props = props_prompt;
    ncsCreateWindowIndirect (&ctrl_templ_prompt, self->hPanel);

    /* Icon No and Yes */
    NCS_PROP_ENTRY props_no [] = {
        {NCSP_IMAGE_IMAGE, (DWORD)GetResource (Str2Key (GLOBAL_IMG_CANCEL))},
        {-1, 0},
    };

    NCS_WND_TEMPLATE ctrl_templ_no = {
        class_name: NCSCTRL_IMAGE,
        id: IDNO,
        x: ACTIVITY_CONTENT_MARGIN_H,
        y: ctrl_templ_prompt.y + ctrl_templ_prompt.h,
        w: (ACTIVITY_CONTENT_W >> 1) - ACTIVITY_CONTENT_MARGIN_H,
        h: ACTIVITY_CONTENT_H - ctrl_templ_prompt.h,
        style: WS_VISIBLE | NCSS_NOTIFY,
        ex_style: WS_EX_NONE,
        caption: "",
    };

    ctrl_templ_no.props = props_no;
    ncsCreateWindowIndirect (&ctrl_templ_no, self->hPanel);

    NCS_PROP_ENTRY props_yes [] = {
        {NCSP_IMAGE_IMAGE, (DWORD)GetResource (Str2Key (GLOBAL_IMG_OK))},
        {-1, 0},
    };

    NCS_WND_TEMPLATE ctrl_templ_yes = {
        class_name: NCSCTRL_IMAGE,
        id: IDYES,
        x: (ACTIVITY_CONTENT_W >> 1),
        y: ctrl_templ_no.y,
        w: ctrl_templ_no.w,
        h: ctrl_templ_no.h,
        style: WS_VISIBLE | NCSS_NOTIFY,
        ex_style: WS_EX_NONE,
        caption: "",
    };

    ctrl_templ_yes.props = props_yes;
    ncsCreateWindowIndirect (&ctrl_templ_yes, self->hPanel);
}

static void init_page_download_firmware (mPage* self)
{
    /* CBPlus image */
    NCS_PROP_ENTRY props_cbplus [] = {
        {NCSP_IMAGE_IMAGE, (DWORD)GetResource (Str2Key (RES_GLOBAL_CBPLUS))},
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

    /* download progress */
    NCS_PROP_ENTRY props_progress [] = {
	    {NCSP_PROG_MAXPOS, 100},
	    {NCSP_PROG_MINPOS, 0 },
	    {NCSP_PROG_LINESTEP, 10},
	    {NCSP_PROG_CURPOS, 0},
        {-1, 0},
    };

    NCS_WND_TEMPLATE ctrl_templ_progress = {
        class_name: NCSCTRL_PROGRESSBAR,
        id: IDC_PROGRESS,
        x: ACTIVITY_CONTENT_MARGIN_H,
        y: ctrl_templ_cbplus.y + ctrl_templ_cbplus.h + ACTIVITY_CONTENT_MARGIN_V,
        w: ACTIVITY_CONTENT_W - ACTIVITY_CONTENT_MARGIN_H - ACTIVITY_CONTENT_MARGIN_H,
        h: 10,
        style: WS_VISIBLE | WS_BORDER,
        ex_style: WS_EX_NONE,
        caption: "",
        props: props_progress,
    };

    ctrl_templ_progress.props = props_progress;
    ncsCreateWindowIndirect (&ctrl_templ_progress, self->hPanel);

    /* download state */
    NCS_WND_TEMPLATE ctrl_templ_state = {
        class_name: NCSCTRL_STATIC,
        id: IDC_STATE,
        x: ACTIVITY_CONTENT_MARGIN_H,
        y: ctrl_templ_progress.y + ctrl_templ_progress.h + ACTIVITY_CONTENT_MARGIN_V/2,
        w: ACTIVITY_CONTENT_W - ACTIVITY_CONTENT_MARGIN_H - ACTIVITY_CONTENT_MARGIN_H,
        h: ACTIVITY_CONTENT_ITEM_H,
        style: WS_VISIBLE,
        ex_style: WS_EX_NONE,
        caption: "",
    };

    ncsCreateWindowIndirect (&ctrl_templ_state, self->hPanel);
}

static void init_page_confirm_upgrade (mPage* self)
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
        caption: _("Do you want to update the firmware immediately?"),
    };

    ctrl_templ_prompt.props = props_prompt;
    ncsCreateWindowIndirect (&ctrl_templ_prompt, self->hPanel);

    /* Icon No and Yes */
    NCS_PROP_ENTRY props_no [] = {
        {NCSP_IMAGE_IMAGE, (DWORD)GetResource (Str2Key (GLOBAL_IMG_CANCEL))},
        {-1, 0},
    };

    NCS_WND_TEMPLATE ctrl_templ_no = {
        class_name: NCSCTRL_IMAGE,
        id: IDNO,
        x: ACTIVITY_CONTENT_MARGIN_H,
        y: ctrl_templ_prompt.y + ctrl_templ_prompt.h,
        w: (ACTIVITY_CONTENT_W >> 1) - ACTIVITY_CONTENT_MARGIN_H,
        h: ACTIVITY_CONTENT_H - ctrl_templ_prompt.h,
        style: WS_VISIBLE | NCSS_NOTIFY,
        ex_style: WS_EX_NONE,
        caption: "",
    };

    ctrl_templ_no.props = props_no;
    ncsCreateWindowIndirect (&ctrl_templ_no, self->hPanel);

    NCS_PROP_ENTRY props_yes [] = {
        {NCSP_IMAGE_IMAGE, (DWORD)GetResource (Str2Key (GLOBAL_IMG_OK))},
        {-1, 0},
    };

    NCS_WND_TEMPLATE ctrl_templ_yes = {
        class_name: NCSCTRL_IMAGE,
        id: IDYES,
        x: (ACTIVITY_CONTENT_W >> 1),
        y: ctrl_templ_no.y,
        w: ctrl_templ_no.w,
        h: ctrl_templ_no.h,
        style: WS_VISIBLE | NCSS_NOTIFY,
        ex_style: WS_EX_NONE,
        caption: "",
    };

    ctrl_templ_yes.props = props_yes;
    ncsCreateWindowIndirect (&ctrl_templ_yes, self->hPanel);
}

static void init_page_upgrade_firmware (mPage* self)
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
        caption: _("Do not power off the device. Your CBPlus will reboot automatically after upgraded."),
    };

    ctrl_templ_prompt.props = props_prompt;
    ncsCreateWindowIndirect (&ctrl_templ_prompt, self->hPanel);

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
        y: ctrl_templ_prompt.y + ctrl_templ_prompt.h,
        w: ACTIVITY_CONTENT_W,
        h: 30,
        style: WS_VISIBLE | NCSS_ANMT_AUTOLOOP | NCSS_ANMT_AUTOPLAY,
        ex_style: WS_EX_NONE,
        caption: "",
    };

    ctrl_templ_loading.props = props_loading;
    ncsCreateWindowIndirect (&ctrl_templ_loading, self->hPanel);

    /* upgrade progress */
    NCS_PROP_ENTRY props_progress [] = {
	    {NCSP_PROG_MAXPOS, 100},
	    {NCSP_PROG_MINPOS, 0 },
	    {NCSP_PROG_LINESTEP, 10},
	    {NCSP_PROG_CURPOS, 0},
        {-1, 0},
    };

    NCS_WND_TEMPLATE ctrl_templ_progress = {
        class_name: NCSCTRL_PROGRESSBAR,
        id: IDC_PROGRESS,
        x: ACTIVITY_CONTENT_MARGIN_H,
        y: ctrl_templ_loading.y + ctrl_templ_loading.h + ACTIVITY_CONTENT_MARGIN_V,
        w: ACTIVITY_CONTENT_W - ACTIVITY_CONTENT_MARGIN_H - ACTIVITY_CONTENT_MARGIN_H,
        h: 10,
        style: WS_VISIBLE | WS_BORDER,
        ex_style: WS_EX_NONE,
        caption: "",
        props: props_progress,
    };

    ctrl_templ_progress.props = props_progress;
    ncsCreateWindowIndirect (&ctrl_templ_progress, self->hPanel);

    /* upgrade state */
    NCS_WND_TEMPLATE ctrl_templ_state = {
        class_name: NCSCTRL_STATIC,
        id: IDC_STATE,
        x: ACTIVITY_CONTENT_MARGIN_H,
        y: ctrl_templ_progress.y + ctrl_templ_progress.h + ACTIVITY_CONTENT_MARGIN_V/2,
        w: ACTIVITY_CONTENT_W - ACTIVITY_CONTENT_MARGIN_H - ACTIVITY_CONTENT_MARGIN_H,
        h: ACTIVITY_CONTENT_ITEM_H,
        style: WS_VISIBLE,
        ex_style: WS_EX_NONE,
        caption: "",
    };

    ncsCreateWindowIndirect (&ctrl_templ_state, self->hPanel);
}

static void init_page_prompt_uptodate (mPage* self)
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
        caption: _("The firmware is up to date."),
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
        caption: _("I see"),
    };

    ncsCreateWindowIndirect (&ctrl_templ_ok, self->hPanel);
}

static void init_page_prompt_lowbattery (mPage* self)
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
        caption: _("The battery is low. Upgrade of firmware was canceled."),
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
        caption: _("I see"),
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
        caption: _("Failed to download firmware. Please check your network settings."),
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
        caption: _("I see"),
    };

    ncsCreateWindowIndirect (&ctrl_templ_ok, self->hPanel);
}

static void mypage_onInitPage (mPage* self, DWORD add_data)
{
    switch (add_data) {
        case PAGE_CHECK_FIRMWARE:       init_page_check_firmware (self);        break;
        case PAGE_CONFIRM_DOWNLOAD:     init_page_confirm_download (self);      break;
        case PAGE_DOWNLOAD_FIRMWARE:    init_page_download_firmware (self);     break;
        case PAGE_CONFIRM_UPGRADE:      init_page_confirm_upgrade (self);       break;
        case PAGE_UPGRADE_FIRMWARE:     init_page_upgrade_firmware (self);      break;
        case PAGE_PROMPT_UPTODATE:      init_page_prompt_uptodate (self);       break;
        case PAGE_PROMPT_LOWBATTERY:    init_page_prompt_lowbattery (self);     break;
        case PAGE_PROMPT_ERROR:         init_page_prompt_error (self);  break;
        default: break;
    }
}

static int mypage_onShowPage (mPage* self, HWND focusChild, int show_cmd)
{
    DWORD page_id = GetWindowAdditionalData (self->hwnd);
    HWND hmain = GetMainWindowHandle (self->hwnd);
    SettingFirmwareActivity* act;

    act = (SettingFirmwareActivity*)GetWindowAdditionalData (hmain);
    if (show_cmd == SW_SHOW) {
        switch (page_id) {
        case PAGE_CHECK_FIRMWARE: {
            mAnimate* ctrl_loading = (mAnimate*)(_c(self)->getChild (self, IDC_LOADING));
            ncsAnimateStart (ctrl_loading);
            FIRMWARESERVICE->checkFirmware (hmain);
            act->setAutoEscape (false);
            break;
        }

        case PAGE_PROMPT_UPTODATE:
        case PAGE_CONFIRM_DOWNLOAD:
        case PAGE_CONFIRM_UPGRADE:
        case PAGE_PROMPT_LOWBATTERY:
        case PAGE_PROMPT_ERROR:
            act->setAutoEscape (true);
            break;

        case PAGE_DOWNLOAD_FIRMWARE:
            act->setAutoEscape (false);
            break;

        case PAGE_UPGRADE_FIRMWARE: {
            statusBar_enableScreenLock (FALSE);
            act->setAutoEscape (false);
            mAnimate* ctrl_loading = (mAnimate*)(_c(self)->getChild (self, IDC_LOADING));
            if (ctrl_loading)
                ncsAnimateStart (ctrl_loading);
            break;
        }

        default:
            break;
        }

        mWidget* subtitle = (mWidget*)ncsGetChildObj (hmain, IDC_SUBTITLE);
        if (subtitle)
            SetWindowText (subtitle->hwnd, GetWindowCaption (((mWidget*)self)->hwnd));
    }
    else {
        switch (page_id) {
        case PAGE_CHECK_FIRMWARE:
        case PAGE_DOWNLOAD_FIRMWARE:
        case PAGE_UPGRADE_FIRMWARE:
        {
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
        _ERR_PRINTF ("SettingFirmwareActivity: failed to get content (property sheet) control.\n");
        return FALSE;
    }

    mPage* page;

    page_info.caption = _("Checking Firmware…");
    page_info.dwAddData = PAGE_CHECK_FIRMWARE;
    page = _c(propsheet)->addPage (propsheet, &page_info, mypage_handlers);
    SetWindowBkColor (((mWidget*)page)->hwnd, PIXEL_black);

    page_info.caption = _("Found New Firmware");
    page_info.dwAddData = PAGE_CONFIRM_DOWNLOAD;
    page = _c(propsheet)->addPage (propsheet, &page_info, mypage_handlers);
    SetWindowBkColor (((mWidget*)page)->hwnd, PIXEL_black);

    page_info.caption = _("Downloading Firmware…");
    page_info.dwAddData = PAGE_DOWNLOAD_FIRMWARE;
    page = _c(propsheet)->addPage (propsheet, &page_info, mypage_handlers);
    SetWindowBkColor (((mWidget*)page)->hwnd, PIXEL_black);

    page_info.caption = _("New Firmware Ready");
    page_info.dwAddData = PAGE_CONFIRM_UPGRADE;
    page = _c(propsheet)->addPage (propsheet, &page_info, mypage_handlers);
    SetWindowBkColor (((mWidget*)page)->hwnd, PIXEL_black);

    page_info.caption = _("Upgrading Firmware…");
    page_info.dwAddData = PAGE_UPGRADE_FIRMWARE;
    page = _c(propsheet)->addPage (propsheet, &page_info, mypage_handlers);
    SetWindowBkColor (((mWidget*)page)->hwnd, PIXEL_black);

    page_info.caption = _("No New Firmware");
    page_info.dwAddData = PAGE_PROMPT_UPTODATE;
    page = _c(propsheet)->addPage (propsheet, &page_info, mypage_handlers);
    SetWindowBkColor (((mWidget*)page)->hwnd, PIXEL_black);

    page_info.caption = _("Low Battery");
    page_info.dwAddData = PAGE_PROMPT_LOWBATTERY;
    page = _c(propsheet)->addPage (propsheet, &page_info, mypage_handlers);
    SetWindowBkColor (((mWidget*)page)->hwnd, PIXEL_black);

    page_info.caption = _("Error");
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
		_ERR_PRINTF ("SettingFirmwareActivity: failed to load resource.\n");
        return FALSE;
    }

    mWidget* ctrl;

    ctrl = (mWidget*)(_c(self)->getChild (self, IDC_TITLE));
    ncsSetElement (ctrl, NCS_FGC_WINDOW, COLOR_TITLE);
    ncsSetFont (ctrl->hwnd, FONT_TITLE);
    SetWindowText (ctrl->hwnd, _("Update Firmware"));

    ctrl = (mWidget*)(_c(self)->getChild (self, IDC_SUBTITLE));
    _c(ctrl)->setProperty (ctrl, NCSP_STATIC_VALIGN, (DWORD)NCS_VALIGN_TOP);
    ncsSetElement (ctrl, NCS_FGC_WINDOW, COLOR_SUBTITLE);
    ncsSetFont (ctrl->hwnd, FONT_SUBTITLE);
    SetWindowText (ctrl->hwnd, _("Checking Firmware…"));

    init_content (self);
    return TRUE;
}

static BOOL mymain_onCommand (mWidget* self, int id, int nc, HWND hCtrl)
{
    return TRUE;
}

#define MIN_ERRCODE             -10
static const char* _error_state [] = {
    N_("Failed to write data to root filesystem."),     // FIRMWARE_UPGRADE_STATE_FAILED_WRITE_ROOTFS = -10
    N_("Failed to write data to boot filesystem."),     // FIRMWARE_UPGRADE_STATE_FAILED_WRITE_BOOT = -9
    N_("Failed to write data to recovery filesystem."), // FIRMWARE_UPGRADE_STATE_FAILED_WRITE_RECOVERY = -8
    N_("Failed to check system data."),                 // FIRMWARE_UPGRADE_STATE_FAILED_CHECK_SYSTEM = -7
    N_("Failed to unpack system data."),                // FIRMWARE_UPGRADE_STATE_FAILED_UNPACK_SYSTEM = -6
    N_("Failed to check recovery data."),               // FIRMWARE_UPGRADE_STATE_FAILED_CHECK_RECOVERY = -5
    N_("Failed to unpack recovery data."),              // FIRMWARE_UPGRADE_STATE_FAILED_UNPACK_RECOVERY = -4
    N_("Failed to download firmware package."),         // FIRMWARE_DOWNLAD_STATE_FAILED_NETWORK = -3
    N_("Failed to unpack firmware package."),           // FIRMWARE_DOWNLAD_STATE_FAILED_UNPACK_BIN = -2
    N_("Failed to check firmware package."),            // FIRMWARE_DOWNLAD_STATE_FAILED_CHECK_BIN = -1
};

#define FIRMWARE_UPGRADE_STATE_SUCCESS      5
static const char* _normal_state [] = {
    N_("Download completed!"),         // FIRMWARE_DOWNLAD_STATE_SUCCESS = 0
    N_("Preparing download…"),        // FIRMWARE_DOWNLAD_STATE_BEGIN = 1
    N_("Downlaoding package…"),       // FIRMWARE_DOWNLAD_STATE_DOWNLOADING = 2
    N_("Checking package…"),          // FIRMWARE_DOWNLAD_STATE_CHECK_BIN = 3
    N_("Unpacking package…"),         // FIRMWARE_DOWNLAD_STATE_UNPACK_BIN = 4
    N_("Upgrade completed!"),          // FIRMWARE_UPGRADE_STATE_SUCCESS = FIRMWARE_DOWNLAD_STATE_SUCCESS = 0
    N_("Unpacking recovery data…"),   // FIRMWARE_UPGRADE_STATE_UNPACK_RECOVERY = 6
    N_("Checking recovery data…"),     // FIRMWARE_UPGRADE_STATE_CHECK_RECOVERY = 7
    N_("Unpacking system data…"),     // FIRMWARE_UPGRADE_STATE_UNPACK_SYSTEM = 8
    N_("Checking system data…"),      // FIRMWARE_UPGRADE_STATE_CHECK_SYSTEM = 9
    N_("Writting recovery filesystem…"),    // FIRMWARE_UPGRADE_STATE_WRITE_RECOVERY = 10
    N_("Writting boot filesystem…"),        // FIRMWARE_UPGRADE_STATE_WRITE_BOOT = 11
    N_("Writing root filesystem…"),         // FIRMWARE_UPGRADE_STATE_WRITE_ROOTFS = 12
};

static LRESULT mymain_inDownloading (mMainWnd* self, UINT msg, DWORD wparam, DWORD lparam)
{
    mWidget* ctrl;
    int progress = (int)wparam;
    int state = (int)lparam;

    mPropSheet* propsheet = (mPropSheet*)ncsGetChildObj (self->hwnd, IDC_CONTENT);
    mPage* page = (mPage*)(_c(propsheet)->getPageByIndex (propsheet, PAGE_DOWNLOAD_FIRMWARE));
    ctrl = (mWidget*)(_c(page)->getChild (page, IDC_PROGRESS));
    if (ctrl) {
        _c(ctrl)->setProperty (ctrl, NCSP_PROG_CURPOS, progress);
    }

    if (state == 0 && progress == 100) {
        _c(propsheet)->setProperty (propsheet, NCSP_PRPSHT_ACTIVEPAGEIDX, PAGE_CONFIRM_UPGRADE);
    }
    else if (state > 0) {
        ctrl = (mWidget*)(_c(page)->getChild (page, IDC_STATE));
        if (ctrl) {
            SetWindowText (ctrl->hwnd, _(_normal_state [state]));
        }
    }
    else {
        page = (mPage*)(_c(propsheet)->getPageByIndex (propsheet, PAGE_PROMPT_ERROR));
        ctrl = (mWidget*)(_c(page)->getChild (page, IDC_DESC));
        if (ctrl) {
            SetWindowText (ctrl->hwnd, _(_error_state [state - MIN_ERRCODE]));
        }

        _c(propsheet)->setProperty (propsheet, NCSP_PRPSHT_ACTIVEPAGEIDX, PAGE_PROMPT_ERROR);
    }

    return 0;
}

static LRESULT mymain_inUpgrading (mMainWnd* self, UINT msg, DWORD wparam, DWORD lparam)
{
    mWidget* ctrl;
    int progress = (int)wparam;
    int state = (int)lparam;

    mPropSheet* propsheet = (mPropSheet*)ncsGetChildObj (self->hwnd, IDC_CONTENT);
    mPage* page = (mPage*)(_c(propsheet)->getPageByIndex (propsheet, PAGE_UPGRADE_FIRMWARE));
    ctrl = (mWidget*)(_c(page)->getChild (page, IDC_PROGRESS));
    if (ctrl) {
        _c(ctrl)->setProperty (ctrl, NCSP_PROG_CURPOS, progress);
    }

    ctrl = (mWidget*)(_c(page)->getChild (page, IDC_STATE));
    if (state == 0) {
        state = FIRMWARE_UPGRADE_STATE_SUCCESS;
        if (ctrl)
            SetWindowText (ctrl->hwnd, _(_normal_state [state]));
    }
    else if (state > 0) {
        if (ctrl)
            SetWindowText (ctrl->hwnd, _(_normal_state [state]));
    }
    else {
        page = (mPage*)(_c(propsheet)->getPageByIndex (propsheet, PAGE_PROMPT_ERROR));
        ctrl = (mWidget*)(_c(page)->getChild (page, IDC_DESC));
        if (ctrl) {
            SetWindowText (ctrl->hwnd, _(_error_state [state - MIN_ERRCODE]));
        }

        _c(propsheet)->setProperty (propsheet, NCSP_PRPSHT_ACTIVEPAGEIDX, PAGE_PROMPT_ERROR);
    }

    return 0;
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
    { MSG_FIRMWARE_DOWNLOAD_STATE, reinterpret_cast<void*>(mymain_inDownloading) },
    { MSG_FIRMWARE_UPGRADE_STATE, reinterpret_cast<void*>(mymain_inUpgrading) },
    { MSG_CLOSE, reinterpret_cast<void*>(mymain_onClose) },
    { 0, NULL}
};

static NCS_MNWND_TEMPLATE mymain_templ = {
    NCSCTRL_DIALOGBOX,
    1,
    ACTIVITY_X, ACTIVITY_Y, ACTIVITY_W, ACTIVITY_H,
    WS_NONE, WS_EX_NONE,
    N_("Checking Firmware"),
    NULL,
    NULL,
    mymain_handlers,
    _ctrl_templ,
    TABLESIZE(_ctrl_templ),
    0, 0, NULL, 0, 0,
    &create_notify_info,
    0
};

SettingFirmwareActivity::SettingFirmwareActivity () :
    NCSActivity (&mymain_templ)
{
    m_style = STYLE_PUSH;
}

