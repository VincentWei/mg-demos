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

int main (int argc, const char* argv[])
{
    setenv ("MG_DEFAULTMODE", "480x640-16bpp.rgb565", 1);

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
    mGEffInit ();

    ACTIVITYSTACK->push ("LauncherActivity");

#if 0
    MSG Msg;

    while (GetMessage (&Msg, widgetStatusBar->hwnd)) {
        TranslateMessage (&Msg);

        if ((Msg.message >= MSG_FIRSTMOUSEMSG && Msg.message <= MSG_LASTMOUSEMSG) ||
                (Msg.message >= MSG_FIRSTKEYMSG && Msg.message <= MSG_LASTKEYMSG)) {
            ResetTimerEx (widgetStatusBar->hwnd, SCREEN_IDLE_TIMER_ID, sbInfo.screen_lock_timeout, NULL);
        }

        DispatchMessage (&Msg);
    }
    DestroyMainWindow (widgetStatusBar->hwnd);
#endif

    releaseRefMemDC32b ();

    mGEffDeinit ();
    ncs4TouchUninitialize ();
    ncsUninitialize ();
    TerminateGUI (0);

    return 0;
}

