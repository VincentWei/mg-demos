/*!============================================================================
 * @file SettingPairActivity.cc 
 * @Synopsis The activity for WiFi password.
 * @author Vincent Wei
 * @version 1.0
 * @date 23/05/2018
 *
 *  Copyright (C) 2018 Beijing Joobot Technogolies Inc.
 */

#include "global.h"
#include "utilities.h"
#include "misc.h"

#include "Activity.hh"
#include "ActivityStack.hh"
#include "NCSActivity.hh"

#define _HAVE_TITLE  1
#include "resource.h"

#include "PairService.hh"
#include "SysConfService.hh"
#include "SettingPairActivity.hh"

#define PAGE_PAIR_CHECK             0
#define PAGE_PAIR_FINISHED          1
#define PAGE_PROMPT_NETWORK_ERROR   2
#define PAGE_PROMPT_TIMEOUT         3

#define IDC_PAIR_QRCODE             1001
#define IDC_CHECK_NETWORK           1002

REGISTER_ACTIVITY(SettingPairActivity);

#define RES_GLOBAL_CBPLUS               "images/cbplus.png"
#define RES_GLOBAL_FINISHED             "images/task-finished.png"

static RES_NODE _myres_list [] = {
    /* images on status bar */
    {RES_GLOBAL_CBPLUS,         RES_TYPE_IMAGE, 16, 0},
    {RES_GLOBAL_FINISHED,       RES_TYPE_IMAGE, 16, 0},

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
    SettingPairActivity* act;
    act = (SettingPairActivity*)GetWindowAdditionalData (self->hwnd);

    if (strcasecmp (intent->getString ("firstBoot").c_str(), "yes") == 0) {
        act->m_firstBoot = true;
    }

    Intent::deleteIntent (intent);
    return 0;
}

static LRESULT mymain_onAsyncTaskFinished (mMainWnd* self, UINT msg, DWORD wparam, DWORD lparam)
{
    PAIR_INFO* info = (PAIR_INFO*)lparam;
    mPropSheet* propsheet = (mPropSheet*)ncsGetChildObj (self->hwnd, IDC_CONTENT);

    switch (wparam) {
    case TASKID_PAIR_GETCODE: {
        if (info->status == PS_GETCODE_OK) {
            /* display the pairing QRCode here */
            mPage* page = (mPage*)(_c(propsheet)->getPageByIndex (propsheet, PAGE_PAIR_CHECK));
            mImage* image = (mImage*)ncsObjFromHandle (GetDlgItem (page->hwnd, IDC_PAIR_QRCODE));
            SetWindowBkColor (image->hwnd, PIXEL_lightwhite);
            _c(image)->setProperty (image, NCSP_IMAGE_IMAGEFILE, (DWORD)info->qrcode_path);

            /* change subtitle */
            mWidget* title = (mWidget*)ncsGetChildObj (self->hwnd, IDC_TITLE);
            SetWindowText (title->hwnd, _("Scan QRCode to pair"));

            PAIRSERVICE->executePair (self->hwnd, info->code);

            SettingPairActivity* act;
            act = (SettingPairActivity*)GetWindowAdditionalData (self->hwnd);
            act->setAutoEscape (false);
            return 0;
        }
        else {
            _c(propsheet)->setProperty (propsheet, NCSP_PRPSHT_ACTIVEPAGEIDX, PAGE_PROMPT_NETWORK_ERROR);
        }

        break;
    }

    case TASKID_PAIR_EXECUTE: {
        if (info->status == PS_EXECUTE_OK) {
#if 0
            SYSCFGSERVICE->setBoundUserId (info->user_id);
            SYSCFGSERVICE->setBoundUserNickname (info->nickname);
#else
            SYSCFGSERVICE->refreshUserInfo ();
#endif
            _c(propsheet)->setProperty (propsheet, NCSP_PRPSHT_ACTIVEPAGEIDX, PAGE_PAIR_FINISHED);
        }
        else if (info->status == PS_EXECUTE_TIMEOUT) {
            _c(propsheet)->setProperty (propsheet, NCSP_PRPSHT_ACTIVEPAGEIDX, PAGE_PROMPT_TIMEOUT);
        }
        else {
            _c(propsheet)->setProperty (propsheet, NCSP_PRPSHT_ACTIVEPAGEIDX, PAGE_PROMPT_NETWORK_ERROR);
        }

        break;
    }
    }

    SettingPairActivity* act;
    act = (SettingPairActivity*)GetWindowAdditionalData (GetMainWindowHandle (self->hwnd));
    if (!act->m_firstBoot) {
        act->setAutoEscape (true);
    }
    return 0;
}

static BOOL mymain_onClose (mMainWnd* self, int message)
{
    _MG_PRINTF ("SettingPairActivity: mymain_onClose called.\n");
    unloadResByTag (_myres_list, 0);
    return TRUE;
}

static BOOL my_onCommand (mWidget* self, int id, int nc, HWND hCtrl)
{
    HWND hmainwin = GetMainWindowHandle (hCtrl);
    mPropSheet *propsheet = (mPropSheet*)ncsGetChildObj (hmainwin, IDC_CONTENT);
    SettingPairActivity* act;
    act = (SettingPairActivity*)GetWindowAdditionalData (GetMainWindowHandle (self->hwnd));

    if (nc == NCSN_WIDGET_CLICKED) {
        switch (id) {
        case IDOK:
            if (act->m_firstBoot) {
                _MG_PRINTF ("Settings for the first boot finished. Now start the launcher.\n");
                while (TRUE) {
                    if (ACTIVITYSTACK->depth () == 0) {
                        break;
                    }

                    ACTIVITYSTACK->pop ();
                }

                ACTIVITYSTACK->push ("LauncherActivity");
            }
            else {
                ACTIVITYSTACK->pop ();
            }
            break;

        case IDRETRY:
            _c(propsheet)->setProperty (propsheet, NCSP_PRPSHT_ACTIVEPAGEIDX, PAGE_PAIR_CHECK);
            break;

        case IDC_CHECK_NETWORK:
            Intent* my_intent = Intent::newIntent ();
            my_intent->putExtra ("firstBoot", std::string ("yes"));
            ACTIVITYSTACK->switchTo ("SettingCheckNetworkActivity", my_intent);
            break;
        }
    }

    return TRUE;
}

static void init_page_pair_check (mWidget* self)
{
    /* CBPlus image */
    NCS_PROP_ENTRY props_qrcode [] = {
        {NCSP_IMAGE_IMAGE, (DWORD)GetResource (Str2Key (RES_GLOBAL_CBPLUS))},
        {-1, 0},
    };

    NCS_WND_TEMPLATE ctrl_templ_qrcode = {
        class_name: NCSCTRL_IMAGE,
        id: IDC_PAIR_QRCODE,
        x: 0,
        y: 0,
        w: ACTIVITY_CONTENT_W,
        h: ACTIVITY_CONTENT_H,
        style: WS_VISIBLE,
        ex_style: WS_EX_NONE,
        caption: "QRCode",
    };

    ctrl_templ_qrcode.props = props_qrcode;
    ncsCreateWindowIndirect (&ctrl_templ_qrcode, self->hwnd);
}

static void init_page_pair_finished (mWidget* self)
{
    /* Task finished image */
    NCS_PROP_ENTRY props_finished [] = {
        {NCSP_IMAGE_IMAGE, (DWORD)GetResource (Str2Key (RES_GLOBAL_FINISHED))},
        {-1, 0},
    };

    NCS_WND_TEMPLATE ctrl_templ_finished = {
        class_name: NCSCTRL_IMAGE,
        id: IDC_STATIC,
        x: 0,
        y: ACTIVITY_CONTENT_MARGIN_V,
        w: ACTIVITY_CONTENT_W,
        h: 100 + ACTIVITY_CONTENT_MARGIN_V + ACTIVITY_CONTENT_MARGIN_V,
        style: WS_VISIBLE,
        ex_style: WS_EX_NONE,
        caption: "",
    };

    ctrl_templ_finished.props = props_finished;
    ncsCreateWindowIndirect (&ctrl_templ_finished, self->hwnd);

    /* ok button */
    NCS_WND_TEMPLATE ctrl_templ_ok = {
        class_name: NCSCTRL_BUTTON,
        id: IDOK,
        x: 50,
        y: ctrl_templ_finished.y + ctrl_templ_finished.h,
        w: ACTIVITY_W - 50 - 50,
        h: ACTIVITY_CONTENT_ITEM_H,
        style: WS_VISIBLE | NCSS_NOTIFY,
        ex_style: WS_EX_NONE,
        caption: _("Close"),
    };

    ncsCreateWindowIndirect (&ctrl_templ_ok, self->hwnd);
}

static void init_page_prompt_network_error (mWidget* self)
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
        caption: _("Failed to pair your CBPlus. Please check your network settings."),
    };

    ctrl_templ_prompt.props = props_prompt;
    ncsCreateWindowIndirect (&ctrl_templ_prompt, self->hwnd);

    /* ok button */
    NCS_WND_TEMPLATE ctrl_templ_ok = {
        class_name: NCSCTRL_BUTTON,
        id: IDC_CHECK_NETWORK,
        x: 50,
        y: ctrl_templ_prompt.y + ctrl_templ_prompt.h,
        w: ACTIVITY_W - 50 - 50,
        h: ACTIVITY_CONTENT_ITEM_H,
        style: WS_VISIBLE | NCSS_NOTIFY,
        ex_style: WS_EX_NONE,
        caption: _("Check Network"),
    };

    ncsCreateWindowIndirect (&ctrl_templ_ok, self->hwnd);
}

static void init_page_prompt_timeout (mWidget* self)
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
        caption: _("Please use Joopic App to scan the QRCode."),
    };

    ctrl_templ_prompt.props = props_prompt;
    ncsCreateWindowIndirect (&ctrl_templ_prompt, self->hwnd);

    /* Retry button */
    NCS_WND_TEMPLATE ctrl_templ_retry = {
        class_name: NCSCTRL_BUTTON,
        id: IDRETRY,
        x: 50,
        y: ctrl_templ_prompt.y + ctrl_templ_prompt.h,
        w: ACTIVITY_W - 50 - 50,
        h: ACTIVITY_CONTENT_ITEM_H,
        style: WS_VISIBLE | NCSS_NOTIFY,
        ex_style: WS_EX_NONE,
        caption: _("Retry"),
    };

    ncsCreateWindowIndirect (&ctrl_templ_retry, self->hwnd);
}

static void mypage_onInitPage (mWidget* self, DWORD add_data)
{
    switch (add_data) {
        case PAGE_PAIR_CHECK:           init_page_pair_check (self);            break;
        case PAGE_PAIR_FINISHED:        init_page_pair_finished (self);         break;
        case PAGE_PROMPT_NETWORK_ERROR: init_page_prompt_network_error (self);  break;
        case PAGE_PROMPT_TIMEOUT:       init_page_prompt_timeout (self);        break;
        default: break;
    }
}

static int mypage_onShowPage (mWidget* self, HWND focusChild, int show_cmd)
{
    DWORD page_id = GetWindowAdditionalData (self->hwnd);
    HWND hmain = GetMainWindowHandle (self->hwnd);
    mAnimate* ctrl_loading = (mAnimate*)(ncsGetChildObj(hmain, IDC_LOADING));

    if (show_cmd == SW_SHOW) {
        switch (page_id) {
        case PAGE_PAIR_CHECK:
        {
            ncsAnimateStart (ctrl_loading);
            ShowWindow (((mWidget*)ctrl_loading)->hwnd, SW_SHOW);
            PAIRSERVICE->getCode (hmain);

            SettingPairActivity* act;
            act = (SettingPairActivity*)GetWindowAdditionalData (hmain);
            act->setAutoEscape (false);
            break;
        }

        default:
            break;
        }

        mWidget* title = (mWidget*)ncsGetChildObj (hmain, IDC_TITLE);
        SetWindowText (title->hwnd, GetWindowCaption (((mWidget*)self)->hwnd));
    }
    else {
        switch (page_id) {
        case PAGE_PAIR_CHECK:
        {
            ncsAnimateStop (ctrl_loading);
            ShowWindow (((mWidget*)ctrl_loading)->hwnd, SW_HIDE);
            break;
        }

        default:
            break;
        }
    }

    return 1;
}

static int mypage_onSheetCmd (mWidget* self, DWORD wParam, DWORD lParam)
{
    return 0;
}

static NCS_EVENT_HANDLER mypage_handlers[] = {
    { MSG_INITPAGE, reinterpret_cast<void*>(mypage_onInitPage) },
    { MSG_SHOWPAGE, reinterpret_cast<void*>(mypage_onShowPage) },
    { MSG_COMMAND, reinterpret_cast<void*>(my_onCommand) },
    { MSG_SHEETCMD, reinterpret_cast<void*>(mypage_onSheetCmd) },
    { 0 , NULL }
};

static BOOL init_content (mComponent* self)
{
    /* init loading GIF */
    char gif_path [MAX_PATH + 1];
    strcpy (gif_path, GetResPath ());
    strcat (gif_path, "/");
    strcat (gif_path, RES_GLOBAL_LOADING_XM);

    mAnimate *animate = (mAnimate*)(_c(self)->getChild (self, IDC_LOADING));
    _c(animate)->setProperty (animate, NCSP_ANMT_GIFFILE, (DWORD)gif_path);
    _c(animate)->setProperty (animate, NCSP_ANMT_INTERVAL, (DWORD)10);

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
        _ERR_PRINTF ("SettingPairActivity: failed to get content (property sheet) control.\n");
        return FALSE;
    }

    mPage* page;

    page_info.caption = _("Checking Pair…");
    page_info.dwAddData = PAGE_PAIR_CHECK;
    page = _c(propsheet)->addPage (propsheet, &page_info, mypage_handlers);
    SetWindowBkColor (((mWidget*)page)->hwnd, PIXEL_black);

    page_info.caption = _("Paired!");
    page_info.dwAddData = PAGE_PAIR_FINISHED;
    page = _c(propsheet)->addPage (propsheet, &page_info, mypage_handlers);
    SetWindowBkColor (((mWidget*)page)->hwnd, PIXEL_black);

    page_info.caption = _("Network Error");
    page_info.dwAddData = PAGE_PROMPT_NETWORK_ERROR;
    page = _c(propsheet)->addPage (propsheet, &page_info, mypage_handlers);
    SetWindowBkColor (((mWidget*)page)->hwnd, PIXEL_black);

    page_info.caption = _("Timeout");
    page_info.dwAddData = PAGE_PROMPT_TIMEOUT;
    page = _c(propsheet)->addPage (propsheet, &page_info, mypage_handlers);
    SetWindowBkColor (((mWidget*)page)->hwnd, PIXEL_black);
    return TRUE;
}

static BOOL mymain_onCreated (struct _NCS_CREATE_NOTIFY_INFO* info, mComponent* self, DWORD special_id)
{
    unsigned cnt;
    cnt = loadResByTag (_myres_list, 0);
    
    if (cnt < (TABLESIZE (_myres_list) - 1)) {
		_ERR_PRINTF ("SettingPairActivity: failed to load resource.\n");
        return FALSE;
    }

    mWidget* ctrl;

    ctrl = (mWidget*)(_c(self)->getChild (self, IDC_TITLE));
    ncsSetElement (ctrl, NCS_FGC_WINDOW, COLOR_TITLE);
    ncsSetFont (ctrl->hwnd, FONT_TITLE);
    SetWindowText (ctrl->hwnd, _("Checking Pair…"));

    init_content (self);

    return TRUE;
}

static BOOL mymain_onCommand (mWidget* self, int id, int nc, HWND hCtrl)
{
    return TRUE;
}

static NCS_EVENT_HANDLER mymain_handlers [] = {
    {MSG_CREATE, reinterpret_cast<void*>(mymain_onCreate)},
    {MSG_USER_APP_DATA, reinterpret_cast<void*>(mymain_onIntent)},
    {MSG_COMMAND, reinterpret_cast<void*>(mymain_onCommand)},
    {MSG_ASYNC_TASK_FINISHED, reinterpret_cast<void*>(mymain_onAsyncTaskFinished)},
    {MSG_CLOSE, reinterpret_cast<void*>(mymain_onClose)},
    {0, NULL}
};

static NCS_CREATE_NOTIFY_INFO create_notify_info = {
    onCreated: mymain_onCreated,
};

static NCS_PROP_ENTRY _props_subtitle [] = {
    {NCSP_STATIC_ALIGN, NCS_ALIGN_LEFT},
    {-1, 0},
};

static NCS_WND_TEMPLATE _ctrl_templ[] =
{
    {
        class_name: NCSCTRL_STATIC,
        id: IDC_TITLE,
        x: ACTIVITY_TITLE_X + 5,
        y: ACTIVITY_TITLE_Y,
        w: ACTIVITY_TITLE_W - SIZE_LOADING_GIF_XM,
        h: ACTIVITY_TITLE_H,
        style: WS_VISIBLE,
        ex_style: WS_EX_NONE,
        caption: "",
        props: _props_subtitle,
    },
    {
        class_name: NCSCTRL_ANIMATE,
        id: IDC_LOADING,
        x: ACTIVITY_TITLE_W - SIZE_LOADING_GIF_XM - 5 - 5,
        y: ACTIVITY_TITLE_Y,
        w: SIZE_LOADING_GIF_XM,
        h: ACTIVITY_TITLE_H,
        style: WS_VISIBLE | NCSS_ANMT_AUTOLOOP | NCSS_ANMT_AUTOPLAY,
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

static NCS_MNWND_TEMPLATE mymain_templ = {
    NCSCTRL_DIALOGBOX, 
    1,
    ACTIVITY_X, ACTIVITY_Y, ACTIVITY_W, ACTIVITY_H,
    WS_NONE, WS_EX_NONE,
    N_("Pair"),
    NULL,
    NULL,
    mymain_handlers,
    _ctrl_templ,
    TABLESIZE(_ctrl_templ),
    0, 0, NULL, 0, 0,
    &create_notify_info,
    0
};

SettingPairActivity::SettingPairActivity () :
    NCSActivity (&mymain_templ)
{ 
    m_style = STYLE_ALPHA;
    m_firstBoot = false;
}

