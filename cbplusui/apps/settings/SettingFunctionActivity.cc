/*!============================================================================
 * @file SettingFunctionActivity.cc 
 * @Synopsis The Function Setting activity of Setting app. 
 * @author Vincent Wei
 * @version 1.0
 * @date 12/06/2018
 *
 *  Copyright (C) 2018 Beijing Joobot Technogolies Inc.
 */

#include "global.h"
#include "utilities.h"
#include "misc.h"

#include "resource.h"

#include "Activity.hh"
#include "ActivityStack.hh"
#include "NCSActivity.hh"

#include "SettingFunctionActivity.hh"
#include "MenuListPiece.hh"

REGISTER_ACTIVITY(SettingFunctionActivity);

#define IDE_ENABLE_GPS      (IDE_MIN + 0)
#define IDE_SAVE_LOCATION   (IDE_MIN + 1)
#define IDE_PUSH_RAW        (IDE_MIN + 3)

static MENU_ENTRY _menu_entries [] = {
    { (IDE_MIN + 0), TYPE_MENU_ENTRY_SWITCH, NULL, N_("GPS")},
    { (IDE_MIN + 1), TYPE_MENU_ENTRY_SWITCH, NULL, N_("Write Location to Photos")},
    { (IDE_MIN + 2), TYPE_MENU_ENTRY_DESC, NULL, N_("Write GPS location to the photo files (only JPEG) before uploading them to the cloud.") },
    { (IDE_MIN + 3), TYPE_MENU_ENTRY_SWITCH, NULL, N_("Push Raw Files")},
    { (IDE_MIN + 4), TYPE_MENU_ENTRY_DESC, NULL, N_("Push the raw files to your app when working in shot-and-push mode.") },
};


static BOOL mymain_onCommand (mMainWnd* self, int id, int nc, HWND hCtrl)
{
    if (id == IDC_CONTENT && nc < (int)TABLESIZE (_menu_entries)
            && _menu_entries[nc].type == TYPE_MENU_ENTRY_ACT
            && _menu_entries[nc].activity != NULL) {
        Intent* my_intent = Intent::newIntent ();
        my_intent->putExtra ("firstBoot", std::string ("no"));
        ACTIVITYSTACK->push (_menu_entries[nc].activity, my_intent);
    }

    return TRUE;
}

static LRESULT mymain_onSwitchStateChanged (mMainWnd* self, UINT msg, DWORD wparam, DWORD lparam)
{
    _MG_PRINTF ("MSG_USER: item #%d switch %d\n", wparam, lparam);
    return 0;
}

static LRESULT mymain_onReady (mMainWnd* self, UINT msg, DWORD wparam, DWORD lparam)
{
    if (wparam == ACTIVITY_PUSHED) {
    }
    else if (wparam == ACTIVITY_POPPED) {
    }

    return 0;
}

static BOOL mymain_onCreate (mMainWnd* self, DWORD dwAddData)
{
    NCS_WND_TEMPLATE ctrl_templ = {
        class_name: NCSCTRL_CONTAINERCTRL,
        id: IDC_CONTENT,
        x: ACTIVITY_CONTENT_X,
        y: ACTIVITY_CONTENT_Y,
        w: ACTIVITY_CONTENT_W,
        h: ACTIVITY_CONTENT_H,
        style: WS_VISIBLE | NCSS_NOTIFY,
        ex_style: WS_EX_NONE,
        caption: "",
    };

    mWidget* ctrl_ctnr = ncsCreateWindowIndirect (&ctrl_templ, self->hwnd);
    if (ctrl_ctnr) {
        RECT rc = {0, 0, ACTIVITY_CONTENT_W, ACTIVITY_CONTENT_H};

        for (size_t i = 0; i < TABLESIZE (_menu_entries); i++) {
            _menu_entries [i].name = _(_menu_entries [i].name);
        }

        /* TODO: initialize the value according to the real settings */
        _menu_entries [IDE_ENABLE_GPS - IDE_MIN].value = SWITCH_ON;
        _menu_entries [IDE_SAVE_LOCATION - IDE_MIN].value = SWITCH_ON;
        _menu_entries [IDE_PUSH_RAW - IDE_MIN].value = SWITCH_ON;

        createMenuListView ((mContainerCtrl*)ctrl_ctnr, &rc, _menu_entries, TABLESIZE (_menu_entries));
    }

    return TRUE;
}

static BOOL mymain_onClose (mMainWnd* self, int message)
{
    _MG_PRINTF ("SettingFunctionActivity: mymain_onClose called.\n");

    return TRUE;
}

static BOOL mymain_onCreated (struct _NCS_CREATE_NOTIFY_INFO* info, mComponent* self, DWORD special_id)
{
    mWidget* ctrl_title = (mWidget*)(_c(self)->getChild (self, IDC_TITLE));
    mHotPiece* piece_title = (mHotPiece*)(ctrl_title->body);
    _c(piece_title)->setProperty (piece_title, NCSP_LABELPIECE_LABEL, (DWORD)_("Function Settings"));
    ncsSetElement (ctrl_title, NCS_FGC_WINDOW, COLOR_TITLE);
    ncsSetFont (ctrl_title->hwnd, FONT_TITLE);
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
    {0, NULL}
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
        style: WS_VISIBLE | NCSS_NOTIFY,
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
};

static NCS_MNWND_TEMPLATE mymain_templ = {
    NCSCTRL_DIALOGBOX, 
    1,
    ACTIVITY_X, ACTIVITY_Y, ACTIVITY_W, ACTIVITY_H,
    WS_NONE, WS_EX_NONE,
    N_("Function Settings"),
    NULL,
    NULL,
    mymain_handlers,
    _ctrl_templ,
    TABLESIZE(_ctrl_templ),
    0, 0, NULL, 0, 0,
    &create_notify_info,
    0
};

SettingFunctionActivity::SettingFunctionActivity() : NCSActivity (&mymain_templ)
{ 
    m_style = STYLE_PUSH;
}

SettingFunctionActivity::~SettingFunctionActivity()
{
}

