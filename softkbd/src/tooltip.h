/*
** This file is a part of mg-demos package.
**
** Copyright (C) 2008 ~ 2019 FMSoft (http://www.fmsoft.cn).
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
/*
** tooltip.h: tooltip for every key.
**
** Created by Wang Jian at 2008-7-24.
*/

#ifndef __TOOLTIP_H__
#define __TOOLTIP_H__

#define TTW_W           30
#define TTW_H           42

#define TTW_MASK_R      0xff
#define TTW_MASK_G      0x0
#define TTW_MASK_B      0x0

#define TTW_PAL_LEN     256

#define MSG_SETCHAR     MSG_USER + 10

typedef struct {
    /* bk gnd bitmap */
    BITMAP      bk_bmp;
    /* the logfont for showing text */
    PLOGFONT    pfont;
    /* mask for window shape */
    MYBITMAP    mask;
    /* color palette */
    RGB         pal[TTW_PAL_LEN];
} TTW_DATA, *TTW_PDATA;

#ifdef __cplusplus
extern "C" {
#endif

HWND CreateToolTip(HWND host);
int ShowToolTip(HWND hwnd, int x, int y, char key_char);

inline static void HideToolTip(HWND hwnd)
{
    ShowWindow(hwnd, SW_HIDE);
}

#ifdef __cplusplus
}
#endif

#endif /* __TOOLTIP_H__ */

