/*
** This file is a part of mg-demos package.
**
** Copyright (C) 2019 FMSoft (http://www.fmsoft.cn).
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
 * @file main.cc
 * @Synopsis
 * @author Vincent Wei
 * @version 1.0
 * @date 29/05/2019
 *
 * Copyright (C) 2019 Beijing FMSoft Technologies Co., Ltd.
 */

#include <pthread.h>
#include <semaphore.h>

#include "global.h"
#include "utilities.h"

#include "Activity.hh"
#include "ActivityStack.hh"

#include "MyGetText.hh"
#include "cJSON.h"

#include "softkeyboard.h"

#include "resource.h"

static mWidget *widgetStatusBar;

static void format_current_time (char* buff, int size, const char* fmt)
{
    struct tm tm;
    time_t curr_time = time (NULL);

    localtime_r (&curr_time, &tm);
    strftime (buff, size, fmt, &tm);
}

static char time_buff[16];
static BOOL on_time_changed (HWND hwnd, LINT id, DWORD data)
{
    char str_time[16];
    format_current_time (str_time, sizeof (str_time), "%H:%M");
    if (strcmp (str_time, time_buff)) {
        strcpy (time_buff, str_time);
        SetWindowText (hwnd, time_buff);
        InvalidateRect (hwnd, NULL, TRUE);
    }

    return TRUE;
}

static BOOL statusBar_onCreate (mMainWnd* self, DWORD dwAddData)
{
    mWidget* ctrl;
    ctrl = (mWidget*)(_c(self)->getChild (self, IDC_TITLE));
    if (ctrl) {
        format_current_time(time_buff, sizeof(time_buff), "%H:%M");
        SetWindowText (ctrl->hwnd, time_buff);
        ncsSetElement (ctrl, NCS_FGC_WINDOW, COLOR_TITLE);
        ncsSetFont (ctrl->hwnd, FONT_TITLE);
        InvalidateRect (ctrl->hwnd, NULL, TRUE);
        SetTimerEx (ctrl->hwnd, 100, 100, on_time_changed);
    }

    return TRUE;
}

static void statusBar_onClose (mWidget* self, UINT message)
{
    while (TRUE) {
        if (ACTIVITYSTACK->depth () == 0) {
            break;
        }

        ACTIVITYSTACK->pop ();
    }
}

static void statusBar_onDestroy (mWidget* self, UINT message)
{
    PostQuitMessage (self->hwnd);
}

static NCS_EVENT_HANDLER _statusbar_handlers [] = {
    {MSG_CREATE, reinterpret_cast<void*>(statusBar_onCreate)},
    {MSG_CLOSE, reinterpret_cast<void*>(statusBar_onClose)},
    {MSG_DESTROY, reinterpret_cast<void*>(statusBar_onDestroy)},
    {0, NULL}
};

static NCS_WND_TEMPLATE _ctrl_templ[] =
{
    {
        class_name: NCSCTRL_STATIC,
        id: IDC_TITLE,
        x: 0,
        y: 0,
        w: STATUSBAR_W,
        h: STATUSBAR_H - 1,
        style: WS_VISIBLE,
        ex_style: WS_EX_NONE,
        caption: "",
    },
    {
        class_name: NCSCTRL_SEPARATOR,
        id: IDC_STATIC,
        x: 0,
        y: STATUSBAR_H - 1,
        w: STATUSBAR_W,
        h: 1,
        style: WS_VISIBLE,
        ex_style: WS_EX_NONE,
        caption: "",
    },
};

static NCS_MNWND_TEMPLATE _statusbar_templ = {
    NCSCTRL_MAINWND,
    1,
    0, 0, SCREEN_W, STATUSBAR_H,
    WS_DISABLED, WS_EX_AUTOSECONDARYDC,
    "Status Bar",
    NULL,
    NULL,
    _statusbar_handlers,
    _ctrl_templ,
    TABLESIZE(_ctrl_templ),
    0, 0, NULL, 0, 0,
    NULL,
    0
};

int main (int argc, const char* argv[])
{
    /* Before calling InitGUI, you can override the default video mode
     * by seting the environment variables.
     * You can also change the GAL engine and IAL engine in this way.
    setenv ("MG_DEFAULTMODE", "480x640-16bpp.rgb565", 1);
    */

    if (InitGUI (argc, argv) != 0) {
        _ERR_PRINTF ("CBPlusGUI>main: failed to call InitGUI.\n");
        return 1;
    }

#ifdef _MGRM_PROCESSES
    JoinLayer(NAME_DEF_LAYER , "SoftKBD", 0, 0);
#endif

    /* init mGNCS and mGNCS4Touch */
    ncsInitialize ();
    ncs4TouchInitialize ();

    /* Register all activities here */
    DO_REGISTER_ACTIVITY (SignUpActivity);

    /* set renderer */
    SetDefaultWindowElementRenderer ("flat");
    ncsSetSystemRenderer ("flat");

    /*
     * Change the background color of dialog box and main window by
     * setting the window class information, instead changing
     * the windows one by one.
     */
    WNDCLASS wnd_class;
    wnd_class.spClassName = NCSCTRL_DIALOGBOX;
    wnd_class.opMask = COP_BKCOLOR;
    wnd_class.iBkColor = PIXEL_black;
    SetWindowClassInfo (&wnd_class);

    wnd_class.spClassName = NCSCTRL_MAINWND;
    SetWindowClassInfo (&wnd_class);

    /* The framework needs a global hosting main window, and
     * before calling mGEffInit, we need to create a main window.
     * Therefore, we create the status bar window first.
     */
    widgetStatusBar = ncsCreateMainWindowIndirect (&_statusbar_templ, HWND_DESKTOP);
    SET_GLOBAL_HOSTING (widgetStatusBar->hwnd);
    mGEffInit ();

    /* It's time to show the status bar and launcher the first activity */
    ShowWindow (widgetStatusBar->hwnd, SW_SHOW);

    ACTIVITYSTACK->push ("SignUpActivity");

    /* Now enter the message loop */
    MSG Msg;
    while (GetMessage (&Msg, widgetStatusBar->hwnd)) {
        TranslateMessage (&Msg);
        DispatchMessage (&Msg);
    }
    DestroyMainWindow (widgetStatusBar->hwnd);

    mGEffDeinit ();

    /* more cleanup */
    releaseRefMemDC32b ();

    ncs4TouchUninitialize ();
    ncsUninitialize ();

    /* call exit instead of TerminateGUI if you want exit completely */
    exit (0);
    // TerminateGUI (0);

    return 0;
}

