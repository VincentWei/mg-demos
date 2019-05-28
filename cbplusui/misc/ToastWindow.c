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
 * @file ToastWindow.c
 * @Synopsis Toast window
 * @author Vincent Wei
 * @version 1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <ctype.h>

#include "global.h"

#include "utilities.h"

#define _FULL_SCREEN
#include "resource.h"

#define IDC_TIMER   100

static void mymain_onTimer (mWidget* self, LINT id, DWORD tick_count)
{
    if (id == IDC_TIMER) {
        ShowWindow (self->hwnd, SW_HIDE);
        KillTimer (self->hwnd, id);
    }
}

static void mymain_onPaint (mMainWnd *self, HDC hdc, const CLIPRGN* inv)
{
    RECT rc;
    GetClientRect (self->hwnd, &rc);
    rc.left += 5;
    rc.top += 5;
    rc.right -= 5;
    rc.bottom -= 5;

    DWORD color = GetWindowAdditionalData (self->hwnd);
    SetTextColor (hdc, DWORD2Pixel(hdc, color));
    SetBkMode (hdc, BM_TRANSPARENT);
    DrawText (hdc, GetWindowCaption (self->hwnd), -1, &rc, DT_SINGLELINE|DT_CENTER|DT_VCENTER);
}

static NCS_EVENT_HANDLER _mainwnd_handlers [] = {
    {MSG_TIMER, mymain_onTimer},
    {MSG_PAINT, mymain_onPaint},
    {0, NULL}
};

static NCS_MNWND_TEMPLATE _mainwnd_templ = {
    NCSCTRL_MAINWND,
    1,
    TOAST_X, TOAST_Y, TOAST_W, TOAST_H,
    WS_NONE, WS_EX_TOOLWINDOW | WS_EX_AUTOSECONDARYDC | WS_EX_TROUNDCNS | WS_EX_BROUNDCNS,
    "Toast Window",
    NULL,
    NULL,
    _mainwnd_handlers,
    NULL,
    0,
    0,
    COLOR_TOAST_BK, NULL, 0, 0,
    NULL,
    0
};

static mWidget* _widget_toast = NULL;
HWND createToastWindow (HWND hosting)
{
    if (_widget_toast) {
        return _widget_toast->hwnd;
    }

    _widget_toast = ncsCreateMainWindowIndirect (&_mainwnd_templ, hosting);
    ncsSetFont (_widget_toast->hwnd, FONT_TOAST);
    return _widget_toast->hwnd;
}

BOOL destroyToastWindow (void)
{
    if (_widget_toast) {
        DestroyMainWindow (_widget_toast->hwnd);
        _widget_toast = NULL;
        return TRUE;
    }

    return FALSE;
}

void showToastWindow (const char* title, const char* text, DWORD color, BOOL autoHide)
{
    if (_widget_toast == NULL) {
        _ERR_PRINTF ("showToastWindow: please call createToastWindow first.\n");
        return;
    }

    SetWindowAdditionalData (_widget_toast->hwnd, color);
    if (IsWindowVisible (_widget_toast->hwnd)) {
        SetWindowCaption (_widget_toast->hwnd, text);
        UpdateWindow (_widget_toast->hwnd, TRUE);

        KillTimer (_widget_toast->hwnd, IDC_TIMER);
        if (autoHide) {
            SetTimer (_widget_toast->hwnd, IDC_TIMER, 200);
        }
    }
    else {
        SetWindowCaption (_widget_toast->hwnd, text);
        ShowWindow (_widget_toast->hwnd, SW_SHOWNORMAL);
        UpdateWindow (_widget_toast->hwnd, TRUE);
        if (autoHide) {
            SetTimer (_widget_toast->hwnd, IDC_TIMER, 200);
        }
    }
}

void hideToastWindow (void)
{
    if (_widget_toast == NULL) {
        _ERR_PRINTF ("hideToastWindow: please call createToastWindow first.\n");
        return;
    }
    ShowWindow (_widget_toast->hwnd, SW_HIDE);
    KillTimer (_widget_toast->hwnd, IDC_TIMER);
}

