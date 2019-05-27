/*!============================================================================
 * @file SettingCheckNetworkActivity.cc 
 * @Synopsis The language activity of Setting app. 
 * @author Vincent Wei
 * @version 1.0
 * @date 19/05/2018
 *
 *  Copyright (C) 2018 Beijing Joobot Technogolies Inc.
 */

#include "global.h"
#include "utilities.h"
#include "resource.h"

#include "Activity.hh"
#include "ActivityStack.hh"
#include "NCSActivity.hh"

#include "NetworkService.hh"
#include "SettingCheckNetworkActivity.hh"

REGISTER_ACTIVITY(SettingCheckNetworkActivity);

static LRESULT mymain_onIntent (mMainWnd* self, UINT msg, DWORD wparam, DWORD lparam)
{
    Intent* intent = (Intent*)wparam;
    SettingCheckNetworkActivity* act;

    act = (SettingCheckNetworkActivity*)GetWindowAdditionalData (self->hwnd);
    if (strcasecmp (intent->getString ("firstBoot").c_str(), "yes") == 0) {
        act->m_firstBoot = true;
    }

    Intent::deleteIntent (intent);
    return 0;
}

static LRESULT mymain_onReady (mMainWnd* self, UINT msg, DWORD wparam, DWORD lparam)
{
    if (wparam == ACTIVITY_PUSHED) {
        NETWORKSERVICE->getInstantState (self->hwnd);
    }
    else if (wparam == ACTIVITY_POPPED) {
        NETWORKSERVICE->getInstantState (self->hwnd);
    }

    return 0;
}

static LRESULT mymain_onAsyncTaskFinished (mMainWnd* self, UINT msg, DWORD wparam, DWORD lparam)
{
    SettingCheckNetworkActivity* act;
    act = (SettingCheckNetworkActivity*)GetWindowAdditionalData (self->hwnd);

    NETWORK_STATE* ns = (NETWORK_STATE*)lparam;
    /* save the new state to NETWORKSERVICE */
    NETWORKSERVICE->onStateChanged (ns);

    Intent* my_intent = Intent::newIntent ();
    if (act->m_firstBoot) {
        my_intent->putExtra ("firstBoot", std::string ("yes"));
        if (ns->global == NETWORK_STATE_CONNECTED) {
            ACTIVITYSTACK->push ("SettingPairActivity", my_intent);
        }
        else {
            char mobile_state [4];
            sprintf (mobile_state, "%d", ns->lte);
            my_intent->putExtra ("mobileState", std::string (mobile_state));
            ACTIVITYSTACK->push ("SettingCheckNetworkPromptActivity", my_intent);
        }
    }
    else {
        _MG_PRINTF ("SettingCheckNetworkActivity > mymain_onAsyncTaskFinished: not in the first boot\n");
    }

    return 0;
}

#define RES_CN_MAIN         "images/check-network/checking-network.png"

static RES_NODE _myres_list [] = {
    /* images on status bar */
    {RES_CN_MAIN,       RES_TYPE_IMAGE, 16, 0},

    /* terminator */
    {NULL}
};

static mWidget* create_title (mMainWnd* self)
{
    NCS_WND_TEMPLATE ctrl_templ = {
        class_name: NCSCTRL_STATIC,
        id: IDC_TITLE,
        x: ACTIVITY_TITLE_X,
        y: ACTIVITY_TITLE_Y,
        w: ACTIVITY_TITLE_W,
        h: ACTIVITY_TITLE_H,
        style: WS_VISIBLE,
        ex_style: WS_EX_NONE,
    };

    ctrl_templ.caption = _("Checking Network…");

    mWidget* ctrl_title = ncsCreateWindowIndirect (&ctrl_templ, self->hwnd);

    if (ctrl_title) {
        ncsSetElement (ctrl_title, NCS_FGC_WINDOW, COLOR_TITLE);
        ncsSetFont (ctrl_title->hwnd, FONT_TITLE);
    }

    return ctrl_title;
}

static mWidget* create_content (mMainWnd* self)
{
    NCS_PROP_ENTRY props [] = {
        {NCSP_IMAGE_IMAGE, (DWORD)GetResource (Str2Key (RES_CN_MAIN))},
        {-1, 0},
    };

    NCS_WND_TEMPLATE ctrl_templ = {
        class_name: NCSCTRL_IMAGE,
        id: IDC_CONTENT,
        x: ACTIVITY_CONTENT_X,
        y: ACTIVITY_CONTENT_Y,
        w: ACTIVITY_CONTENT_W,
        h: ACTIVITY_CONTENT_H,
        style: WS_VISIBLE,
        ex_style: WS_EX_NONE,
        caption: "",
    };

    ctrl_templ.props = props;
    return ncsCreateWindowIndirect (&ctrl_templ, self->hwnd);
}

static mWidget* create_loading (mMainWnd* self)
{
    char gif_path [MAX_PATH + 1];
    strcpy (gif_path, GetResPath ());
    strcat (gif_path, "/");
    strcat (gif_path, RES_GLOBAL_LOADING_SM);

    NCS_PROP_ENTRY props [] = {
        {NCSP_ANMT_GIFFILE, (DWORD)gif_path},
        {NCSP_ANMT_INTERVAL, (DWORD)10},
        {-1, 0},
    };

    NCS_WND_TEMPLATE ctrl_templ = {
        class_name: NCSCTRL_ANIMATE,
        id: IDC_STATIC,
        x: ACTIVITY_W - 40,
        y: ACTIVITY_H - 40,
        w: 28,
        h: 28,
        style: WS_VISIBLE | NCSS_ANMT_AUTOPLAY | NCSS_ANMT_AUTOLOOP,
        ex_style: WS_EX_NONE,
        caption: "",
    };

    ctrl_templ.props = props;
    return ncsCreateWindowIndirect (&ctrl_templ, self->hwnd);
}

static BOOL mymain_onCreate (mMainWnd* self, DWORD dwAddData)
{
    unsigned cnt;

    cnt = loadResByTag (_myres_list, 0);
    
    if (cnt < (TABLESIZE (_myres_list) - 1)) {
		_ERR_PRINTF ("SettingCheckNetworkActivity: failed to load resource.\n");
        return FALSE;
    }

    if (create_title (self) == NULL) {
		_ERR_PRINTF ("SettingCheckNetworkActivity: failed to create title.\n");
        return FALSE;
    }

    if (create_content (self) == NULL) {
		_ERR_PRINTF ("SettingCheckNetworkActivity: failed to create content.\n");
        return FALSE;
    }

    if (create_loading (self) == NULL) {
		_ERR_PRINTF ("SettingCheckNetworkActivity: failed to create loading animation.\n");
        return FALSE;
    }

    return TRUE;
}

static BOOL mymain_onCreated (struct _NCS_CREATE_NOTIFY_INFO* info, mComponent* self, DWORD special_id)
{
    return TRUE;
}

static BOOL mymain_onClose (mWidget* _this, int message)
{
    _MG_PRINTF ("SettingCheckNetworkActivity: mymain_onClose called.\n");

    unloadResByTag (_myres_list, 0);
    return TRUE;
}

static NCS_CREATE_NOTIFY_INFO create_notify_info = {
    onCreated: mymain_onCreated,
};

static NCS_WND_TEMPLATE _ctrl_templ[] =
{
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

static NCS_EVENT_HANDLER mymain_handlers [] = {
    {MSG_CREATE, reinterpret_cast<void*>(mymain_onCreate)},
    {MSG_USER_APP_DATA, reinterpret_cast<void*>(mymain_onIntent)},
    {MSG_USER_APP_READY, reinterpret_cast<void*>(mymain_onReady)},
    {MSG_ASYNC_TASK_FINISHED, reinterpret_cast<void*>(mymain_onAsyncTaskFinished)},
    {MSG_CLOSE, reinterpret_cast<void*>(mymain_onClose)},
    {0, NULL}
};

static NCS_MNWND_TEMPLATE mymain_templ = {
    NCSCTRL_DIALOGBOX, 
    1,
    ACTIVITY_X, ACTIVITY_Y, ACTIVITY_W, ACTIVITY_H,
    WS_NONE, WS_EX_NONE,
    N_("Checking Network"),
    NULL,
    NULL,
    mymain_handlers,
    _ctrl_templ,
    TABLESIZE(_ctrl_templ),
    0, 0, NULL, 0, 0,
    &create_notify_info,
    0
};

SettingCheckNetworkActivity::SettingCheckNetworkActivity() :
    NCSActivity (&mymain_templ)
{ 
    m_style = STYLE_PUSH;
    m_firstBoot = false;
}

SettingCheckNetworkActivity::~SettingCheckNetworkActivity()
{
}

