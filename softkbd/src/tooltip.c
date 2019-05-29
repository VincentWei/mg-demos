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
** tooltip.c: Create the tooltip window for soft keyboard.
**
** Created by Wang Jian at 2008-07-23.
*/

#include <stdio.h>
#include <string.h>

#include "softkeywindow.h"
#include "tooltip.h"

static BITMAP bmp_f;
static DEVFONT *dev_font;

static int init_ttw_data(HWND hWnd, TTW_PDATA padd)
{
    int i;

    if (get_kbd_mybitmap(&(padd->mask), padd->pal, TTW_MASK_BMP)) {
        fprintf(stderr, "Fail to load bitmap for tooltip window mask. \n");
        return 1;
    }
    if (get_kbd_bitmap(HDC_SCREEN, &(padd->bk_bmp), TTW_BK_BMP)) {
        fprintf (stderr, "Fail to load bitmap for tooltip window BKGND. \n");
        return 1;
    }

#if 0
    InitFreetypeLibrary();

    LoadDevFontFromFile ("ttf-times-rrncnn-0-0-ISO8859-1", "times.ttf");

    padd->pfont = CreateLogFont ("ttf", "times", "ISO8859-1",
                        FONT_WEIGHT_BOOK, FONT_SLANT_ROMAN,
                        FONT_FLIP_NIL, FONT_OTHER_NIL,
                        FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE,
                        40, 0);
#endif
#if 0
    padd->pfont = CreateLogFont ("rbf", "times", "ISO8859-1",
                        FONT_WEIGHT_BOOK, FONT_SLANT_ROMAN,
                        FONT_FLIP_NIL, FONT_OTHER_AUTOSCALE,
                        FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE,
                        18, 0);
#else
    //if (LoadBitmap (HDC_SCREEN, &bmp_f, "bitmap_font.png")) {
    if (get_kbd_bitmap (HDC_SCREEN, &bmp_f, TTW_FONT_BMP)) {
        fprintf (stderr, "Fail to load bitmap for tooltip window font. \n");
        return 1;
    }

    dev_font = CreateBMPDevFont("bmp-iphone-rrncnn-30-33-ISO8859-1", &bmp_f, "!", 94, 30);
    padd->pfont = CreateLogFont(FONT_TYPE_NAME_BITMAP_BMP, "iphone", "ISO8859-1",
                                FONT_WEIGHT_REGULAR, FONT_SLANT_ROMAN,
                                FONT_SETWIDTH_NORMAL, FONT_SPACING_CHARCELL,
                                FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE,
                                10, 0);
#endif
    for (i=0; i<TTW_PAL_LEN; ++i) {
        if (padd->pal[i].r == TTW_MASK_R && padd->pal[i].g == TTW_MASK_G
                && padd->pal[i].b == TTW_MASK_B) {
            padd->mask.transparent = i;
            break;
        }
    }
    return 0;
}

static int destroy_ttw_data(HWND hWnd)
{
    TTW_PDATA padd = (TTW_PDATA)GetWindowAdditionalData(hWnd);

    release_kbd_mybitmap(&(padd->mask));
    release_kbd_bitmap(&(padd->bk_bmp));
    DestroyLogFont(padd->pfont);
    free(padd);

    return 0;
}

static LRESULT TTWinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    TTW_PDATA padd;
    static char str[2];

    switch (message) {
    case MSG_CREATE:
        str[0] = ' ';
        str[1] = '\0';

        padd = (TTW_PDATA)calloc(1, sizeof(TTW_DATA));
        if (0 != init_ttw_data(hWnd, padd)) {
            fprintf(stderr, "init nr window error!");
            return -1;
        }

        if (!SetWindowMask(hWnd, &padd->mask)){
            fprintf(stderr, "set window mask erro!");
            destroy_ttw_data (hWnd);
            return -1;
        }

        SetWindowAdditionalData(hWnd, (DWORD)padd);
        break;
    case MSG_LBUTTONUP: {
        int x = LOSWORD(lParam);
        int y = HISWORD(lParam);

        ClientToScreen(hWnd, &x, &y);
        ScreenToClient(GetHosting(hWnd), &x, &y);
        SendMessage(GetHosting(hWnd), MSG_LBUTTONUP, 0, MAKELONG(x, y));
        ShowWindow(hWnd, SW_HIDE);
        break;
    }
    case MSG_ERASEBKGND:
        padd = (TTW_PDATA)GetWindowAdditionalData(hWnd);
        if (padd) {
            BOOL fGetDC = FALSE;
            hdc = (HDC)wParam;
            if (!hdc) {
                hdc = GetClientDC(hWnd);
                fGetDC = TRUE;
            }
            FillBoxWithBitmap(hdc, 0, 0, 0, 0, &(padd->bk_bmp));
            if (fGetDC)
                ReleaseDC(hdc);
        }
        return 0;
    case MSG_PAINT:
        padd = (TTW_PDATA)GetWindowAdditionalData(hWnd);
        if (padd) {
            hdc = BeginPaint(hWnd);
            RECT rc = {0, 0, TTW_W, TTW_H};
            SetBkMode(hdc, BM_TRANSPARENT);
            SelectFont(hdc, padd->pfont);

            /* space have not bitmap glyph. */
            if (str[0] != 0x20)
                DrawText(hdc, str, -1, &rc, DT_CENTER);

            EndPaint(hWnd, hdc);
         }
         return 0;
    case MSG_SETCHAR:
        str[0] = (char)wParam;
        str[1] = '\0';

        InvalidateRect(hWnd, NULL, TRUE);
        return 0;
    case MSG_CLOSE:
        if (dev_font)
            DestroyBMPFont(dev_font);
        UnloadBitmap(&bmp_f);

        DestroyMainWindow(hWnd);
        PostQuitMessage(hWnd);
        // destroy_ttw_data(hWnd);
        return 0;
    case MSG_DESTROY:
        destroy_ttw_data(hWnd);
        return 0;
    }
    return DefaultMainWinProc(hWnd, message, wParam, lParam);
}

HWND CreateToolTip(HWND host)
{
    HWND hNrWnd;
    MAINWINCREATE CreateInfo;

    CreateInfo.dwStyle      = WS_VISIBLE;
    CreateInfo.dwExStyle    = WS_EX_TOPMOST | WS_EX_TOOLWINDOW;
    CreateInfo.spCaption    = "Char Tooltip Window";
    CreateInfo.hMenu        = 0;
    CreateInfo.hCursor      = GetSystemCursor(0);
    CreateInfo.hIcon        = 0;
    CreateInfo.MainWindowProc = TTWinProc;
    CreateInfo.lx           = 0;
    CreateInfo.ty           = 0;
    CreateInfo.rx           = TTW_W;
    CreateInfo.by           = TTW_H;
    CreateInfo.iBkColor     = PIXEL_lightgray;
    CreateInfo.dwAddData    = (DWORD)0;
    CreateInfo.hHosting     = host;

    hNrWnd = CreateMainWindow(&CreateInfo);
    ShowWindow(hNrWnd, SW_HIDE);

    return hNrWnd;
}

int ShowToolTip(HWND hwnd, int x, int y, char key_char)
{
    MoveWindow(hwnd, x, y, TTW_W, TTW_H, FALSE);
    SendMessage(hwnd, MSG_SETCHAR, key_char, 0L);
    ShowWindow(hwnd, SW_SHOWNORMAL);
    return 0;
}

