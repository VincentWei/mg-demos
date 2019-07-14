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
** softkeywindow.c.
**
** Create date: 2008/7/11
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>

#include "common.h"
#include "softkeywindow.h"
#include "tooltip.h"

#if KBD_ANIMATE
#include "animate/animate.h"
#include "animate/common_animates.h"
#endif

#ifdef DEBUG
#define ENTER() printf(">>>   enter %s\n", __FUNCTION__);
#define LEAVE() printf("<<<   leave %s\n", __FUNCTION__);
#define dbg() printf("%s %d\n", __FUNCTION__, __LINE__)
#else
#define ENTER()
#define LEAVE()
#define dbg()
#endif

#define MSG_IME_USER_SENDCHAR   8000

static key_board_t* keyboard [SFKB_NUM];
static BOOL active = TRUE;
static int HEIGHT_TASKBAR;

/*ime status table*/
static int status_table [4][4] = {
   {-1,  3,  1, -1},
   { 0,  3, -1,  2},
   { 0,  3,  1, -1},
   { 0, -1,  1, -1}
};

static int uc32_to_utf8(unsigned int c, char* outbuf)
{
    int len = 0;
    int first;
    int i;

    if (c < 0x80) {
        first = 0;
        len = 1;
    }
    else if (c < 0x800) {
        first = 0xc0;
        len = 2;
    }
    else if (c < 0x10000) {
        first = 0xe0;
        len = 3;
    }
    else if (c < 0x200000) {
        first = 0xf0;
        len = 4;
    }
    else if (c < 0x4000000) {
        first = 0xf8;
        len = 5;
    }
    else {
        first = 0xfc;
        len = 6;
    }

    if (outbuf) {
        for (i = len - 1; i > 0; --i) {
            outbuf[i] = (c & 0x3f) | 0x80;
            c >>= 6;
        }
        outbuf[0] = c | first;
    }

    return len;
}

static int convert2utf8(PLOGFONT logfont, char *buffer,
        const unsigned char* mbs, size_t mbs_len)
{
    int i, converted, ucs_len;
    unsigned int wcs[256];

    assert(logfont);

    ENTER();

    ucs_len = MBS2WCSEx (logfont, wcs, TRUE,
                mbs, mbs_len, sizeof(wcs), &converted);

    for (i = 0; i < ucs_len; i++) {
        int n = uc32_to_utf8 (wcs[i], buffer);
        buffer += n;
    }
    *buffer = '\0';

    LEAVE();
    return ucs_len;
}

static int utf8_len_first_char (const char* mstr, int len)
{
    int t, c = *((const unsigned char *)(mstr++));

    if (c & 0x80) {
        int n = 1, ch_len = 0;
        while (c & (0x80 >> n))
            n++;

        if (n > len)
            return 0;

        ch_len = n;
        while (--n > 0) {
            t = *((const unsigned char *)(mstr++));

            if ((!(t & 0x80)) || (t & 0x40))
                return 0;
        }

        return ch_len;
    }

    /* for ascii character */
    return 1;
}

static void send_utf8_string(HWND target_hwnd, const char* str, int nr_ucs)
{
    ENTER();

    int len = strlen(str);
    while (len > 0) {
        WPARAM wParam;
        LPARAM lParam = 0;

        int ch_len = utf8_len_first_char (str, len);
        switch (ch_len) {
        case 1:
            wParam = (WPARAM)str[0];
            break;
        case 2:
            wParam = ((Uint8)str[0]) | (((Uint8)str[1])<<8);
            break;
        case 3:
            wParam = ((Uint8)str[0]) | (((Uint8)str[0+1])<<8) | (((Uint8)str[0+2])<<16);
            break;
        case 4:
            wParam = ((Uint8)str[0]) | (((Uint8)str[0+1])<<8) | (((Uint8)str[0+2])<<16)| (((Uint8)str[0+3])<<24);
            break;
        case 5:
            wParam = ((Uint8)str[0]) | (((Uint8)str[0+1])<<8) | (((Uint8)str[0+2])<<16)| (((Uint8)str[0+3])<<24);
            lParam = ((Uint8)str[0+4]);
            break;
        case 6:
            wParam = ((Uint8)str[0]) | (((Uint8)str[0+1])<<8) | (((Uint8)str[0+2])<<16)| (((Uint8)str[0+3])<<24);
            lParam = ((Uint8)str[0+4]) | (((Uint8)str[0+5])<<8);
            break;
        default:
            return;
        }

#if defined(_MGRM_PROCESSES) && (MINIGUI_MAJOR_VERSION > 3)
        Send2ActiveWindow(mgTopmostLayer, MSG_CHAR, wParam, lParam);
#else
        PostMessage(target_hwnd, MSG_CHAR, wParam, lParam);
#endif

        str += ch_len;
        len -= ch_len;
    }

    LEAVE();
}

//send 2-Byte-coded chinese character to target window
static void send_ch_string(HWND target_hwnd, char* word, int len)
{
    //ENTER();
    int i;
    for (i=0; i<len; i += 2) {
        WORD wDByte;
        wDByte = MAKEWORD(word[i], word[i + 1]);
#if defined(_MGRM_PROCESSES) && (MINIGUI_MAJOR_VERSION > 3)
        Send2ActiveWindow(mgTopmostLayer, MSG_CHAR, wDByte, 0);
#else
        PostMessage(target_hwnd, MSG_CHAR, wDByte, 0);
#endif
    }
    //LEAVE();
}

static void send_imeword_to_target(SOFTKBD_DATA* pdata, char *word, int len)
{
    int i = 0;
    if(len<=0 || word == NULL)
        return;

    switch(pdata->ime_status_language) {
        case IME_LANGUAGE_LATIN:
            for (i=0; i<len; i++) {
#if defined(_MGRM_PROCESSES) && (MINIGUI_MAJOR_VERSION > 3)
                Send2ActiveWindow(mgTopmostLayer, MSG_CHAR, word[i], 0);
#else
                PostMessage(pdata->target_hwnd, MSG_CHAR, word[i], 0);
#endif
            }
            break;
        case IME_LANGUAGE_ZHTW:
            //printf("IME_LANGUAGE_ZHTW\n");
            if(pdata->ime_status_encode == IME_ENCODING_LOCAL) {
                send_ch_string(pdata->target_hwnd, word, len);
            }
            else if(pdata->ime_status_encode == IME_ENCODING_UTF8) {
                //printf("IME_ENCODING_UTF8\n");
                char buffer[1024];
                len = convert2utf8(pdata->keyboard->view_window->view_font,
                    buffer, (unsigned char *)word, len);
                send_utf8_string(pdata->target_hwnd, (char*)buffer, len);
            }
            break;
        case IME_LANGUAGE_ZHCN:
            //printf("IME_LANGUAGE_ZHCN\n");
            if(pdata->ime_status_encode == IME_ENCODING_LOCAL) {
                //printf("IME_ENCODING_LOCAL\n");
                send_ch_string(pdata->target_hwnd, word, len);
            }
            else if(pdata->ime_status_encode == IME_ENCODING_UTF8) {
                //printf("IME_ENCODING_UTF8\n");
                char buffer[1024];
                len = convert2utf8(pdata->keyboard->view_window->view_font,
                    buffer, (unsigned char *)word, len);
                send_utf8_string(pdata->target_hwnd, buffer, len);
            }
            break;

        default:
            assert(0);
    }
    return;
}

static key_board_t* init_keyboard_data(HWND hWnd)
{
    /* for english key board */
    if (!(keyboard[0] = (key_board_t*)calloc(1, sizeof (key_board_t)))) {
        _MY_PRINTF("calloc keyboard data for EN failed\n");
        return NULL;
    }
    if (-1 == init_en_keyboard(hWnd, keyboard[0])) {
        _MY_PRINTF("error for initalize En-keyboard.\n");
        return NULL;
    }

    /* for num key board */
    if (!(keyboard[1] = (key_board_t*)calloc(1, sizeof (key_board_t)))) {
        _MY_PRINTF("calloc keyboard data for NUM failed\n");
        return NULL;
    }
    if (-1 == init_num_keyboard(hWnd, keyboard[1])) {
        _MY_PRINTF("error for initalize num-keyboard.\n");
        return NULL;
    }

    /* for punct key board */
    if (!(keyboard[2] = (key_board_t*)calloc(1, sizeof (key_board_t)))) {
        _MY_PRINTF("calloc keyboard data for punctuation failed\n");
        return NULL;
    }
    if (-1 == init_punct_keyboard(hWnd, keyboard[2])) {
        _MY_PRINTF("error for initalize Puctuation-keyboard.\n");
        return NULL;
    }

    /* for py key board */
    if (!(keyboard[3] = (key_board_t*)calloc(1, sizeof (key_board_t)))) {
        _MY_PRINTF("calloc keyboard data for EN failed\n");
        return NULL;
    }
    if (-1 == init_py_keyboard(hWnd, keyboard[3])) {
        _MY_PRINTF("error for initalize PinYin-keyboard.\n");
        return NULL;
    }

    //return the default keyboard at beginning.
    return keyboard[0];
}

static void destroy_key_win(void)
{
   //FIXME:
    destroy_en_keyboard(keyboard[0]);
    free(keyboard[0]);
    destroy_num_keyboard(keyboard[1]);
    free(keyboard[1]);
    destroy_punct_keyboard(keyboard[2]);
    free(keyboard[2]);
    destroy_py_keyboard(keyboard[3]);
    free(keyboard[3]);
}

#if KBD_ANIMATE
static void on_imewnd_jmp_finished(ANIMATE_SENCE* as)
{
    SOFTKBD_DATA* pdata;
    pdata = (SOFTKBD_DATA*)as->param;
    if (!pdata->is_opened) {
        ShowWindow((HWND)as->normal->img, SW_HIDE);
    } else {
        ShowWindow((HWND)as->normal->img, SW_SHOW);
    }

}
#endif

static void show_ime_window(HWND hWnd, SOFTKBD_DATA* pdata, BOOL show, WPARAM wParam)
{
   if(!pdata)
      return ;

   pdata->is_opened = show;
   softkey_reset();
   if(!show) //hide
   {
      if(pdata && pdata-> keyboard && pdata->keyboard->clear)
         pdata->keyboard->clear(pdata->keyboard);
   }
#if KBD_ANIMATE
    int x, y;
    x = (RECTW(g_rcDesktop) - SKB_WIN_W)/2;
    y = RECTH(g_rcDesktop) - HEIGHT_TASKBAR;
    if (show) {
        SetInterval(50);
        MoveWindow(hWnd, x, y-1, SKB_WIN_W, SKB_WIN_H, FALSE);
        ShowWindow(hWnd, SW_SHOW);
        RunJumpWindow(hWnd, x, y-1, x, y - SKB_WIN_H,
                SKB_WIN_W, SKB_WIN_H, on_imewnd_jmp_finished, pdata);
    } else {
        if (wParam == 0) {
            // use animates
            RunJumpWindow(hWnd, x, y - SKB_WIN_H, x, y,
                    SKB_WIN_W, SKB_WIN_H, on_imewnd_jmp_finished, pdata);
        } else {
            pdata->is_opened = 0;
            ShowWindow(hWnd, SW_HIDE);
        }
    }
#else
    ShowWindow(hWnd, show?SW_SHOW: SW_HIDE);
    pdata->is_opened = show;
#endif
}

static LRESULT DefaultIMEWinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    SOFTKBD_DATA* pdata = NULL;

    if (!active)
        return 0;

    if (message != MSG_CREATE) {
        pdata = (SOFTKBD_DATA*)GetWindowAdditionalData(hWnd);
    }

    switch (message) {
        case MSG_IME_SETPOS:
            //printf("MSG_IME_SETPOS\n");
            return 1;
        case MSG_IME_SETSTATUS:
            //printf("MSG_IME_SETSTATUS\n");
            if (wParam == IME_STATUS_ENCODING) {
                pdata->ime_status_encode = lParam;
            } else if(wParam == IME_STATUS_VERSION)  {
                return 1;
            }
            return 0;
        case MSG_IME_GETSTATUS:
            //printf("MSG_IME_GETSTATUS\n");
            if(lParam) {
                if (wParam == IME_STATUS_LANGUAGE) {
                    *((int*)lParam) = pdata->ime_status_language;
                } else if (wParam == IME_STATUS_ENCODING) {
                    *((int*)lParam) = pdata->ime_status_encode;
                }
            }
            return (int)pdata->is_opened;
        case MSG_IME_SETTARGET:
            //printf("MSG_IME_SETTARGET\n");
            if ((HWND)wParam != hWnd)
                pdata->target_hwnd = (HWND)wParam;
            return 0;
        case MSG_IME_SET_TARGET_INFO:
            //printf("MSG_IME_SET_TARGET_INFO\n");
            {
                IME_TARGET_INFO info = *((IME_TARGET_INFO*)lParam);
                RECT rc;
                GetWindowRect(hWnd, &rc);

#ifndef _MGI_IS_ONLY_INPUT_DEV
                if(info.iEditBoxType == IME_WINDOW_TYPE_PASSWORD) {
                    printf("pass word edit\n");
                    SendMessage(hWnd, MSG_IME_CLOSE, 0, 0);
                    return 0;
                }
#endif

                if (info.ptCaret.x + RECTW(rc) > RECTW(g_rcScr)) {
                    info.ptCaret.x = RECTW(g_rcScr) - RECTW(rc);
                }
                if ((info.ptCaret.y + RECTH(rc)) > RECTH(g_rcScr)) {
                    info.ptCaret.y = info.ptCaret.y -  RECTH(rc);
                }

                MoveWindow(hWnd, info.ptCaret.x, info.ptCaret.y, SKB_WIN_W, SKB_WIN_H, TRUE);
                return 0;
            }
        case MSG_IME_GETTARGET:
            return (LRESULT)pdata->target_hwnd;

        case MSG_IME_USER_SENDCHAR:
            send_imeword_to_target(pdata, pdata->keyboard->action.str, strlen(pdata->keyboard->action.str));
            break;

        case MSG_IME_OPEN: {
            pdata->is_closing = FALSE;
            if (pdata->is_opened)
                break;

            show_ime_window(hWnd, pdata, TRUE, wParam);

            if (GetIMEStatus(IME_STATUS_AUTOTRACK)) {
                IME_TARGET_INFO info;
                GetIMETargetInfo(&info);
                SendMessage(hWnd, MSG_IME_SET_TARGET_INFO, 0, (LPARAM)(&info));
            }
            break;
        }

        case MSG_IME_CLOSE:
            pdata->is_closing = TRUE;
            break;

        case MSG_IDLE:
            if (pdata->is_closing) {
                pdata->is_closing = FALSE;
                show_ime_window(hWnd, pdata, FALSE, wParam);
            }
            break;
    }

    return DefaultMainWinProc(hWnd, message, wParam, lParam);
}

static LRESULT SoftKeyWinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    SOFTKBD_DATA* pdata = NULL;

    if (!active)
        return 0;

    if (message != MSG_CREATE) {
        pdata = (SOFTKBD_DATA*)GetWindowAdditionalData(hWnd);
    }

    switch (message) {
    case MSG_NCCREATE:
        RegisterIMEWindow(hWnd);
        break;

    case MSG_CREATE: {
        if (!(pdata = (SOFTKBD_DATA*)calloc(1, sizeof (SOFTKBD_DATA)))) {
            _MY_PRINTF("Fail to calloc SOFTKBD data.\n");
            return -1;
        }

        pdata->current_board_idx = 0;
        pdata->ime_status_encode = IME_ENCODING_UTF8;
        pdata->ime_status_language = IME_LANGUAGE_LATIN;
        if (NULL == (pdata->keyboard = init_keyboard_data(hWnd))) {
            _MY_PRINTF("Soft Key Window init failed\n");
            return -1;
        }

#ifdef KBD_TOOLTIP
        pdata->tooltip_win = CreateToolTip(hWnd);
#endif
        SetWindowAdditionalData(hWnd, (DWORD)pdata);
        break;
    }
    case MSG_KEYDOWN:
    case MSG_KEYUP:
#if defined(_MGRM_PROCESSES) && (MINIGUI_MAJOR_VERSION > 3)
        Send2ActiveWindow (mgTopmostLayer, message, wParam, lParam);
#else
        PostMessage(pdata->target_hwnd, message, wParam, lParam);
#endif
       return 0;

    case MSG_LBUTTONDOWN:
    case MSG_LBUTTONUP:
    case MSG_MOUSEMOVE:
    case MSG_NCMOUSEMOVE:
#ifdef KBD_TOOLTIP
      if(MSG_LBUTTONUP == message)
         HideToolTip((HWND)((SOFTKBD_DATA *)GetWindowAdditionalData(hWnd))->tooltip_win);
#endif

      if(MSG_LBUTTONDOWN == message || MSG_LBUTTONUP == message)
         reset_mouse_state(hWnd);

       switch (pdata->keyboard->proceed_msg(pdata->keyboard, hWnd, message, wParam, lParam)) {
        case AC_CHANGE_KBD: {
            int y;
            POINT p;

            p.x = LOSWORD(lParam);
            p.y = HISWORD(lParam);

            md_key_t* key = pdata->keyboard->key_window->get_key(pdata->keyboard->key_window, p);
            // FIXME: If get_key return NULL, I think dispatch this message again.
            if (!key)
                break;

            /* english */
            if (key->scan_code == SCANCODE_TOEN){
                y = 0;
                pdata->ime_status_language = IME_LANGUAGE_LATIN;
            }
            /* pinyin */
            if (key->scan_code == SCANCODE_TOPY) {
                y = 1;
                pdata->ime_status_language = IME_LANGUAGE_ZHCN;
            }

            /* 123 */
            if (key->scan_code == SCANCODE_TONUM) {
                y = 2;
                pdata->ime_status_language = IME_LANGUAGE_LATIN;
            }
            /* oprator */
            if (key->scan_code == SCANCODE_TOOP)   {
                y = 3;
                pdata->ime_status_language = IME_LANGUAGE_LATIN;
            }

            /*clear old state */
            pdata->keyboard->clear(pdata->keyboard);
            pdata->current_board_idx = status_table[pdata->current_board_idx][y];
            pdata->keyboard = keyboard[pdata->current_board_idx];

            SendMessage(hWnd, MSG_ERASEBKGND, 0, 0L);
            softkey_reset();
            return 0;
        }
        case AC_SEND_EN_STRING:
        case AC_SEND_CN_STRING:
            SendMessage(hWnd, MSG_IME_USER_SENDCHAR, 0, 0);
            break;

        case AC_SEND_MSG:
#if defined(_MGRM_PROCESSES) && (MINIGUI_MAJOR_VERSION > 3)
            Send2ActiveWindow(mgTopmostLayer,
                pdata->keyboard->action.message,
                pdata->keyboard->action.wParam,
                pdata->keyboard->action.lParam);
#else
            PostMessage(pdata->target_hwnd,
                pdata->keyboard->action.message,
                pdata->keyboard->action.wParam,
                pdata->keyboard->action.lParam);
#endif
            return 0;
        }
        break;

        case MSG_PAINT: {
            HDC hdc = BeginPaint(hWnd);
            do {
                view_window_t*   view_window   = pdata->keyboard->view_window;
                stroke_window_t* stroke_window = pdata->keyboard->stroke_window;
                if (view_window == NULL || stroke_window == NULL)
                    break;
                view_window->style |= VW_DRAW_ELMTS;
                view_window->style &= ~VW_EL_PRESSED;
                view_window->update(view_window, hWnd, NULL);
                stroke_window->update(stroke_window, hWnd);
            } while(FALSE);

            EndPaint(hWnd, hdc);
            return 0;
        }

        case MSG_ERASEBKGND:
            if (pdata && pdata->keyboard)
                pdata->keyboard->update(pdata->keyboard, hWnd, wParam, (RECT*)lParam);
            return 0;

        case MSG_CLOSE:
            UnregisterIMEWindow(hWnd);
            //SendMessage(HWND_DESKTOP, MSG_IME_UNREGISTER, (WPARAM) hWnd, 0);
            destroy_key_win();
#ifdef KBD_TOOLTIP
            DestroyMainWindow(pdata->tooltip_win);
#endif
            free(pdata);
            DestroyMainWindow(hWnd);
            PostQuitMessage(hWnd);
            return 0;
    }

    return DefaultIMEWinProc(hWnd, message, wParam, lParam);
}

static void init_createinfo(PMAINWINCREATE pCreateInfo)
{
    pCreateInfo->dwStyle = WS_ABSSCRPOS;
    pCreateInfo->dwExStyle = WS_EX_TOPMOST;
    pCreateInfo->spCaption = "Soft Key Window" ;
    pCreateInfo->hMenu = 0;
    pCreateInfo->hCursor = GetSystemCursor (0);
    pCreateInfo->hIcon = 0;
    pCreateInfo->MainWindowProc = SoftKeyWinProc;
    pCreateInfo->lx = (RECTW(g_rcDesktop) - SKB_WIN_W)/2;
    pCreateInfo->ty = RECTH(g_rcDesktop) - SKB_WIN_H - HEIGHT_TASKBAR;
    pCreateInfo->rx = (RECTW(g_rcDesktop) + SKB_WIN_W)/2;
    pCreateInfo->by = RECTH(g_rcDesktop);
    pCreateInfo->iBkColor = COLOR_lightwhite;
    pCreateInfo->dwAddData = 0;
}

static HWND create_ime_win(HWND hosting)
{
    MAINWINCREATE CreateInfo;
    HWND hMainWnd;

    init_createinfo(&CreateInfo);
    CreateInfo.hHosting = hosting;

    hMainWnd = CreateMainWindow(&CreateInfo);
    if (hMainWnd == HWND_INVALID)
        return HWND_INVALID;

    return hMainWnd;
}

void mgiEnableSoftKeypad(BOOL e)
{
   active = e;
}

#ifdef _MGRM_THREADS

typedef struct ime_info {
    sem_t wait;
    HWND hwnd;
} IME_INFO;

static void* start_ime(void* data)
{
    MSG Msg;
    IME_INFO* ime_info = (IME_INFO*) data;
    HWND ime_hwnd;

    ime_hwnd = ime_info->hwnd = create_ime_win(HWND_DESKTOP);
    if (ime_hwnd == HWND_INVALID)
        return NULL;

    sem_post(&ime_info->wait);

    while (GetMessage(&Msg, ime_hwnd)) {
        TranslateMessage (&Msg);
        DispatchMessage(&Msg);
    }

    MainWindowThreadCleanup (ime_hwnd);

    return NULL;
}

static pthread_t imethread;

/* the argument of 'hosting' is ignored. */
HWND skbCreateSoftKeyboard(HWND hosting, int margin_bottom)
{
    IME_INFO ime_info;
    pthread_attr_t new_attr;

    HEIGHT_TASKBAR = margin_bottom;

    sem_init(&ime_info.wait, 0, 0);
    pthread_attr_init(&new_attr);
    pthread_attr_setdetachstate(&new_attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&imethread, &new_attr, start_ime, &ime_info);
    pthread_attr_destroy(&new_attr);

    sem_wait(&ime_info.wait);
    sem_destroy(&ime_info.wait);

    return ime_info.hwnd;
}

#else /* _MGRM_THREADS */

HWND skbCreateSoftKeyboard(HWND hosting, int margin_bottom)
{
    HEIGHT_TASKBAR = margin_bottom;
    return create_ime_win(hosting);
}

#endif /* !_MGRM_THREADS */
