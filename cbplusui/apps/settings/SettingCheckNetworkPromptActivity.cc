/*!============================================================================
 * @file SettingCheckNetworkPromptActivity.cc 
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
#include "SettingCheckNetworkPromptActivity.hh"

REGISTER_ACTIVITY(SettingCheckNetworkPromptActivity);

#define IDC_TEXT_NO4G   101
#define IDC_RETRY_4G    102
#define IDC_TEXT_WIFI   103
#define IDC_SETUP_WIFI  104

static BOOL mymain_onCreate (mMainWnd* self, DWORD dwAddData)
{
    return TRUE;
}

static BOOL mymain_onCreated (struct _NCS_CREATE_NOTIFY_INFO* info, mComponent* self, DWORD special_id)
{
    mWidget* ctrl;

    ctrl = (mWidget*)(_c(self)->getChild (self, IDC_TEXT_NO4G));
    _c(ctrl)->setProperty (ctrl, NCSP_STATIC_AUTOWRAP, 1);
    _c(ctrl)->setProperty (ctrl, NCSP_STATIC_VALIGN, NCS_VALIGN_BOTTOM);

    ctrl = (mWidget*)(_c(self)->getChild (self, IDC_TEXT_WIFI));
    _c(ctrl)->setProperty (ctrl, NCSP_STATIC_AUTOWRAP, 1);
    _c(ctrl)->setProperty (ctrl, NCSP_STATIC_VALIGN, NCS_VALIGN_BOTTOM);
    SetWindowText (ctrl->hwnd, _("Please set up WiFi"));

    ctrl = (mWidget*)(_c(self)->getChild (self, IDC_RETRY_4G));
    SetWindowText (ctrl->hwnd, _("Retry"));

    ctrl = (mWidget*)(_c(self)->getChild (self, IDC_SETUP_WIFI));
    SetWindowText (ctrl->hwnd, _("Setup WiFi"));

    return TRUE;
}

static LRESULT mymain_onIntent (mMainWnd* self, UINT msg, DWORD wparam, DWORD lparam)
{
    mWidget* ctrl_prompt;
    mWidget* ctrl_btn;
    Intent* intent = (Intent*)wparam;
    int mobile_state;
    const char* prompt;

    SettingCheckNetworkPromptActivity* act;
    act = (SettingCheckNetworkPromptActivity*)GetWindowAdditionalData (self->hwnd);
    if (strcasecmp (intent->getString ("firstBoot").c_str(), "yes") == 0) {
        act->m_firstBoot = true;
    }

    ctrl_prompt = (mWidget*)(_c(self)->getChild (self, IDC_TEXT_NO4G));
    ctrl_btn = (mWidget*)(_c(self)->getChild (self, IDC_RETRY_4G));

    mobile_state = atoi (intent->getString ("mobileState").c_str());
    switch (mobile_state) {
    case NETWORK_STATE_ABSENT:
        prompt = _("No 4G module equipped for this device.");
        ShowWindow (ctrl_btn->hwnd, SW_HIDE);
        break;

    case NETWORK_STATE_DISABLED:
        prompt = _("4G module is disabled.");
        ShowWindow (ctrl_btn->hwnd, SW_HIDE);
        break;

    case NETWORK_STATE_NO_MEDIA:
        prompt = _("No SIM card present.");
        break;

    default:
        prompt = _("Mobile network is unavailable.");
        break;
    }

    Intent::deleteIntent (intent);

    _c(ctrl_prompt)->setProperty (ctrl_prompt, NCSP_STATIC_AUTOWRAP, 1);
    _c(ctrl_prompt)->setProperty (ctrl_prompt, NCSP_STATIC_VALIGN, NCS_VALIGN_BOTTOM);
    SetWindowText (ctrl_prompt->hwnd, prompt);

    return 0;
}

static BOOL mymain_onCommand (mWidget* self, int id, int nc, HWND hCtrl)
{
    if (nc == NCSN_WIDGET_CLICKED) {
        switch (id) {
        case IDC_RETRY_4G: {
            ACTIVITYSTACK->pop ();
            break;
        }
        case IDC_SETUP_WIFI: {
            SettingCheckNetworkPromptActivity* act;
            act = (SettingCheckNetworkPromptActivity*)GetWindowAdditionalData (self->hwnd);

            Intent* my_intent = Intent::newIntent ();
            if (act->m_firstBoot)
                my_intent->putExtra ("firstBoot", std::string ("yes"));
            else
                my_intent->putExtra ("firstBoot", std::string ("no"));
            ACTIVITYSTACK->push ("SettingWifiListActivity", my_intent);
            break;
        }
        default:
            break;
        }
    }

    return TRUE;
}

static BOOL mymain_onClose (mWidget* _this, int message)
{
    _MG_PRINTF ("SettingCheckNetworkPromptActivity: mymain_onClose called.\n");
    return TRUE;
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
        id: IDC_TEXT_NO4G,
        x: ACTIVITY_CONTENT_MARGIN_H,
        y: 0,
        w: ACTIVITY_W - ACTIVITY_CONTENT_MARGIN_H - ACTIVITY_CONTENT_MARGIN_H,
        h: ACTIVITY_CONTENT_ITEM_H + (ACTIVITY_CONTENT_ITEM_H >> 1),
        style: WS_VISIBLE,
        ex_style: WS_EX_NONE,
        caption: "",
    },
    {
        class_name: NCSCTRL_BUTTON,
        id: IDC_RETRY_4G,
        x: 50,
        y: ACTIVITY_CONTENT_ITEM_H + (ACTIVITY_CONTENT_ITEM_H >> 1),
        w: ACTIVITY_W - 50 - 50,
        h: ACTIVITY_CONTENT_ITEM_H,
        style: WS_VISIBLE | NCSS_NOTIFY,
        ex_style: WS_EX_NONE,
        caption: "",
    },
    {
        class_name: NCSCTRL_STATIC,
        id: IDC_TEXT_WIFI,
        x: ACTIVITY_CONTENT_MARGIN_H,
        y: ACTIVITY_CONTENT_ITEM_H + ACTIVITY_CONTENT_ITEM_H + (ACTIVITY_CONTENT_ITEM_H >> 1),
        w: ACTIVITY_W - ACTIVITY_CONTENT_MARGIN_H - ACTIVITY_CONTENT_MARGIN_H,
        h: ACTIVITY_CONTENT_ITEM_H + (ACTIVITY_CONTENT_ITEM_H >> 1),
        style: WS_VISIBLE,
        ex_style: WS_EX_NONE,
        caption: "",
    },
    {
        class_name: NCSCTRL_BUTTON,
        id: IDC_SETUP_WIFI,
        x: 50,
        y: ACTIVITY_CONTENT_ITEM_H + ACTIVITY_CONTENT_ITEM_H + ACTIVITY_CONTENT_ITEM_H + ACTIVITY_CONTENT_ITEM_H,
        w: ACTIVITY_W - 50 - 50,
        h: ACTIVITY_CONTENT_ITEM_H,
        style: WS_VISIBLE | NCSS_NOTIFY,
        ex_style: WS_EX_NONE,
        caption: "",
    },
};

static NCS_MNWND_TEMPLATE mymain_templ = {
    NCSCTRL_DIALOGBOX, 
    1,
    ACTIVITY_X, ACTIVITY_Y, ACTIVITY_W, ACTIVITY_H,
    WS_NONE, WS_EX_NONE,
    N_("Checking Network Prompt"),
    NULL,
    NULL,
    mymain_handlers,
    _ctrl_templ,
    TABLESIZE(_ctrl_templ),
    0, 0, NULL, 0, 0,
    &create_notify_info,
    0
};

SettingCheckNetworkPromptActivity::SettingCheckNetworkPromptActivity() :
    NCSActivity (&mymain_templ)
{ 
    m_style = STYLE_PUSH;
    m_firstBoot = false;
}

