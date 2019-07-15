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
 * @file SettingTimeZoneActivity.cc
 * @Synopsis The TimeZone activity of Setting app.
 * @author Vincent Wei
 * @version 1.0
 * @date 09/11/2018
 */

#include "global.h"
#include "utilities.h"
#include "misc.h"

#include "resource.h"

#include "Activity.hh"
#include "ActivityStack.hh"
#include "NCSActivity.hh"

#include "RuntimeConfService.hh"
#include "NetworkService.hh"
#include "SysConfService.hh"
#include "SettingTimeZoneActivity.hh"
#include "MenuListPiece.hh"

REGISTER_ACTIVITY(SettingTimeZoneActivity);

static MENU_ENTRY _menu_entries [] = {
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT +8 Beijing"),
            SWITCH_OFF, NULL, NULL, (DWORD)"Asia/Shanghai" },           // Default/Current TZ
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT -12 IDL West"),
            SWITCH_OFF, NULL, NULL, (DWORD)"IDLW12" },                  // IDLW12
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT -11 Samoa"),
            SWITCH_OFF, NULL, NULL, (DWORD)"Pacific/Apia" },            // WST11
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT -11 Mid. Isl."),
            SWITCH_OFF, NULL, NULL, (DWORD)"Pacific/Midway" },          // SST11
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT -10 Honolulu"),
            SWITCH_OFF, NULL, NULL, (DWORD)"Pacific/Honolulu" },        // HST10
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT -9:30 Ma. Isl."),
            SWITCH_OFF, NULL, NULL, (DWORD)"Pacific/Marquesas" },       // MART9:30
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT -9 Alaska"),
            SWITCH_OFF, NULL, NULL, (DWORD)"America/Anchorage#Alaska" },// AKST9AKDT,M3.2.0,M11.1.0
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT -9 Anchorage"),
            SWITCH_OFF, NULL, NULL, (DWORD)"America/Anchorage" },       // AKST9AKDT,M3.2.0,M11.1.0
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT -8 San Francisco"),
            SWITCH_OFF, NULL, NULL, (DWORD)"America/Los_Angeles" },     // PST8PDT,M3.2.0,M11.1.0
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT -8 Seattle"),
            SWITCH_OFF, NULL, NULL, (DWORD)"America/Los_Angeles#Seattle" }, // PST8PDT,M3.2.0,M11.1.0
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT -8 Vancouver"),
            SWITCH_OFF, NULL, NULL, (DWORD)"America/Vancouver" },       // PST8PDT,M3.2.0,M11.1.0
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT -7 Phoenix"),
            SWITCH_OFF, NULL, NULL, (DWORD)"America/Phoenix" },         // MST7
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT -6 Chicago"),
            SWITCH_OFF, NULL, NULL, (DWORD)"America/Chicago" },         // CST6CDT,M3.2.0,M11.1.0
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT -6 Mexico City"),
            SWITCH_OFF, NULL, NULL, (DWORD)"America/Mexico_City" },     // CST6CDT,M4.1.0,M10.5.0
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT -5 New York"),
            SWITCH_OFF, NULL, NULL, (DWORD)"America/New_York" },        // EST5EDT,M3.2.0,M11.1.0
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT -5 Toronto"),
            SWITCH_OFF, NULL, NULL, (DWORD)"America/Toronto" },         // EST5EDT,M3.2.0,M11.1.0
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT -5 Washington D.C."),
            SWITCH_OFF, NULL, NULL, (DWORD)"America/New_York#Washington" }, // EST5EDT,M3.2.0,M11.1.0
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT -4:30 Caracas"),
            SWITCH_OFF, NULL, NULL, (DWORD)"America/Caracas" },         // VET4:30
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT -4 La Paz"),
            SWITCH_OFF, NULL, NULL, (DWORD)"America/La_Paz" },          // BOT4
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT -4 Santiago"),
            SWITCH_OFF, NULL, NULL, (DWORD)"America/La_Paz#Santiago" }, // BOT4
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT -3:30 Saint John's"),
            SWITCH_OFF, NULL, NULL, (DWORD)"America/St_Johns" },        // NST3:30NDT,M3.2.0/0:01,M11.1.0/0:01
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT -3 Brasilia"),
            SWITCH_OFF, NULL, NULL, (DWORD)"America/Sao_Paulo" },       // BRT3BRST,M10.3.0/0,M2.3.0/0
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT -3 Bs. As. City"),
            SWITCH_OFF, NULL, NULL, (DWORD)"America/Argentina/Buenos_Aires" },    // ART3
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT -2 Mid-Atlantic"),
            SWITCH_OFF, NULL, NULL, (DWORD)"Atlantic/South_Georgia" },  // GST2
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT -1 Azores"),
            SWITCH_OFF, NULL, NULL, (DWORD)"Atlantic/Azores" },         // AZOT1AZOST,M3.5.0/0,M10.5.0/1
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT -1 Cape Verde"),
            SWITCH_OFF, NULL, NULL, (DWORD)"Atlantic/Cape_Verde" },     // CVT1
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT London"),
            SWITCH_OFF, NULL, NULL, (DWORD)"Europe/London" },           // GMT0BST,M3.5.0/1,M10.5.0
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT Casablanca"),
            SWITCH_OFF, NULL, NULL, (DWORD)"Africa/Casablanca" },       // WET0
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT +1 Berlin"),
            SWITCH_OFF, NULL, NULL, (DWORD)"Europe/Berlin" },           // CET-1CEST,M3.5.0,M10.5.0/3
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT +1 Lagos"),
            SWITCH_OFF, NULL, NULL, (DWORD)"Africa/Lagos" },            // WAT-1
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT +1 Paris"),
            SWITCH_OFF, NULL, NULL, (DWORD)"Europe/Paris" },            // CET-1CEST,M3.5.0,M10.5.0/3
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT +1 Rome"),
            SWITCH_OFF, NULL, NULL, (DWORD)"Europe/Rome" },             // CET-1CEST,M3.5.0,M10.5.0/3
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT +1 Warsaw"),
            SWITCH_OFF, NULL, NULL, (DWORD)"Europe/Warsaw" },           // CET-1CEST,M3.5.0,M10.5.0/3
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT +2 Athens"),
            SWITCH_OFF, NULL, NULL, (DWORD)"Europe/Athens" },           // EET-2EEST,M3.5.0/3,M10.5.0/4
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT +2 Cairo"),
            SWITCH_OFF, NULL, NULL, (DWORD)"Africa/Tripol" },           // EET-2
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT +2 Helsinki"),
            SWITCH_OFF, NULL, NULL, (DWORD)"Europe/Helsinki" },         // EET-2EEST,M3.5.0/3,M10.5.0/4
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT +2 Istanbul"),
            SWITCH_OFF, NULL, NULL, (DWORD)"Europe/Istanbul " },        // EET-2EEST,M3.5.0/3,M10.5.0/4
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT +2 Jerusalem"),
            SWITCH_OFF, NULL, NULL, (DWORD)"Asia/Gaza" },               // EET-2EEST,M3.5.6/0:01,M9.1.5
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT +2 Pretoria"),
            SWITCH_OFF, NULL, NULL, (DWORD)"Africa/Johannesburg" },     // SAST-2; SAST: South Africa Standard Time
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT +3 Baghdad"),
            SWITCH_OFF, NULL, NULL, (DWORD)"Asia/Baghdad" },            // AST-3
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT +3 Kuwait"),
            SWITCH_OFF, NULL, NULL, (DWORD)"Asia/Kuwait" },             // AST-3
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT +3 Moscow"),
            SWITCH_OFF, NULL, NULL, (DWORD)"Europe/Moscow" },           // MSK-4
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT +3 Nairobi"),
            SWITCH_OFF, NULL, NULL, (DWORD)"Africa/Nairobi" },          // EAT-3
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT +3:30 Tehran"),
            SWITCH_OFF, NULL, NULL, (DWORD)"Asia/Tehran" },             // IRST-3:30IRDT,80/0,264/0
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT +4 Dubai"),
            SWITCH_OFF, NULL, NULL, (DWORD)"Asia/Dubai" },              // GST-4
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT +4 Muscat"),
            SWITCH_OFF, NULL, NULL, (DWORD)"Asia/Muscat" },             // GST-4
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT +4:30 Kabul"),
            SWITCH_OFF, NULL, NULL, (DWORD)"Asia/Kabul" },              // AFT-4:30
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT +4:30 Tashkent"),
            SWITCH_OFF, NULL, NULL, (DWORD)"Asia/Tashkent" },           // UZT-5
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT +5 Ekaterinburg"),
            SWITCH_OFF, NULL, NULL, (DWORD)"Asia/Yekaterinburg" },      // YEKT-5YEKST,M3.5.0,M10.5.0/3
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT +5 Islamabad"),
            SWITCH_OFF, NULL, NULL, (DWORD)"Asia/Karachi" },            // PKT-5; PKT: Pakistan Standard Time
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT +5:30 Kolkata"),
            SWITCH_OFF, NULL, NULL, (DWORD)"Asia/Kolkata" },            // IST-5:30; IST: India Standard Time
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT +5:30 Mumbai"),
            SWITCH_OFF, NULL, NULL, (DWORD)"Asia/Kolkata" },            // IST-5:30; IST: India Standard Time
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT +6 Almaty"),
            SWITCH_OFF, NULL, NULL, (DWORD)"Asia/Almaty" },             // ALMT-6
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT +6 Dhaka"),
            SWITCH_OFF, NULL, NULL, (DWORD)"Asia/Dhaka" },              // BDT-6
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT +6 Novosibirsk"),
            SWITCH_OFF, NULL, NULL, (DWORD)"Asia/Novosibirsk" },        // NOVT-6NOVST,M3.5.0,M10.5.0/3
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT +6:30 Yangon"),
            SWITCH_OFF, NULL, NULL, (DWORD)"Asia/Rangoon" },            // MMT-6:30
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT +7 Bangkok"),
            SWITCH_OFF, NULL, NULL, (DWORD)"Asia/Bangkok" },            // ICT-7
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT +7 Hanoi"),
            SWITCH_OFF, NULL, NULL, (DWORD)"Asia/Ho_Chi_Minh" },        // ICT-7
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT +7 Jakarta"),
            SWITCH_OFF, NULL, NULL, (DWORD)"Asia/Jakarta" },            // WIT-7
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT +8 Beijing"),
            SWITCH_OFF, NULL, NULL, (DWORD)"Asia/Shanghai" },           // CST-8; CST: China Standard Time
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT +8 Hong Kong"),
            SWITCH_OFF, NULL, NULL, (DWORD)"Asia/Hong_Kong" },          // HKT-8
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT +9 Seoul"),
            SWITCH_OFF, NULL, NULL, (DWORD)"Asia/Seoul" },              // KST-9
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT +9 Tokyo"),
            SWITCH_OFF, NULL, NULL, (DWORD)"Asia/Tokyo" },              // JST-9
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT +9:30 Adelaide"),
            SWITCH_OFF, NULL, NULL, (DWORD)"Australia/Adelaide" },      // CST-9:30CST,M10.1.0,M4.1.0/3
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT +9:30 Darwin"),
            SWITCH_OFF, NULL, NULL, (DWORD)"Australia/Darwin" },        // CST-9:30
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT +10 Brisbane"),
            SWITCH_OFF, NULL, NULL, (DWORD)"Australia/Brisbane" },      // EST-10
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT +10 Canberra"),
            SWITCH_OFF, NULL, NULL, (DWORD)"Australia/Sydney " },       // EST-10EST,M10.1.0,M4.1.0/3
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT +10:30 LDH"),
            SWITCH_OFF, NULL, NULL, (DWORD)"Australia/Lord_Howe" },     // LHST-10:30LHST-11,M10.1.0,M4.1.0
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT +11:30 Norfolk Is."),
            SWITCH_OFF, NULL, NULL, (DWORD)"Pacific/Norfolk" },         // NFT-11:30
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT +12 Anadyr"),
            SWITCH_OFF, NULL, NULL, (DWORD)"Asia/Anadyr" },             // ANAT-11ANAST,M3.5.0,M10.5.0/3
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT +12 Auckland"),
            SWITCH_OFF, NULL, NULL, (DWORD)"Pacific/Auckland" },        // NZST-12NZDT,M9.5.0,M4.1.0/3
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT +12 Fiji"),
            SWITCH_OFF, NULL, NULL, (DWORD)"Pacific/Fiji" },            // FJT-12
    { 0, TYPE_MENU_ENTRY_CHECKABLE, NULL, N_("GMT +12 Magadan"),
            SWITCH_OFF, NULL, NULL, (DWORD)"Asia/Magadan" },            // MAGT-11MAGST,M3.5.0,M10.5.0/3
};

const char* GetTimeZoneName (const char* tz)
{
    for (int i = 0; i < (int)TABLESIZE (_menu_entries); i++) {
        if (strcmp (tz, (const char*)_menu_entries [i].add_data) == 0) {
            return _(_menu_entries [i].name);
        }
    }

    return NULL;
}

static LRESULT mymain_onIntent (mMainWnd* self, UINT msg, DWORD wparam, DWORD lparam)
{
    Intent* intent = (Intent*)wparam;
    SettingTimeZoneActivity* act;
    act = (SettingTimeZoneActivity*)GetWindowAdditionalData (self->hwnd);

    if (intent && strcasecmp (intent->getString ("firstBoot").c_str(), "yes") == 0) {
        act->m_firstBoot = true;
        act->setAutoEscape (false);
    }

    Intent::deleteIntent (intent);
    return 0;
}

static BOOL mymain_onCommand (mMainWnd* self, int id, int nc, HWND hCtrl)
{
    if (id == IDC_CONTENT && nc >= 0 && nc < (int)TABLESIZE (_menu_entries)) {

        const char* tz = (const char*)_menu_entries [nc].add_data;

        RTCFGSERVICE->setTimeZone (tz);
        RTCFGSERVICE->save ();
        SYSCFGSERVICE->setSysTimeZone (tz);

        SettingTimeZoneActivity* act;
        act = (SettingTimeZoneActivity*)GetWindowAdditionalData (self->hwnd);
        if (act->m_firstBoot) {
            Intent* my_intent = Intent::newIntent ();
            my_intent->putExtra ("firstBoot", std::string ("yes"));
            ACTIVITYSTACK->push ("SettingCheckNetworkActivity", my_intent);
        }
        else {
            ACTIVITYSTACK->pop ();
        }
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

        int idx = 0;
        const char* tz = RTCFGSERVICE->getTimeZone ();
        for (int i = 1; i < (int)TABLESIZE (_menu_entries); i++) {
            _menu_entries [i].id = IDE_MIN + i;
            _menu_entries [i].name = _(_menu_entries [i].name);

            if (strcmp (tz, (const char*)_menu_entries [i].add_data) == 0) {
                _menu_entries [i].value = SWITCH_ON;
                idx = i;
            }
            else {
                _menu_entries [i].value = SWITCH_OFF;
            }
        }

        if (idx)
            memcpy (_menu_entries, _menu_entries + idx, sizeof (MENU_ENTRY));
        else
            _ERR_PRINTF ("Invalid current timezone: %s\n", tz);

        createMenuListView ((mContainerCtrl*)ctrl_ctnr, &rc, _menu_entries, TABLESIZE (_menu_entries));
    }

    return TRUE;
}

static BOOL mymain_onClose (mMainWnd* self, int message)
{
    _MG_PRINTF ("SettingTimeZoneActivity: mymain_onClose called.\n");

    return TRUE;
}

static BOOL mymain_onCreated (struct _NCS_CREATE_NOTIFY_INFO* info, mComponent* self, DWORD special_id)
{
    mWidget* ctrl_title = (mWidget*)(_c(self)->getChild (self, IDC_TITLE));
    mHotPiece* piece_title = (mHotPiece*)(ctrl_title->body);
    _c(piece_title)->setProperty (piece_title, NCSP_LABELPIECE_LABEL, (DWORD)_("Time Zone"));
    ncsSetElement (ctrl_title, NCS_FGC_WINDOW, COLOR_TITLE);
    ncsSetFont (ctrl_title->hwnd, FONT_TITLE);
    return TRUE;
}

static NCS_CREATE_NOTIFY_INFO create_notify_info = {
    onCreated: mymain_onCreated,
};

static NCS_EVENT_HANDLER mymain_handlers [] = {
    {MSG_CREATE, reinterpret_cast<void*>(mymain_onCreate)},
    {MSG_USER_APP_DATA, reinterpret_cast<void*>(mymain_onIntent)},
    {MSG_COMMAND, reinterpret_cast<void*>(mymain_onCommand)},
    {MSG_USER, reinterpret_cast<void*>(mymain_onSwitchStateChanged)},
    {MSG_USER_APP_READY, reinterpret_cast<void*>(mymain_onReady)},
    {MSG_CLOSE, reinterpret_cast<void*>(mymain_onClose)},
    {MSG_LBUTTONDOWN, reinterpret_cast<void*>(SpeedMeterMessageHandler)},
    {MSG_LBUTTONUP, reinterpret_cast<void*>(SpeedMeterMessageHandler)},
    {MSG_MOUSEMOVE, reinterpret_cast<void*>(SpeedMeterMessageHandler)},
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
    N_("Screen Lock Time"),
    NULL,
    NULL,
    mymain_handlers,
    _ctrl_templ,
    TABLESIZE(_ctrl_templ),
    0, 0, NULL, 0, 0,
    &create_notify_info,
    0
};

SettingTimeZoneActivity::SettingTimeZoneActivity()
    : NCSActivity (&mymain_templ)
{
    m_style = STYLE_PUSH;
    m_firstBoot = false;
}

SettingTimeZoneActivity::~SettingTimeZoneActivity()
{
}

