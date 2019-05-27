/*!============================================================================
 * @file SettingResetActivity.cc
 * @Synopsis The activity for WiFi password.
 * @author Vincent Wei
 * @version 1.0
 * @date 23/05/2018
 *
 *  Copyright (C) 2018 Beijing Joobot Technogolies Inc.
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

#include "resource.h"

#include "SysConfService.hh"
#include "SettingResetActivity.hh"

#define PAGE_CONFIRM_RESET          0
#define PAGE_DO_RESET               1
#define PAGE_PROMPT_LOWBATTERY      2
#define PAGE_PROMPT_ERROR           3

#define IDC_DESC                    300
#define IDC_PROGRESS                301
#define IDC_STATE                   302

REGISTER_ACTIVITY(SettingResetActivity);

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
    int* status = (int*)lparam;
    mPropSheet* propsheet = (mPropSheet*)ncsGetChildObj (self->hwnd, IDC_CONTENT);

    switch (*status) {
    case SYS_RESET_OK:
        // will not be here.
        break;

    case SYS_RESET_ERR_LOWPOWER:
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
    _MG_PRINTF ("SettingResetActivity: mymain_onClose called.\n");
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
    if (idx_page == PAGE_CONFIRM_RESET) {
        if (id == IDYES) {
            _c(propsheet)->setProperty (propsheet, NCSP_PRPSHT_ACTIVEPAGEIDX, PAGE_DO_RESET);
            SYSCFGSERVICE->resetDevice(hmainwin);
        }
        else if (id == IDNO) {
            ACTIVITYSTACK->pop ();
        }
    }

    return TRUE;
}

static void init_page_confirm_reset (mPage* self)
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
        caption: _("Do you want to reset this CBPlus? All configuration data will be lost."),
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

static void init_page_do_reset (mPage* self)
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
        caption: _("Do not power off the device. Your CBPlus will reboot automatically."),
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
        y: ctrl_templ_prompt.y + ctrl_templ_prompt.h + 30,
        w: ACTIVITY_CONTENT_W,
        h: 30,
        style: WS_VISIBLE | NCSS_ANMT_AUTOLOOP | NCSS_ANMT_AUTOPLAY,
        ex_style: WS_EX_NONE,
        caption: "",
    };

    ctrl_templ_loading.props = props_loading;
    ncsCreateWindowIndirect (&ctrl_templ_loading, self->hPanel);

#if 0
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
#endif
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
        caption: _("The battery is low. Reset was canceled."),
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
        caption: _("Failed to reset device due to system error. Please try again."),
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
    case PAGE_CONFIRM_RESET:
        init_page_confirm_reset (self);
        break;
    case PAGE_DO_RESET:
        init_page_do_reset (self);
        break;
    case PAGE_PROMPT_LOWBATTERY:
        init_page_prompt_lowbattery (self);
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
    SettingResetActivity* act;

    act = (SettingResetActivity*)GetWindowAdditionalData (hmain);
    if (show_cmd == SW_SHOW) {
        switch (page_id) {
        case PAGE_CONFIRM_RESET:
        case PAGE_PROMPT_LOWBATTERY:
        case PAGE_PROMPT_ERROR:
            act->setAutoEscape (true);
            break;

        case PAGE_DO_RESET: {
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

#if 0
        mWidget* subtitle = (mWidget*)ncsGetChildObj (hmain, IDC_SUBTITLE);
        if (subtitle)
            SetWindowText (subtitle->hwnd, GetWindowCaption (((mWidget*)self)->hwnd));
#endif
    }
    else {
        switch (page_id) {
        case PAGE_DO_RESET: {
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
        _ERR_PRINTF ("SettingResetActivity: failed to get content (property sheet) control.\n");
        return FALSE;
    }

    mPage* page;

    page_info.caption = _("Confirm to Reset");
    page_info.dwAddData = PAGE_CONFIRM_RESET;
    page = _c(propsheet)->addPage (propsheet, &page_info, mypage_handlers);
    SetWindowBkColor (((mWidget*)page)->hwnd, PIXEL_black);

    page_info.caption = _("Reset…");
    page_info.dwAddData = PAGE_DO_RESET;
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
        _ERR_PRINTF ("SettingResetActivity: failed to load resource.\n");
        return FALSE;
    }

    mWidget* ctrl;

    ctrl = (mWidget*)(_c(self)->getChild (self, IDC_TITLE));
    ncsSetElement (ctrl, NCS_FGC_WINDOW, COLOR_TITLE);
    ncsSetFont (ctrl->hwnd, FONT_TITLE);
    SetWindowText (ctrl->hwnd, _("Reset CBPlus"));

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
    N_("Reset CBPlus"),
    NULL,
    NULL,
    mymain_handlers,
    _ctrl_templ,
    TABLESIZE(_ctrl_templ),
    0, 0, NULL, 0, 0,
    &create_notify_info,
    0
};

SettingResetActivity::SettingResetActivity () :
    NCSActivity (&mymain_templ)
{
    m_style = STYLE_PUSH;
}

