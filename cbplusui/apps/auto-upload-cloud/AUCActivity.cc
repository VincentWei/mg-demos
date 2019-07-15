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
 * @file AUCActivity.cc
 * @Synopsis The activity for Auto-Upload-Cloud.
 * @author Vincent Wei
 * @version 1.0
 * @date 06/06/2018
 */

#include <math.h>

#include "global.h"
#include "utilities.h"
#include "misc.h"

#include "Activity.hh"
#include "ActivityStack.hh"
#include "NCSActivity.hh"

#include "resource.h"

#include "SysConfService.hh"
#include "MiscService.hh"
#include "AUCActivity.hh"

REGISTER_ACTIVITY(AUCActivity);

static RES_NODE _myres_list [] = {
    {COMMON_IMG_CLOUD, RES_TYPE_IMAGE, 16, 0},
    {COMMON_IMG_FTP, RES_TYPE_IMAGE, 16, 0},
    {NULL},
};

static BOOL load_res (void)
{
    unsigned u;

    u = loadResByTag (_myres_list, 0);
    if (u < (TABLESIZE (_myres_list) - 1)) {
        _ERR_PRINTF ("AUCActivity: failed to load resource.\n");
        return FALSE;
    }

    return TRUE;
}

static void unload_res (void)
{
    unloadResByTag (_myres_list, 0);
}

#define IDC_CLOUD_ICON      201
#define IDC_CLOUD_SPACE_TXT 202
#define IDC_CLOUD_SPACE_BAR 203

#define IDC_TOUPLOAD        300
#define IDC_TOUPLOAD_NR     301
#define IDC_TOUPLOAD_SIZE   302

#define IDC_UPLOADED        400
#define IDC_UPLOADED_NR     401
#define IDC_UPLOADED_SIZE   402

#define IDC_WORK_STATE      420

#define IDC_SETTINGS        500

#define ICON_SIZE_W         80
#define ICON_SIZE_H         60

#define ICON_PULLUP_H       30

#define MARGIN_H            10
#define MARGIN_V            10

#define SPACING_V           10

#define TEXT_SUBJECT_H      30
#define TEXT_NUMBER_H       40
#define TEXT_SIZE_H         20

static bool set_cloud_icon (mMainWnd* self)
{
    mWidget* ctrl;
    if (SYSCFGSERVICE->getTransferProtocol() == SYS_TRANSFER_PROTOCOL_FTP) {
        ctrl = (mWidget*)(_c(self)->getChild (self, IDC_CLOUD_ICON));
        _c(ctrl)->setProperty (ctrl, NCSP_IMAGE_IMAGE, (DWORD)GetResource (Str2Key (COMMON_IMG_FTP)));
        return false;
    }
    else {
        ctrl = (mWidget*)(_c(self)->getChild (self, IDC_CLOUD_ICON));
        _c(ctrl)->setProperty (ctrl, NCSP_IMAGE_IMAGE, (DWORD)GetResource (Str2Key (COMMON_IMG_CLOUD)));

        return true;
    }
}

static void set_cloud_info (mMainWnd* self, float space_free, float space_total)
{
    mWidget* ctrl;
    float space_used = space_total - space_free;

    if (space_total < 1.0f || space_free > space_total) {
        _ERR_PRINTF ("AUCActivity: bad cloud space data (%f/%f).\n", space_free, space_total);
        return;
    }

    ctrl = (mWidget*)(_c(self)->getChild (self, IDC_CLOUD_SPACE_TXT));
    if (ctrl) {
        char size_used [16], size_total [16], size_all [32];
        humanSize (size_used, 15, space_used);
        humanSize (size_total, 15, space_total);

        strcpy (size_all, size_used);
        strcat (size_all, "/");
        strcat (size_all, size_total);

        SetWindowText (ctrl->hwnd, size_all);
    }

    ctrl = (mWidget*)(_c(self)->getChild (self, IDC_CLOUD_SPACE_BAR));
    if (ctrl) {
        int persent = (int)(space_used*100.0f/space_total);
         _c(ctrl)->setProperty (ctrl, NCSP_PROG_CURPOS, (DWORD)persent);
    }
}

static const char* upload_status[] = {
    N_("Idle"),
    N_("Reading: %s"),
    N_("Uploading: %s"),
    N_("Failed: %s"),
    N_("Unknown"),
};

static void set_upload_info (mMainWnd* self, int status, char* name)
{
    mWidget* ctrl;

    if (status < UPLOAD_STATUS_IDLE && status > UPLOAD_STATUS_UNKNOWN)
        status = UPLOAD_STATUS_UNKNOWN;

    ctrl = (mWidget*)(_c(self)->getChild (self, IDC_CLOUD_SPACE_TXT));
    if (ctrl) {
        char buff[256];

        memset(buff, 0, 256);
        switch (status) {
        case UPLOAD_STATUS_IDLE:
        case UPLOAD_STATUS_UNKNOWN:
            strncpy(buff, _(upload_status[status]), 255);
            break;

        default:
            if (name)
                snprintf(buff, 255, _(upload_status[status]), name);
            else
                snprintf(buff, 255, _(upload_status[status]), "");
            break;
        }

        SetWindowText (ctrl->hwnd, buff);
    }

    if (name)
        free (name);

    ctrl = (mWidget*)(_c(self)->getChild (self, IDC_CLOUD_SPACE_BAR));
    if (ctrl) {
        int percent;

        switch (status) {
        case UPLOAD_STATUS_READING:
            percent = 30;
            break;
        case UPLOAD_STATUS_UPLOADING:
            percent = 80;
            break;

        default:
            percent = 0;
            break;
        }

        _c(ctrl)->setProperty (ctrl, NCSP_PROG_CURPOS, (DWORD)percent);
    }
}

static BOOL mymain_onCreated (struct _NCS_CREATE_NOTIFY_INFO* info, mComponent* self, DWORD special_id)
{
    load_res ();

    mWidget* ctrl;
    ctrl = (mWidget*)(_c(self)->getChild (self, IDC_CLOUD_ICON));
    if (ctrl)
        _c(ctrl)->setProperty (ctrl, NCSP_IMAGE_IMAGE, (DWORD)GetResource (Str2Key (COMMON_IMG_CLOUD)));

    ctrl = (mWidget*)(_c(self)->getChild (self, IDC_TOUPLOAD));
    if (ctrl) {
        ncsSetFont (ctrl->hwnd, GLOBAL_FONT_LG);
        SetWindowText (ctrl->hwnd, _("To Upload"));
    }

    ctrl = (mWidget*)(_c(self)->getChild (self, IDC_TOUPLOAD_NR));
    if (ctrl) {
        ncsSetFont (ctrl->hwnd, GLOBAL_FONT_XL);
    }

    ctrl = (mWidget*)(_c(self)->getChild (self, IDC_UPLOADED));
    if (ctrl) {
        ncsSetFont (ctrl->hwnd, GLOBAL_FONT_LG);
        SetWindowText (ctrl->hwnd, _("Uploaded"));
    }

    ctrl = (mWidget*)(_c(self)->getChild (self, IDC_UPLOADED_NR));
    if (ctrl) {
        ncsSetFont (ctrl->hwnd, GLOBAL_FONT_XL);
    }

    ctrl = (mWidget*)(_c(self)->getChild (self, IDC_WORK_STATE));
    if (ctrl) {
        ncsSetElement (ctrl, NCS_FGC_WINDOW, COLOR_STATE_NORMAL);
        ncsSetFont (ctrl->hwnd, GLOBAL_FONT_SM);
        SetWindowText (ctrl->hwnd, _("Idle"));
    }

    ctrl = (mWidget*)(_c(self)->getChild (self, IDC_SETTINGS));
    if (ctrl)
        _c(ctrl)->setProperty (ctrl, NCSP_IMAGE_IMAGE, (DWORD)GetResource (Str2Key (GLOBAL_IMG_ARROW_UP)));

    if (set_cloud_icon ((mMainWnd*)self)) {
        set_cloud_info ((mMainWnd*)self, statusBar_getSpaceInfo (SBINFO_SPACE_FREE),
                statusBar_getSpaceInfo (SBINFO_SPACE_TOTAL));
    }
    else {
        set_upload_info ((mMainWnd*)self, UPLOAD_STATUS_IDLE, NULL);
    }

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
    unload_res ();
    return TRUE;
}

static BOOL mymain_onCommand (mWidget* self, int id, int nc, HWND hCtrl)
{
    if (id == IDC_SETTINGS && nc == NCSN_WIDGET_CLICKED) {
        ACTIVITYSTACK->push ("AUCSettingsActivity");
    }

    return TRUE;
}

static LRESULT mymain_onSateChanged (mMainWnd* self, UINT msg, DWORD wparam, DWORD lparam)
{
    mWidget* ctrl;

    switch (msg) {
    case MSG_CLOUD_CHANGED: {
        if (SYSCFGSERVICE->getTransferProtocol() != SYS_TRANSFER_PROTOCOL_FTP) {
            set_cloud_info (self, statusBar_getSpaceInfo (SBINFO_SPACE_FREE),
                    statusBar_getSpaceInfo (SBINFO_SPACE_TOTAL));
        }
        break;
    }

    case MSG_UPLOAD_CHANGED: {
        if (SYSCFGSERVICE->getTransferProtocol() == SYS_TRANSFER_PROTOCOL_FTP) {
            set_upload_info (self, (int)wparam, (char*)lparam);
        }
        break;
    }

    case MSG_TOUPLOAD_CHANGED: {
        int toupload_nr = (int)wparam;
        float toupload_size;
        memcpy (&toupload_size, &lparam, sizeof (float));

        ctrl = (mWidget*)(_c(self)->getChild (self, IDC_TOUPLOAD_NR));
        if (ctrl) {
            char buff [16];
            snprintf (buff, 15, "%d", toupload_nr);
            SetWindowText (ctrl->hwnd, buff);
        }
        ctrl = (mWidget*)(_c(self)->getChild (self, IDC_TOUPLOAD_SIZE));
        if (ctrl) {
            char buff [16];
            humanSize (buff, 15, toupload_size);
            SetWindowText (ctrl->hwnd, buff);
        }
        break;
    }

    case MSG_UPLOADED_CHANGED: {
        int uploaded_nr = (int)wparam;
        float uploaded_size;
        memcpy (&uploaded_size, &lparam, sizeof (float));

        ctrl = (mWidget*)(_c(self)->getChild (self, IDC_UPLOADED_NR));
        if (ctrl) {
            char buff [16];
            snprintf (buff, 15, "%d", uploaded_nr);
            SetWindowText (ctrl->hwnd, buff);
        }
        ctrl = (mWidget*)(_c(self)->getChild (self, IDC_UPLOADED_SIZE));
        if (ctrl) {
            char buff [16];
            humanSize (buff, 15, uploaded_size);
            SetWindowText (ctrl->hwnd, buff);
        }
        break;
    }

    default:
        break;
    }

    return 0;
}

static NCS_EVENT_HANDLER mymain_handlers [] = {
    {MSG_USER_APP_DATA, reinterpret_cast<void*>(mymain_onIntent)},
    {MSG_COMMAND, reinterpret_cast<void*>(mymain_onCommand)},
    {MSG_CLOUD_CHANGED, reinterpret_cast<void*>(mymain_onSateChanged)},
    {MSG_UPLOAD_CHANGED, reinterpret_cast<void*>(mymain_onSateChanged)},
    {MSG_TOUPLOAD_CHANGED, reinterpret_cast<void*>(mymain_onSateChanged)},
    {MSG_UPLOADED_CHANGED, reinterpret_cast<void*>(mymain_onSateChanged)},
    {MSG_CLOSE, reinterpret_cast<void*>(mymain_onClose)},
    {0, NULL}
};

static NCS_CREATE_NOTIFY_INFO create_notify_info = {
    onCreated: mymain_onCreated,
};

static NCS_PROP_ENTRY _props_static [] = {
    {NCSP_STATIC_ALIGN, NCS_ALIGN_LEFT},
    {-1, 0},
};

#ifndef OEM_TPP
static NCS_PROP_ENTRY _props_space [] = {
    {NCSP_PROG_MAXPOS, 100},
    {NCSP_PROG_MINPOS, 0},
    {NCSP_PROG_LINESTEP, 10},
    {NCSP_PROG_CURPOS, 0},
    {-1, 0},
};
#endif

static NCS_WND_TEMPLATE _ctrl_templ[] =
{
    {
        class_name: NCSCTRL_IMAGE,
        id: IDC_CLOUD_ICON,
        x: MARGIN_H,
        y: 0,
        w: ICON_SIZE_W,
        h: ICON_SIZE_H,
        style: WS_VISIBLE,
        ex_style: WS_EX_NONE,
        caption: "",
    },
#ifdef OEM_TPP
    {
        class_name: NCSCTRL_STATIC,
        id: IDC_STATIC,
        x: MARGIN_H + ICON_SIZE_W,
        y: MARGIN_V,
        w: ACTIVITY_W - ICON_SIZE_W - MARGIN_H*2,
        h: ICON_SIZE_H - MARGIN_V - 20,
        style: WS_VISIBLE,
        ex_style: WS_EX_NONE,
        caption: "自动上传云端",
        props: _props_static,
    },
#else
    {
        class_name: NCSCTRL_STATIC,
        id: IDC_CLOUD_SPACE_TXT,
        x: MARGIN_H + ICON_SIZE_W,
        y: MARGIN_V,
        w: ACTIVITY_W - ICON_SIZE_W - MARGIN_H*2,
        h: ICON_SIZE_H - MARGIN_V - 20,
        style: WS_VISIBLE,
        ex_style: WS_EX_NONE,
        caption: "0MB/0GB",
        props: _props_static,
    },
    {
        class_name: NCSCTRL_PROGRESSBAR,
        id: IDC_CLOUD_SPACE_BAR,
        x: MARGIN_H + ICON_SIZE_W,
        y: ICON_SIZE_H - 20,
        w: ACTIVITY_W - ICON_SIZE_W - MARGIN_H*3,
        h: 6,
        style: WS_VISIBLE | WS_BORDER,
        ex_style: WS_EX_NONE,
        caption: "",
        props: _props_space,
    },
#endif
    {
        class_name: NCSCTRL_STATIC,
        id: IDC_TOUPLOAD,
        x: MARGIN_H,
        y: ICON_SIZE_H + SPACING_V,
        w: ACTIVITY_W/2 - MARGIN_H,
        h: TEXT_SUBJECT_H,
        style: WS_VISIBLE,
        ex_style: WS_EX_NONE,
        caption: "",
    },
    {
        class_name: NCSCTRL_STATIC,
        id: IDC_TOUPLOAD_NR,
        x: MARGIN_H,
        y: ICON_SIZE_H + SPACING_V + TEXT_SUBJECT_H,
        w: ACTIVITY_W/2 - MARGIN_H,
        h: TEXT_NUMBER_H,
        style: WS_VISIBLE,
        ex_style: WS_EX_NONE,
        caption: "0",
    },
    {
        class_name: NCSCTRL_STATIC,
        id: IDC_TOUPLOAD_SIZE,
        x: MARGIN_H,
        y: ICON_SIZE_H + SPACING_V + TEXT_SUBJECT_H + TEXT_NUMBER_H,
        w: ACTIVITY_W/2 - MARGIN_H,
        h: TEXT_SIZE_H,
        style: WS_VISIBLE,
        ex_style: WS_EX_NONE,
        caption: "0KB",
    },
    {
        class_name: NCSCTRL_STATIC,
        id: IDC_UPLOADED,
        x: ACTIVITY_W/2,
        y: ICON_SIZE_H + SPACING_V,
        w: ACTIVITY_W/2 - MARGIN_H,
        h: TEXT_SUBJECT_H,
        style: WS_VISIBLE,
        ex_style: WS_EX_NONE,
        caption: "",
    },
    {
        class_name: NCSCTRL_STATIC,
        id: IDC_UPLOADED_NR,
        x: ACTIVITY_W/2,
        y: ICON_SIZE_H + SPACING_V + TEXT_SUBJECT_H,
        w: ACTIVITY_W/2 - MARGIN_H,
        h: TEXT_NUMBER_H,
        style: WS_VISIBLE,
        ex_style: WS_EX_NONE,
        caption: "0",
    },
    {
        class_name: NCSCTRL_STATIC,
        id: IDC_UPLOADED_SIZE,
        x: ACTIVITY_W/2,
        y: ICON_SIZE_H + SPACING_V + TEXT_SUBJECT_H + TEXT_NUMBER_H,
        w: ACTIVITY_W/2 - MARGIN_H,
        h: TEXT_SIZE_H,
        style: WS_VISIBLE,
        ex_style: WS_EX_NONE,
        caption: "0KB",
    },
#ifndef OEM_TPP
    {
        class_name: NCSCTRL_STATIC,
        id: IDC_WORK_STATE,
        x: ACTIVITY_CONTENT_MARGIN_H,
        y: ICON_SIZE_H + SPACING_V + TEXT_SUBJECT_H + TEXT_NUMBER_H + TEXT_SIZE_H,
        w: ACTIVITY_W - ACTIVITY_CONTENT_MARGIN_H * 2,
        h: TEXT_SIZE_H,
        style: WS_VISIBLE,
        ex_style: WS_EX_NONE,
        caption: "",
    },
    {
        class_name: NCSCTRL_IMAGE,
        id: IDC_SETTINGS,
        x: 0,
        y: ACTIVITY_H - ICON_PULLUP_H,
        w: ACTIVITY_W,
        h: ICON_PULLUP_H,
        style: WS_VISIBLE | NCSS_NOTIFY,
        ex_style: WS_EX_NONE,
        caption: "",
    },
#endif
};

static NCS_MNWND_TEMPLATE mymain_templ = {
    NCSCTRL_DIALOGBOX,
    1,
    ACTIVITY_X, ACTIVITY_Y, ACTIVITY_W, ACTIVITY_H,
    WS_NONE, WS_EX_NONE,
    N_("Auto Upload Cloud"),
    NULL,
    NULL,
    mymain_handlers,
    _ctrl_templ,
    TABLESIZE(_ctrl_templ),
    0, 0, NULL, 0, 0,
    &create_notify_info,
    0
};

int AUCActivity::onCancel ()
{
    if (messageScreen (m_hwnd, _("Please Confirm"), _("Exit the current work mode?"),
            MB_OKCANCEL | MS_AUTOCANCEL) == IDOK) {
        ACTIVITYSTACK->pop();
        return 0;
    }

    return 1;
}

int AUCActivity::onStart ()
{
    MISCSERVICE->startFunc (GET_GLOBAL_HOSTING, APP_AUTO_UPLOAD_CLOUD);
    return 0;
}

int AUCActivity::onStop ()
{
    MISCSERVICE->stopFunc (GET_GLOBAL_HOSTING, APP_AUTO_UPLOAD_CLOUD);
    return 0;
}

AUCActivity::AUCActivity () : NCSActivity (&mymain_templ, true)
{
    m_style = STYLE_ZOOM;
}

