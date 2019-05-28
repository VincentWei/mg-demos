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
 * @file MUCActivity.cc
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
#include "MUCActivity.hh"
#include "MenuListPiece.hh"

REGISTER_ACTIVITY(MUCActivity);

#define PAGE_SET_AND_START  0
#define PAGE_UPLOAD_STATE   1

#define IDC_CLOUD_ICON      201
#define IDC_CLOUD_SPACE_TXT 202
#define IDC_CLOUD_SPACE_BAR 203

#define IDC_SET_TIME        300
#define IDC_START           301
#define IDC_SETTING         302

#define IDC_TOUPLOAD        400
#define IDC_TOUPLOAD_NR     401
#define IDC_TOUPLOAD_SIZE   402

#define IDC_UPLOADED        410
#define IDC_UPLOADED_NR     411
#define IDC_UPLOADED_SIZE   412

#define IDC_WORK_STATE      420

#define IDC_STOP            430

#define ICON_SIZE_W         80
#define ICON_SIZE_H         60

#define ICON_PULLUP_H       30

#define MARGIN_H            10
#define MARGIN_V            10

#define SPACING_V           10

#define TEXT_SUBJECT_H      30
#define TEXT_NUMBER_H       40
#define TEXT_SIZE_H         20

#define IDE_UPLOAD_TIME  (IDE_MIN + 0)

static MENU_ENTRY _menu_entries [] = {
    { IDE_UPLOAD_TIME,  TYPE_MENU_ENTRY_ACT,    "MUCTimeActivity", },
};

#define IDE_TIME_1H      (IDE_MIN + 0)
#define IDE_TIME_2H      (IDE_MIN + 1)
#define IDE_TIME_4H      (IDE_MIN + 2)
#define IDE_TIME_8H      (IDE_MIN + 3)
#define IDE_TIME_ALL     (IDE_MIN + 4)

MENU_ENTRY g_menu_entries_muc_time [] = {
    { IDE_TIME_1H,   TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("in 1 hour"), SWITCH_ON},
    { IDE_TIME_2H,   TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("in 2 hours"), SWITCH_OFF},
    { IDE_TIME_4H,   TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("in 4 hours"), SWITCH_OFF},
    { IDE_TIME_8H,   TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("in 8 hours"), SWITCH_OFF},
    { IDE_TIME_ALL,  TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("All photos"), SWITCH_OFF},
};

int g_nr_entries_muc_time = TABLESIZE (g_menu_entries_muc_time);

static void update_menu_entries_muc_time (int duration)
{
    int idx_on = IDE_TIME_ALL - IDE_MIN;

    for (int i = 0; i < g_nr_entries_muc_time; i++) {
        g_menu_entries_muc_time [i].value = SWITCH_OFF;
    }

    switch (duration) {
    case 3600:
        idx_on = 0;
        break;
    case 3600*2:
        idx_on = 1;
        break;
    case 3600*4:
        idx_on = 2;
        break;
    case 3600*8:
        idx_on = 3;
        break;
    default:
        break;
    }

    g_menu_entries_muc_time [idx_on].value = SWITCH_ON;
}

static mMenuListPiece* _listpiece;
static LRESULT mymain_onReady (mMainWnd* self, UINT msg, DWORD wparam, DWORD lparam)
{
    if (wparam == ACTIVITY_POPPED) {
        MUCActivity* act;
        act = (MUCActivity*)GetWindowAdditionalData (self->hwnd);
        switch (lparam) {
        case 0:
            act->m_duration = 3600;
            break;
        case 1:
            act->m_duration = 3600 * 2;
            break;
        case 2:
            act->m_duration = 3600 * 4;
            break;
        case 3:
            act->m_duration = 3600 * 8;
            break;
        default:
            act->m_duration = -1;
            break;
        }

        _menu_entries [IDE_UPLOAD_TIME - IDE_MIN].value = _(g_menu_entries_muc_time [lparam].name);
        _c(_listpiece)->reloadData (_listpiece);
    }

    return 0;
}

static BOOL mypage_onCommand (mWidget* self, int id, int nc, HWND hCtrl)
{
    HWND hmainwin = GetMainWindowHandle (hCtrl);
    MUCActivity* act = (MUCActivity*)GetWindowAdditionalData (hmainwin);

    if (id == IDC_SET_TIME && (nc + IDE_MIN) == IDE_UPLOAD_TIME) {
        update_menu_entries_muc_time (act->m_duration);
        ACTIVITYSTACK->push (_menu_entries[nc].activity);
    }
    else if (nc == NCSN_WIDGET_CLICKED) {
        mPropSheet *propsheet = (mPropSheet*)ncsGetChildObj (hmainwin, IDC_CONTENT);

        switch (id) {
        case IDC_START:
            MISCSERVICE->startMUC (hmainwin, act->m_duration, act->m_filter);
            _c(propsheet)->setProperty (propsheet, NCSP_PRPSHT_ACTIVEPAGEIDX, PAGE_UPLOAD_STATE);
            break;

        case IDC_STOP:
            MISCSERVICE->stopMUC (hmainwin);
            _c(propsheet)->setProperty (propsheet, NCSP_PRPSHT_ACTIVEPAGEIDX, PAGE_SET_AND_START);
            break;

        case IDC_SETTING:
            ACTIVITYSTACK->push ("MUCSettingsActivity");
            break;
        }
    }

    return TRUE;
}

static NCS_WND_TEMPLATE _set_and_start_ctrls [] = {
    {
        class_name: NCSCTRL_CONTAINERCTRL,
        id: IDC_SET_TIME,
        x: 0,
        y: 0,
        w: ACTIVITY_ITEM_WITH_BRIEF_W,
        h: ACTIVITY_ITEM_WITH_BRIEF_H,
        style: WS_VISIBLE | NCSS_NOTIFY,
        ex_style: WS_EX_NONE,
        caption: "",
    },
    {
        class_name: NCSCTRL_BUTTON,
        id: IDC_START,
        x: 50,
        y: ACTIVITY_ITEM_WITH_BRIEF_H + ACTIVITY_CONTENT_MARGIN_V * 2,
        w: ACTIVITY_W - 50 - 50,
        h: ACTIVITY_CONTENT_ITEM_H,
        style: WS_VISIBLE | NCSS_NOTIFY,
        ex_style: WS_EX_NONE,
        caption: "",
    },
#ifndef OEM_TPP
    {
        class_name: NCSCTRL_IMAGE,
        id: IDC_SETTING,
        x: 0,
        y: ACTIVITY_H - ICON_SIZE_H - ICON_PULLUP_H,
        w: ACTIVITY_CONTENT_ITEM_W,
        h: ICON_PULLUP_H,
        style: WS_VISIBLE | NCSS_NOTIFY,
        ex_style: WS_EX_NONE,
        caption: "",
    },
#endif
};

static void init_page_set_and_start (mPage* self)
{
    for (size_t i = 0; i < TABLESIZE (_set_and_start_ctrls); i++) {
        ncsCreateWindowIndirect (_set_and_start_ctrls + i, self->hPanel);
    }

    mWidget* ctrl;
    ctrl = (mWidget*)(_c(self)->getChild (self, IDC_SET_TIME));
    if (ctrl) {
        RECT rc = {0, 0, ACTIVITY_ITEM_WITH_BRIEF_W, ACTIVITY_ITEM_WITH_BRIEF_H};

        _menu_entries [IDE_UPLOAD_TIME - IDE_MIN].name = _("Upload Photos");
        _menu_entries [IDE_UPLOAD_TIME - IDE_MIN].value = _("in 1 hour");
        _listpiece = createMenuListView ((mContainerCtrl*)ctrl, &rc, _menu_entries, TABLESIZE (_menu_entries));
    }

    ctrl = (mWidget*)(_c(self)->getChild (self, IDC_START));
    if (ctrl)
        SetWindowText (ctrl->hwnd, _("Start"));

    ctrl = (mWidget*)(_c(self)->getChild (self, IDC_SETTING));
    if (ctrl)
        _c(ctrl)->setProperty (ctrl, NCSP_IMAGE_IMAGE, (DWORD)GetResource (Str2Key (GLOBAL_IMG_ARROW_UP)));
}

static NCS_WND_TEMPLATE _upload_state_ctrls [] = {
    {
        class_name: NCSCTRL_STATIC,
        id: IDC_TOUPLOAD,
        x: MARGIN_H,
        y: 0,
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
        y: TEXT_SUBJECT_H,
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
        y: TEXT_SUBJECT_H + TEXT_NUMBER_H,
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
        y: 0,
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
        y: TEXT_SUBJECT_H,
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
        y: TEXT_SUBJECT_H + TEXT_NUMBER_H,
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
        y: TEXT_SUBJECT_H + TEXT_NUMBER_H + TEXT_SIZE_H,
        w: ACTIVITY_W - ACTIVITY_CONTENT_MARGIN_H * 2,
        h: TEXT_SIZE_H,
        style: WS_NONE, // WS_VISIBLE,      VincentWei: hide this widget temporarily.
        ex_style: WS_EX_NONE,
        caption: "",
    },
#endif
    {
        class_name: NCSCTRL_BUTTON,
        id: IDC_STOP,
        x: 50,
        y: TEXT_SUBJECT_H + TEXT_NUMBER_H + TEXT_SIZE_H + TEXT_SIZE_H + ACTIVITY_CONTENT_MARGIN_V,
        w: ACTIVITY_W - 50 - 50,
        h: 30,
        style: WS_VISIBLE | NCSS_NOTIFY,
        ex_style: WS_EX_NONE,
        caption: "",
    },
};

static void init_page_upload_state (mPage* self)
{
    for (size_t i = 0; i < TABLESIZE (_upload_state_ctrls); i++) {
        ncsCreateWindowIndirect (_upload_state_ctrls + i, self->hPanel);
    }

    mWidget* ctrl;
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
        SetWindowText (ctrl->hwnd, _("Preparing…"));
        ncsSetElement (ctrl, NCS_FGC_WINDOW, COLOR_STATE_NORMAL);
        ncsSetFont (ctrl->hwnd, GLOBAL_FONT_SM);
    }

    ctrl = (mWidget*)(_c(self)->getChild (self, IDC_STOP));
    if (ctrl) {
        SetWindowText (ctrl->hwnd, _("Stop"));
    }
}

static void mypage_onInitPage (mPage* self, DWORD add_data)
{
    switch (add_data) {
        case PAGE_SET_AND_START:
            init_page_set_and_start (self);
            break;

        case PAGE_UPLOAD_STATE:
            init_page_upload_state (self);
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
        _ERR_PRINTF ("MUCActivity: failed to get content (property sheet) control.\n");
        return FALSE;
    }

    mPage* page;

    page_info.caption = "Set and Start";
    page_info.dwAddData = PAGE_SET_AND_START;
    page = _c(propsheet)->addPage (propsheet, &page_info, mypage_handlers);
    SetWindowBkColor (((mWidget*)page)->hwnd, PIXEL_black);

    page_info.caption = "Uploading";
    page_info.dwAddData = PAGE_UPLOAD_STATE;
    page = _c(propsheet)->addPage (propsheet, &page_info, mypage_handlers);
    SetWindowBkColor (((mWidget*)page)->hwnd, PIXEL_black);

    return TRUE;
}

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
        _ERR_PRINTF ("MUCActivity: failed to load resource.\n");
        return FALSE;
    }

    return TRUE;
}

static void unload_res (void)
{
    unloadResByTag (_myres_list, 0);
}

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
        _ERR_PRINTF ("MUCActivity: bad cloud space data (%f/%f).\n", space_free, space_total);
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

    for (int i =0; i < g_nr_entries_muc_time; i++) {
        g_menu_entries_muc_time [i].name = _(g_menu_entries_muc_time [i].name);
    }

    init_content (self);

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
    if (nc == NCSN_WIDGET_CLICKED) {
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

        mPropSheet* propsheet = (mPropSheet*)ncsGetChildObj (self->hwnd, IDC_CONTENT);
        mPage* page = (mPage*)(_c(propsheet)->getPageByIndex (propsheet, PAGE_UPLOAD_STATE));
        ctrl = (mWidget*)(_c(page)->getChild (page, IDC_TOUPLOAD_NR));
        if (ctrl) {
            char buff [16];
            snprintf (buff, 15, "%d", toupload_nr);
            SetWindowText (ctrl->hwnd, buff);
        }
        ctrl = (mWidget*)(_c(page)->getChild (page, IDC_TOUPLOAD_SIZE));
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

        mPropSheet* propsheet = (mPropSheet*)ncsGetChildObj (self->hwnd, IDC_CONTENT);
        mPage* page = (mPage*)(_c(propsheet)->getPageByIndex (propsheet, PAGE_UPLOAD_STATE));
        ctrl = (mWidget*)(_c(page)->getChild (page, IDC_UPLOADED_NR));
        if (ctrl) {
            char buff [16];
            snprintf (buff, 15, "%d", uploaded_nr);
            SetWindowText (ctrl->hwnd, buff);
        }
        ctrl = (mWidget*)(_c(page)->getChild (page, IDC_UPLOADED_SIZE));
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

static LRESULT mymain_onAsyncTaskFinished (mMainWnd* self, UINT msg, DWORD wparam, DWORD lparam)
{
    MUCActivity* act;
    act = (MUCActivity*)GetWindowAdditionalData (self->hwnd);

    switch (wparam) {
    case TASKID_MISC_START_MUC: {
        _DBG_PRINTF ("CBPlusGUI>main: async task TASKID_MISC_START_MUC finished: %d\n", lparam);
        act->m_started = true;
        break;
    }
    case TASKID_MISC_STOP_MUC: {
        _DBG_PRINTF ("CBPlusGUI>main: async task TASKID_MISC_STOP_MUC finished\n");
        if (act->m_stopped) {
            ACTIVITYSTACK->pop();
        }
        break;
    }
    default:
        _DBG_PRINTF ("CBPlusGUI>main: Unhandled async task: %d\n", wparam);
        break;
    }

    return 0;
}

static NCS_EVENT_HANDLER mymain_handlers [] = {
    {MSG_USER_APP_DATA, reinterpret_cast<void*>(mymain_onIntent)},
    {MSG_COMMAND, reinterpret_cast<void*>(mymain_onCommand)},
    {MSG_USER_APP_READY, reinterpret_cast<void*>(mymain_onReady)},
    {MSG_CLOUD_CHANGED, reinterpret_cast<void*>(mymain_onSateChanged)},
    {MSG_UPLOAD_CHANGED, reinterpret_cast<void*>(mymain_onSateChanged)},
    {MSG_TOUPLOAD_CHANGED, reinterpret_cast<void*>(mymain_onSateChanged)},
    {MSG_UPLOADED_CHANGED, reinterpret_cast<void*>(mymain_onSateChanged)},
    {MSG_ASYNC_TASK_FINISHED, reinterpret_cast<void*>(mymain_onAsyncTaskFinished)},
    {MSG_CLOSE, reinterpret_cast<void*>(mymain_onClose)},
    {MSG_LBUTTONDOWN, reinterpret_cast<void*>(SpeedMeterMessageHandler)},
    {MSG_LBUTTONUP, reinterpret_cast<void*>(SpeedMeterMessageHandler)},
    {MSG_MOUSEMOVE, reinterpret_cast<void*>(SpeedMeterMessageHandler)},
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
        caption: "手动上传云端",
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
        class_name: NCSCTRL_PROPSHEET,
        id: IDC_CONTENT,
        x: -2,
        y: ICON_SIZE_H,
        w: ACTIVITY_CONTENT_W + 4,
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
    N_("Manual Upload Cloud"),
    NULL,
    NULL,
    mymain_handlers,
    _ctrl_templ,
    TABLESIZE(_ctrl_templ),
    0, 0, NULL, 0, 0,
    &create_notify_info,
    0
};

int MUCActivity::onStart ()
{
    MISCSERVICE->startFunc (GET_GLOBAL_HOSTING, APP_MANUAL_UPLOAD_CLOUD);
    return 0;
}

int MUCActivity::onCancel ()
{
    if (messageScreen (m_hwnd, _("Please Confirm"), _("Exit the current work mode?"),
            MB_OKCANCEL | MS_AUTOCANCEL) == IDCANCEL)
        return 1;

    if (m_started && !m_stopped) {
        MISCSERVICE->stopMUC (this->hwnd());
        m_stopped = true;
    }
    else {
        ACTIVITYSTACK->pop();
    }

    return 0;
}

int MUCActivity::onStop ()
{
    MISCSERVICE->stopFunc (GET_GLOBAL_HOSTING, APP_MANUAL_UPLOAD_CLOUD);
    return 0;
}

MUCActivity::MUCActivity () : NCSActivity (&mymain_templ, true)
{
    m_duration = 3600;
    m_filter = PHOTO_FILTER_JPEG;
    m_started = false;
    m_stopped = false;

    m_style = STYLE_ZOOM;
}

