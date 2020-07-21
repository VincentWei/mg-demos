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
 * @file SignUpActivity.cc
 * @Synopsis A sample activity for sign up.
 * @author Vincent Wei
 * @version 1.0
 * @date 30/05/2019
 */

#include "global.h"
#include "utilities.h"

#include "resource.h"

#include "Activity.hh"
#include "ActivityStack.hh"
#include "NCSActivity.hh"

#include "SignUpActivity.hh"

REGISTER_ACTIVITY(SignUpActivity);

static BOOL mymain_onCreate(mWidget *_this, DWORD add_data)
{
    return TRUE;
}

static void mymain_onClose(mWidget *_this, int message)
{
}

static NCS_EVENT_HANDLER mymain_handlers[] = {
    {MSG_CREATE, reinterpret_cast<void *>(mymain_onCreate)},
    {MSG_CLOSE, reinterpret_cast<void *>(mymain_onClose)},
    {MSG_LBUTTONDOWN, reinterpret_cast<void *>(SpeedMeterMessageHandler)},
    {MSG_LBUTTONUP, reinterpret_cast<void *>(SpeedMeterMessageHandler)},
    {MSG_MOUSEMOVE, reinterpret_cast<void *>(SpeedMeterMessageHandler)},
    {0, NULL}};

#define ID_NAME 104
#define ID_COUN 105
#define ID_CITY 106
#define ID_PSWD 107
#define ID_SPIN 108
#define ID_INFO 109

#define ID_SIGNUP 155
#define ID_CANCEL 156

static void btn_onClicked(mWidget *_this, int id, int nc, HWND hCtrl)
{
    if (nc == NCSN_WIDGET_CLICKED)
    {
        if (id == ID_CANCEL)
        {
            ACTIVITYSTACK->pop();
        }
        else if (id == ID_SIGNUP)
        {
            //TODO
        }
    }
}

static NCS_EVENT_HANDLER btn_handlers[] =
    {
        {NCS_NOTIFY_CODE(NCSN_WIDGET_CLICKED), reinterpret_cast<void *>(btn_onClicked)},
        {0, NULL}};

static NCS_RDR_INFO btn_rdr_info[] =
    {
        {"flat", "flat", NULL}};

static NCS_PROP_ENTRY static_props[] =
    {
        {NCSP_STATIC_ALIGN, NCS_ALIGN_RIGHT},
        {0, 0}};

static NCS_PROP_ENTRY spin_props[] =
    {
        {NCSP_SPNBOX_MAXPOS, 99},
        {NCSP_SPNBOX_MINPOS, 0},
        {NCSP_SPNBOX_CURPOS, 25},
        {NCSP_SPNBOX_LINESTEP, 1},
        {0, 0}};

#define MARGIN_LEFT 10
#define MARGIN_TOP 10

#define FIELD_HEIGHT 25
#define FIELD_WIDTH 200
#define LABEL_WIDTH 100
#define EDITOR_WIDTH 200
#define EDITOR_HEIGHT 160

#define SPACING 10

//START_OF_TEMPLATE
static NCS_WND_TEMPLATE _ctrl_templ[] =
    {
        {NCSCTRL_STATIC,
         0,
         MARGIN_LEFT, MARGIN_TOP, LABEL_WIDTH, FIELD_HEIGHT,
         WS_VISIBLE,
         WS_EX_NONE,
         "Name:",
         static_props,
         NULL,
         NULL, NULL, 0, 0},
        {NCSCTRL_SLEDIT,
         ID_NAME,
         MARGIN_LEFT + LABEL_WIDTH + SPACING, MARGIN_TOP, FIELD_WIDTH, FIELD_HEIGHT,
         WS_BORDER | WS_VISIBLE | NCSS_EDIT_LEFT,
         WS_EX_NONE,
         "",
         NULL,
         NULL,
         NULL, NULL, 0, 0},
        {NCSCTRL_STATIC,
         0,
         MARGIN_LEFT, MARGIN_TOP + FIELD_HEIGHT + SPACING, LABEL_WIDTH, FIELD_HEIGHT,
         WS_VISIBLE,
         WS_EX_NONE,
         "Age:",
         static_props,
         NULL,
         NULL, NULL, 0, 0},
        {NCSCTRL_SPINBOX,
         ID_SPIN,
         MARGIN_LEFT + LABEL_WIDTH + SPACING, MARGIN_TOP + FIELD_HEIGHT + SPACING, FIELD_WIDTH, FIELD_HEIGHT,
         WS_VISIBLE | NCSS_SPNBOX_NUMBER | NCSS_SPNBOX_AUTOLOOP,
         WS_EX_NONE,
         "",
         spin_props,
         NULL,
         NULL, NULL, 0, 0},

        {NCSCTRL_STATIC,
         0,
         MARGIN_LEFT, MARGIN_TOP + (FIELD_HEIGHT + SPACING) * 2, LABEL_WIDTH, FIELD_HEIGHT,
         WS_VISIBLE,
         WS_EX_NONE,
         "Country:",
         static_props,
         NULL,
         NULL, NULL, 0, 0},
        {NCSCTRL_SLEDIT,
         ID_COUN,
         MARGIN_LEFT + LABEL_WIDTH + SPACING, MARGIN_TOP + (FIELD_HEIGHT + SPACING) * 2, FIELD_WIDTH, FIELD_HEIGHT,
         WS_BORDER | WS_VISIBLE | NCSS_EDIT_CENTER | NCSS_EDIT_UPPERCASE,
         WS_EX_NONE,
         "",
         NULL,
         NULL,
         NULL, NULL, 0, 0},
        {NCSCTRL_STATIC,
         0,
         MARGIN_LEFT, MARGIN_TOP + (FIELD_HEIGHT + SPACING) * 3, LABEL_WIDTH, FIELD_HEIGHT,
         WS_VISIBLE,
         WS_EX_NONE,
         "City :",
         static_props,
         NULL,
         NULL, NULL, 0, 0},
        {NCSCTRL_SLEDIT,
         ID_CITY,
         MARGIN_LEFT + LABEL_WIDTH + SPACING, MARGIN_TOP + (FIELD_HEIGHT + SPACING) * 3, FIELD_WIDTH, FIELD_HEIGHT,
         WS_BORDER | WS_VISIBLE | NCSS_EDIT_LOWERCASE,
         WS_EX_NONE,
         "",
         NULL,
         NULL,
         NULL, NULL, 0, 0},
        {NCSCTRL_STATIC,
         0,
         MARGIN_LEFT, MARGIN_TOP + (FIELD_HEIGHT + SPACING) * 4, LABEL_WIDTH, FIELD_HEIGHT,
         WS_VISIBLE,
         WS_EX_NONE,
         "Password:",
         static_props,
         NULL,
         NULL, NULL, 0, 0},
        {NCSCTRL_SLEDIT,
         ID_PSWD,
         MARGIN_LEFT + LABEL_WIDTH + SPACING, MARGIN_TOP + (FIELD_HEIGHT + SPACING) * 4, FIELD_WIDTH, FIELD_HEIGHT,
         WS_BORDER | WS_VISIBLE | NCSS_SLEDIT_PASSWORD,
         WS_EX_NONE,
         "",
         NULL,
         NULL,
         NULL, NULL, 0, 0},
        {NCSCTRL_STATIC,
         0,
         MARGIN_LEFT, MARGIN_TOP + (FIELD_HEIGHT + SPACING) * 5, LABEL_WIDTH, FIELD_HEIGHT,
         WS_VISIBLE,
         WS_EX_NONE,
         "Bio:",
         static_props,
         NULL,
         NULL, NULL, 0, 0},
        {NCSCTRL_TEXTEDITOR,
         ID_INFO,
         MARGIN_LEFT + LABEL_WIDTH + SPACING, MARGIN_TOP + (FIELD_HEIGHT + SPACING) * 5, EDITOR_WIDTH, EDITOR_HEIGHT,
         WS_BORDER | WS_VISIBLE | WS_VSCROLL | NCSS_EDIT_BASELINE,
         WS_EX_NONE,
         "",
         NULL,
         NULL,
         NULL, NULL, 0, 0},

        {NCSCTRL_BUTTON,
         ID_SIGNUP,
         240, 380, 80, 25,
         WS_VISIBLE | NCSS_NOTIFY,
         WS_EX_NONE,
         "Sign Up",
         NULL,
         btn_rdr_info,
         btn_handlers, NULL, 0, 0},
        {NCSCTRL_BUTTON,
         ID_CANCEL,
         120, 380, 80, 25,
         WS_VISIBLE | NCSS_NOTIFY,
         WS_EX_NONE,
         "Cancel",
         NULL,
         btn_rdr_info,
         btn_handlers, NULL, 0, 0},
};
//END_OF_TEMPLATE

static NCS_MNWND_TEMPLATE mymain_templ =
    {
        NCSCTRL_DIALOGBOX,
        1,
        ACTIVITY_X,
        ACTIVITY_Y,
        ACTIVITY_W,
        ACTIVITY_H,
        WS_NONE,
        WS_EX_NONE,
        "Sign Up",
        NULL,
        NULL,
        mymain_handlers,
        _ctrl_templ,
        sizeof(_ctrl_templ) / sizeof(NCS_WND_TEMPLATE),
        0,
        0,
        0,
};

SignUpActivity::SignUpActivity() : NCSActivity(&mymain_templ, false)
{
    m_style = STYLE_PUSH;
}

SignUpActivity::~SignUpActivity()
{
}
