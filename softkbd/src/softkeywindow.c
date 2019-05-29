/*
** $Id$
**
** softkeywindow.c.
**
** Copyright (C) 2008 FMSoft.
**
** All right reserved by FMSoft.
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
#include "softkeyboard.h"
#include "tooltip.h"

#define HEIGHT_TASKBAR  40

//#define _MGI_IS_ONLY_INPUT_DEV

#if KBD_ANIMATE
#include "animate/animate.h"
#include "animate/common_animates.h"
#endif

#define ENTER() printf(">>>   enter %s\n", __FUNCTION__);
#define LEAVE() printf("<<<   leave %s\n", __FUNCTION__);
#define dbg() printf("%s %d\n", __FUNCTION__, __LINE__)

#define MSG_IME_USER_SENDCHAR   8000

static void (*op_cb)(BOOL) = NULL;
static key_board_t* keyboard [SFKB_NUM];
static BOOL active = TRUE;

/*ime status table*/
static int status_table [4][4] = {
   {-1,  3,  1, -1},
   { 0,  3, -1,  2},
   { 0,  3,  1, -1},
   { 0, -1,  1, -1}
};

static int convert2utf8(unsigned char *buffer, const unsigned char* characters, size_t mbs_length, char* encode)
{
    ENTER();
    int conved_mbs_len, ucs_len;
    char buffer2[1024];

    //PLOGFONT logfont = CreateLogFont (NULL, "arial", "gb2312",
    PLOGFONT logfont = CreateLogFont (NULL, "arial", encode,
            FONT_WEIGHT_REGULAR, FONT_SLANT_ROMAN, FONT_SETWIDTH_NORMAL,
            FONT_SPACING_CHARCELL, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE,
            12, 0);
    assert(logfont);

    const unsigned char* source = characters;
    ucs_len = MBS2WCSEx (logfont, (void *)buffer2, FALSE,  source, mbs_length, sizeof(buffer2),  &conved_mbs_len);
    DestroyLogFont(logfont);

    logfont = CreateLogFont (NULL, "arial", "utf8",
            FONT_WEIGHT_REGULAR, FONT_SLANT_ROMAN, FONT_SETWIDTH_NORMAL,
            FONT_SPACING_CHARCELL, FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE,
            12, 0);
    assert(logfont);

    ucs_len = WCS2MBSEx (logfont, (unsigned char *)buffer,   (unsigned char *)buffer2,
            ucs_len, FALSE, sizeof(buffer2),  &conved_mbs_len);

    DestroyLogFont(logfont);
    LEAVE();
    return ucs_len;
}

static void send_utf8_string(HWND target_hwnd, char* str, int len)
{
    ENTER();
    int i = 0;
    while(i<len) {
        WPARAM wParam;
        LPARAM lParam = 0;
        //Process UTF8 flow
        if(((Uint8)str[i]) <= 0x7F) {//ascii code
            wParam = (WPARAM)str[i];
            i++;
        } else if(((Uint8)str[i]) <= 0xBF) {
            i ++;
            continue;
        } else if(((Uint8)str[i]) <= 0xDF) { //2 code
            wParam = ((Uint8) str[i])|(((Uint8)str[i+1])<<8);
            i += 2;
        } else if(((Uint8)str[i]) <= 0xEF) { //three code
            wParam = ((Uint8)str[i]) | (((Uint8)str[i+1])<<8) | (((Uint8)str[i+2])<<16);
            i += 3;
        } else if(((Uint8)str[i]) <= 0xF7) { //four code
            wParam = ((Uint8)str[i]) | (((Uint8)str[i+1])<<8) | (((Uint8)str[i+2])<<16)| (((Uint8)str[i+3])<<24);
            i += 4;
        } else if(((Uint8)str[i]) <= 0xFB) { //five code
            wParam = ((Uint8)str[i]) | (((Uint8)str[i+1])<<8) | (((Uint8)str[i+2])<<16)| (((Uint8)str[i+3])<<24);
            lParam = ((Uint8)str[i+4]);
            i += 5;
        } else if(((Uint8)str[i]) <= 0xFD) { //six code
            wParam = ((Uint8)str[i]) | (((Uint8)str[i+1])<<8) | (((Uint8)str[i+2])<<16)| (((Uint8)str[i+3])<<24);
            lParam = ((Uint8)str[i+4]) | (((Uint8)str[i+5])<<8);
            i += 6;
        } else {
            i ++;
            continue;
        }
#if defined(_MGRM_PROCESSES) && (MINIGUI_MAJOR_VERSION > 1) && !defined(_STAND_ALONE)
        Send2ActiveWindow(mgTopmostLayer, MSG_CHAR, wParam, lParam);
#elif defined(_MGRM_THREADS) && !defined(_STAND_ALONE)
        PostMessage(target_hwnd, MSG_CHAR, wParam, lParam);
#endif
        LEAVE();
    }
}

//send 2-Byte-coded chinese character to target window
static void send_ch_string(HWND target_hwnd, char* word, int len)
{
    //ENTER();
    int i;
    for (i=0; i<len; i += 2) {
        WORD wDByte;
        wDByte = MAKEWORD(word[i], word[i + 1]);
#if defined(_MGRM_PROCESSES) && (MINIGUI_MAJOR_VERSION > 1) && !defined(_STAND_ALONE)
        Send2ActiveWindow(mgTopmostLayer, MSG_CHAR, wDByte, 0);
#elif defined(_MGRM_THREADS) && !defined(_STAND_ALONE)
        PostMessage(target_hwnd, MSG_CHAR, wDByte, 0);
        printf("wDByte = %x\n", wDByte);
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
#if defined(_MGRM_PROCESSES) && (MINIGUI_MAJOR_VERSION > 1) && !defined(_STAND_ALONE)
                Send2ActiveWindow(mgTopmostLayer, MSG_CHAR, word[i], 0);
#elif defined(_MGRM_THREADS) && !defined(_STAND_ALONE)
                PostMessage(pdata->target_hwnd, MSG_CHAR, word[i], 0);
#endif
            }
            break;
        case IME_LANGUAGE_ZHTW:
            //printf("IME_LANGUAGE_ZHTW\n");
            if(pdata->ime_status_encode == IME_ENCODING_LOCAL) {
                send_ch_string(pdata->target_hwnd, word, len);
            } else if(pdata->ime_status_encode == IME_ENCODING_UTF8) {
                //printf("IME_ENCODING_UTF8\n");
                unsigned char buffer[1024];
                char* encoding = "big5";
                len =  convert2utf8(buffer, (unsigned char *)word, len, encoding);
                //word = (char *)buffer;
                send_utf8_string(pdata->target_hwnd, (char*)buffer, len);
            }
            break;
        case IME_LANGUAGE_ZHCN:
            //printf("IME_LANGUAGE_ZHCN\n");
            if(pdata->ime_status_encode == IME_ENCODING_LOCAL) {
                //printf("IME_ENCODING_LOCAL\n");
                send_ch_string(pdata->target_hwnd, word, len);
            } else if(pdata->ime_status_encode == IME_ENCODING_UTF8) {
                //printf("IME_ENCODING_UTF8\n");
                unsigned char buffer[1024];
                char* encoding = "gb2312";
                len =  convert2utf8(buffer, (unsigned char *)word, len, encoding);
                word = (char *)buffer;
                send_utf8_string(pdata->target_hwnd, word, len);
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
    y = RECTH(g_rcDesktop)-HEIGHT_TASKBAR;
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
    if (op_cb)
        op_cb(pdata->is_opened);
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
        case MSG_IME_OPEN:
            //printf("MSG_IME_OPEN\n");
            {
#ifndef MGDESKTOP_VERSION
                show_ime_window(hWnd, pdata, TRUE, wParam);

                if(GetIMEStatus(IME_STATUS_AUTOTRACK)) {
                    IME_TARGET_INFO info;
                    GetIMETargetInfo(&info);
                    SendMessage(hWnd, MSG_IME_SET_TARGET_INFO, 0, (LPARAM)(&info));
                }
#endif
            }
            break;
        case MSG_IME_CLOSE:
            //printf("MSG_IME_CLOSE\n");
#ifndef MGDESKTOP_VERSION
            show_ime_window(hWnd, pdata, FALSE, wParam);
#endif
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
#if defined(_MGRM_PROCESSES) && (MINIGUI_MAJOR_VERSION > 1) && !defined(_STAND_ALONE)
        Send2ActiveWindow (mgTopmostLayer, message, wParam, lParam);
#elif defined(_MGRM_THREADS) && !defined(_STAND_ALONE)
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
#if defined(_MGRM_PROCESSES) && (MINIGUI_MAJOR_VERSION > 1) && !defined(_STAND_ALONE)
            Send2ActiveWindow(mgTopmostLayer,
                pdata->keyboard->action.message,
                pdata->keyboard->action.wParam,
                pdata->keyboard->action.lParam);
#elif defined(_MGRM_THREADS) && !defined(_STAND_ALONE)
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

#ifdef _MGRM_PROCESSES
HWND mgiCreateSoftKeypad(void (*cb)(BOOL IsShown))
{
   op_cb = cb;
   return create_ime_win(HWND_DESKTOP);
}

#else

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
HWND mgiCreateSoftKeypad(void (*cb)(BOOL IsShown))
{
    op_cb = cb;
    IME_INFO ime_info;
    pthread_attr_t new_attr;

    sem_init(&ime_info.wait, 0, 0);

    pthread_attr_init(&new_attr);
    pthread_attr_setdetachstate(&new_attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&imethread, &new_attr, start_ime, &ime_info);
    pthread_attr_destroy(&new_attr);

    sem_wait(&ime_info.wait);
    sem_destroy(&ime_info.wait);

    return ime_info.hwnd;
}

#endif
