/*!============================================================================
 * @file SettingLanguageActivity.cc 
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

#include "RuntimeConfService.hh"
#include "SettingLanguageActivity.hh"
#include "LanguageListPiece.hh"

REGISTER_ACTIVITY(SettingLanguageActivity);

static LRESULT mymain_onIntent (mMainWnd* self, UINT msg, DWORD wparam, DWORD lparam)
{
    Intent* intent = (Intent*)wparam;
    SettingLanguageActivity* act;
    act = (SettingLanguageActivity*)GetWindowAdditionalData (self->hwnd);

    if (intent && strcasecmp (intent->getString ("firstBoot").c_str(), "yes") == 0) {
        act->m_firstBoot = true;
        act->setAutoEscape (false);
    }

    Intent::deleteIntent (intent);
    return 0;
}

static BOOL mymain_onCommand (mWidget* self, int id, int nc, HWND hCtrl)
{
    if (id == IDC_CONTENT) {
        LanguageSettings* lang_settings = RTCFGSERVICE->m_lang_settings;
        LangMap::iterator it = lang_settings->langs.begin ();
        for (int i = 0; i < nc; i++) {
            it ++;
            if (it == lang_settings->langs.end ()) {
                return TRUE;
            }
        }

        if (strcmp (it->first.c_str(), lang_settings->current)) {
            RTCFGSERVICE->setCurrentLanguage (it->first.c_str());
            // NOTE: do not save in development stage
            RTCFGSERVICE->save ();
        }

        SettingLanguageActivity* act;
        act = (SettingLanguageActivity*)GetWindowAdditionalData (self->hwnd);
        if (act->m_firstBoot) {
            Intent* my_intent = Intent::newIntent ();
            my_intent->putExtra ("firstBoot", std::string ("yes"));
            ACTIVITYSTACK->push ("SettingTimeZoneActivity", my_intent);
        }
        else {
            ACTIVITYSTACK->pop ();
        }
    }

    return TRUE;
}

static BOOL mymain_onClose (mWidget* _this, int message)
{
    _MG_PRINTF ("SettingLanguageActivity: mymain_onClose called.\n");
    return TRUE;
}

static void createLangListView (mContainerCtrl* ctrl_ctnr, LanguageSettings* lang_settings)
{
    RECT rc = {0, 0, ACTIVITY_CONTENT_W, ACTIVITY_CONTENT_H};
    mLanguageListPiece* langListPiece;
    mPanelPiece* panel;

    langListPiece = (mLanguageListPiece *)NEWPIECEEX (mLanguageListPiece, lang_settings);
    langListPiece->ctrl_ctnr = ctrl_ctnr;
    _c(langListPiece)->setRect (langListPiece, &rc);
    _c(langListPiece)->reloadData (langListPiece);

    panel = NEWPIECE (mPanelPiece);
    _c(panel)->addContent (panel, (mHotPiece*)langListPiece, 0, 0);
    _c(panel)->setRect (panel, &rc);

    _c(ctrl_ctnr)->setBody (ctrl_ctnr, (mHotPiece*)panel);

    /* set scrollbar color here */
    mHotPiece *scrollbar;
    scrollbar = langListPiece->m_scrollbar->piece;
    _c(scrollbar)->setProperty(scrollbar, NCSP_TRANROUND_BKCOLOR, COLOR_SCROLLBAR);
}

static NCS_EVENT_HANDLER mymain_handlers [] = {
    {MSG_USER_APP_DATA, reinterpret_cast<void*>(mymain_onIntent)},
    {MSG_COMMAND, reinterpret_cast<void*>(mymain_onCommand)},
    {MSG_CLOSE, reinterpret_cast<void*>(mymain_onClose)},
    {MSG_LBUTTONDOWN, reinterpret_cast<void*>(SpeedMeterMessageHandler)},
    {MSG_LBUTTONUP, reinterpret_cast<void*>(SpeedMeterMessageHandler)},
    {MSG_MOUSEMOVE, reinterpret_cast<void*>(SpeedMeterMessageHandler)},
    {0, NULL}
};

static BOOL mymain_onCreated (struct _NCS_CREATE_NOTIFY_INFO* info, mComponent* self, DWORD special_id)
{
	mWidget* ctrl_title = (mWidget*)(_c(self)->getChild (self, IDC_TITLE));
    ncsSetElement (ctrl_title, NCS_FGC_WINDOW, COLOR_TITLE);
    ncsSetFont (ctrl_title->hwnd, FONT_TITLE);

	mContainerCtrl* ctrl_ctnr = (mContainerCtrl*)(_c(self)->getChild (self, IDC_CONTENT));
    createLangListView (ctrl_ctnr, RTCFGSERVICE->m_lang_settings);
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
        caption: "Select Language",
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
        class_name: NCSCTRL_CONTAINERCTRL,
        id: IDC_CONTENT,
        x: ACTIVITY_CONTENT_X,
        y: ACTIVITY_CONTENT_Y,
        w: ACTIVITY_CONTENT_W,
        h: ACTIVITY_CONTENT_H,
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
    N_("Language Setting"),
    NULL,
    NULL,
    mymain_handlers,
    _ctrl_templ,
    TABLESIZE(_ctrl_templ),
    0, 0, NULL, 0, 0,
    &create_notify_info,
    0
};

SettingLanguageActivity::SettingLanguageActivity() :
    NCSActivity (&mymain_templ)
{ 
    MGNCS_INIT_CLASS (mLanguageListPiece);

    m_style = STYLE_PUSH;
    m_firstBoot = false;
}

SettingLanguageActivity::~SettingLanguageActivity()
{
}

