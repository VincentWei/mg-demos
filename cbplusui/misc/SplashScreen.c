/*
** This file is a part of mg-demos package.
**
** Copyright (C) 2019 FMSoft (http://www.fmsoft.cn).
** Copyright (C) 2018 Beijing Joobot Technologies Inc.
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
 * @file SplashScreen.c
 * @Synopsis Splash screen.
 * @author Vincent Wei
 * @version 1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include "global.h"
#include "utilities.h"
#include "misc.h"

#define _FULL_SCREEN
#include "resource.h"

#define IDC_ANIMATION   100
#define IDC_TIMER_QUIT  100

#ifdef OEM_TPP
    #define LOGO_FILE     "images/splash/taopaipai.gif"
#else
    #define LOGO_FILE     "images/splash/joopic.gif"
#endif

#define LOGO_W        138
#define LOGO_H        44

static BOOL splash_onCreate (mWidget* self, DWORD add_data)
{
    HWND hwnd;
    char gif_path [MAX_PATH + 1];
    strcpy (gif_path, GetResPath ());
    strcat (gif_path, "/");
    strcat (gif_path, LOGO_FILE);

    ANIMATION* anim = CreateAnimationFromGIF89aFile (HDC_SCREEN, gif_path);
    if (anim == NULL) {
		_ERR_PRINTF ("CBPlus>SplashScreen: failed to load animaton from %s.\n", gif_path);
        return FALSE;
    }

    hwnd = CreateWindow (CTRL_ANIMATION,
        "",
        WS_VISIBLE,
        IDC_ANIMATION,
        (SCREEN_W - LOGO_W)/2,
        (SCREEN_H - LOGO_H)/2,
        LOGO_W,
        LOGO_H,
        self->hwnd,
        (DWORD)anim);

    SetWindowBkColor (hwnd, PIXEL_black);
    SendMessage (hwnd, ANM_STARTPLAY, 0, 0);
    return TRUE;
}

static BOOL splash_onCommand (mWidget* self, int id, int nc, HWND hCtrl)
{
    if (id == IDC_ANIMATION && nc == ANNC_NOFRAME) {
        ASYNC_TASK_PROC task_proc = (ASYNC_TASK_PROC) GetWindowAdditionalData (self->hwnd);
        if (task_proc) {
            task_proc (self->hwnd);
        }
        else {
            SetTimer (self->hwnd, IDC_TIMER_QUIT, 100);
        }
    }

    return TRUE;
}

static void splash_onTimer (mWidget *self, int id, DWORD count)
{
    if (id == IDC_TIMER_QUIT) {
        KillTimer (self->hwnd, IDC_TIMER_QUIT);
        SendNotifyMessage (self->hwnd, MSG_CLOSE, 0, 0);
    }
}

static NCS_EVENT_HANDLER splash_handlers[] = {
    { MSG_CREATE, splash_onCreate },
    { MSG_TIMER, splash_onTimer },
    { MSG_COMMAND, splash_onCommand },
    { 0, NULL }
};

//define the main window template
static NCS_MNWND_TEMPLATE splash_templ = {
    class_name: NCSCTRL_DIALOGBOX,
    id: 1,
    x: 0,
    y: 0,
    w: SCREEN_W,
    h: SCREEN_H,
    style: WS_VISIBLE,
    ex_style: WS_EX_NONE,
    caption: "Splash Screen",
    handlers: splash_handlers,
    bk_color: 0xFF000000,
};

void splashScreen (HWND hosting, ASYNC_TASK_PROC task_proc)
{
    splash_templ.user_data = (DWORD)task_proc;
    mDialogBox* mydlg = (mDialogBox *)ncsCreateMainWindowIndirect (&splash_templ, hosting);
    _c(mydlg)->doModal (mydlg, TRUE);
}

