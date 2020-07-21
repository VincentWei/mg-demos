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
** softkeyboard.h: soft keyboard head file.
**
** Create date: 2020/07/18
**
*/

#ifndef __SOFTKEYBOARD_H__
#define __SOFTKEYBOARD_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>

#ifdef SOFTKBD_320_240
#include "320x240/size_320x240.h"
#elif defined(SOFTKBD_480_272)
#include "480x272/size_480x272.h"
#elif defined(SOFTKBD_240_320)
#include "240x320/size_240x320.h"
#endif

#ifndef _MY_PRINTF
#define _MY_PRINTF(fmt...) fprintf(stderr, fmt)
#endif

#define SFKB_NUM 4

/* indicate that this key is normal character key */
#define KEY_PAD_CHAR 1

/* indicate that this key is functional key */
#define KEY_PAD_FUNC 2

/* indicate that this key is pressed */
#define KEY_PAD_PRESSED 4

/* indicate that this key shouldn't be drawed. */
#define KEY_PAD_DRAWED 8

#define SCANCODE_TONUM SCANCODE_USER + 1
#define SCANCODE_TOPY SCANCODE_USER + 2
#define SCANCODE_TOEN SCANCODE_USER + 3
#define SCANCODE_TOOP SCANCODE_USER + 4

#define EN_KBD_BKG 0
#define NUM_KBD_BKG 1
#define PUNCT_KBD_BKG 2
#define RIME_KBD_BKG 3
#define CHAR_KEY_MASK 4
#define FUNC_KEY_MASK 5
#define LEFT_ARROW_ENABLE 6
#define LEFT_ARROW_DISABLE 7
#define RIGHT_ARROW_ENABLE 8
#define RIGHT_ARROW_DISABLE 9
#define TTW_BK_BMP 10
#define TTW_MASK_BMP 11
#define TTW_FONT_BMP 12

#define __FILL_DIRECT__ 1

typedef struct _md_key_t
{

    /* the corrosponding rect of the key pad */
    RECT bound;

    /* the corresponging character of this key pad
     * such as 'a','b' etc
     */
    char key_char;

    /* the correcponging scancode of this key pad.
     * such as SCANCODE_F1~F3,BACKSPACE,ENTER,UP,DOWN,LEFT,RIGHT...
     */
    int scan_code;

    /* the style of this key pad.
     * KEY_CHAR indaces the keypad is a character key pad
     * KEY_FUNC indaces the keypad is a functional key pad
     */
    int style;

    /* private data for method update */
    void *data;

    /* this function is used to update the rect of key,
     * which decided by key_pad.
     * for example:
     * if(key->style & KEY_PAD_PRESSED) {
     *    draw the image what key pressed.
     * } else {
     *    draw the image what key released.
     * }
     */
    void (*update)(struct _md_key_t *key, HWND hwnd);

} md_key_t;

/* indicate that page up key is visible or invisible*/
#define VW_SHOW_PU 0x10

/* indicate that page down key is visible or invisible*/
#define VW_SHOW_PD 0x20

/* indicate that page up key is pressed */
#define VW_PU_PRESSED 0x1

/* indicate that page down key is pressed */
#define VW_PD_PRESSED 0x2

/* indicate that elements of view window should be drawn*/
#define VW_DRAW_ELMTS 0x40

/* indicate that one element of view window was pressed */
#define VW_EL_PRESSED 0x4

/* length of element string */
#define VW_ELMT_LEN 16

/* element of view window */
typedef struct _vw_element
{
    RECT bound;
    char string[VW_ELMT_LEN];
    int len;
    char *index;
} vw_element_t;

#define VW_BUFFER_LEN 128
#define VW_ELMENT_NR 16

typedef struct _vw_add_data_t
{
    BITMAP pu_normal;
    BITMAP pu_press;

    BITMAP pd_normal;
    BITMAP pd_press;

    BITMAP sel_bk;
} vw_add_data_t;

typedef struct _view_window_t
{
    /* rect of whole view window  */
    RECT bound;

    /* page up key pad */
    RECT key_pg_up;

    /* page down key pad */
    RECT key_pg_down;

    /* buffer to save the string set by IME methods*/
    char buffer[VW_BUFFER_LEN];

    PLOGFONT view_font;

    /* max string length of this view window. The max_str_len is
     *  used to set buffer length (also called length of one page)
     *  in IME methods.
     */
    int max_str_len;

    /* keeps all elements of current view window*/
    vw_element_t elements[VW_ELMENT_NR];

    /* sum of elements */
    int element_num;

    /* set elments according to str, hwnd*/
    void (*set_elements)(struct _view_window_t *view_window, HWND hwnd);

    /* clear elements */
    void (*clear_elements)(struct _view_window_t *view_window);

    /* this function is used to get element of current point*/
    vw_element_t *(*get_element)(struct _view_window_t *view_window, POINT p);

    /* the style of page up/down key.
     * VW_SHOW_UP indaces page up key must be shown
     * VW_SHOW_DOWN indaces page down key must be shown
     */
    int style;

    /* private data for method update */
    void *data;

    /* this function is used to update the rect of up/down key,
     * which decided by view_window.
     * for example:
     * if(view_window->style & VW_SHOW_PU) {
     *     if(view_window->style & VW_PU_PRESSED) {
     *        draw the image what page_up_key pressed.
     *     } else {
     *        draw the image what page_up_key released.
     *     }
     * } else if(view_window->style & VW_SHOW_PD) {
     *     if(view_window->style & VW_PD_PRESSED) {
     *        draw the image what page_down_key pressed.
     *     } else {
     *        draw the image what page_down_key released.
     *     }
     * }
     */
    void (*update)(struct _view_window_t *view_window,
                   HWND hwnd, vw_element_t *element);

} view_window_t;

#define SW_STR_LEN 32
typedef struct _stroke_window_t
{
    /* rect of whole stroke window  */
    RECT bound;

    /*the stings will be on the stroke window*/
    char str[SW_STR_LEN];

    /* private data for method update */
    void *data;

    PLOGFONT stroke_font;

    /*this function is used to set the strings in the stroke window*/
    void (*update)(struct _stroke_window_t *stoke_window, HWND hwnd);
} stroke_window_t;

typedef struct _kw_add_data_t
{
    /* for char key press */
    BITMAP char_key_press;

    /* for func key press */
    BITMAP func_key_press;

#ifdef KBD_TOOLTIP
    /*handle of the tooltip window*/
    HWND tooltip_win;
#endif
} kw_add_data_t;

typedef struct _key_window_t
{
    /* rect of whole key window  */
    RECT bound;

    /* point to the pressed postion*/
    POINT press;

    /* pointer to keys[] */
    md_key_t *key;

    /* the number of the keys[] */
    int key_num;

    /*this function is used to get the key structure .*/
    md_key_t *(*get_key)(struct _key_window_t *key_window, POINT point);

    /* private data for method update */
    void *data;

    /*this function is used to update the keyboard window*/
    void (*update)(struct _key_window_t *key_window, HWND hwnd);
} key_window_t;

#define METHOD_NAME_MAX 32
typedef int (*ime_callback)(const char *strokes, char *buffer, int buffer_len, int index, int mode);

typedef struct _md_ime_t
{
    int method_id;
    char method_name[METHOD_NAME_MAX];

    ime_callback match_keystrokes;
    ime_callback translate_word;
    ime_callback predict_word;
    int (*init)(void *);

} md_ime_t;

#define AC_NULL 0
#define AC_SEND_MSG 1
#define AC_SEND_EN_STRING 2
#define AC_SEND_CN_STRING 3
#define AC_CHANGE_KBD 4

typedef struct _action_t
{
    int operation; //AC_SEND_MSG, AC_CHANGE_KBD...
    int message;
    int wParam;
    int lParam;
    char *str; //word to send;
    /*unused*/
    //int next_kbd_id;
} action_t;

typedef struct _key_board_t
{
    /*close rect*/
    RECT close_rc;
    /* the corrosponding rect of the view window */
    view_window_t *view_window;

    /* the corrosponding rect of the stoke window */
    stroke_window_t *stroke_window;

    /* the corrosponding rect of the keyboard window */
    key_window_t *key_window;

    /*the add data used to update the key window */
    void *data;

    /*this function is used to update whole keybord. Called by window process in MSG_ERASEBKGND */
    //void (*update)(struct _key_board_t* key_board, HWND hwnd, RECT* rect);
    void (*update)(struct _key_board_t *key_board, HWND hwnd, WPARAM wParam, RECT *rect);

    /*pointer of current input method operations*/
    md_ime_t *ime;

    /*actions to let hwnd do*/
    action_t action;

    /* this function is used to proceed messages we cared
     * for example:
     * int rime_proceed_msg(struct _key_bord_t* key_board, HWND hwnd, int message, WPARAM wParam, LPARAM lParam)
     * {
     *    static RECT old_hit_rect;
     *    POINT p;
     *
     *    switch(message) {
     *    case MSG_LBUTTONDOWN:
     *    //...
     *    case MSG_LBUTTONUP:
     *        p.x = LOWORD(wParam);
     *        p.y = HIWORD(wParam);
     *        if(hit_rect(key_board->view_window->bound, p)) {
     *            if(hit_rect(key_board->view_window->key_pg_up)){
     *                //...
     *            }else if(hit_rect(key_board->view_window->key_pg_down)){
     *                //...
     *            }else{
     *                ele_ment_t* e = key_board->view_window->get_element(key_board->view_window, p);
     *                if(e == NULL){
     *                    key_board->action.operation = AC_NULL;
     *                    return AC_NULL;
     *                }
     *                key_board->action.operation = AC_SEND_STRING;
     *                key_board->action.str = e->string;
     *                return AC_SEND_CN_STRING;
     *            }
     *        }else if(hit_rect(key_board->view_window->bound, p)) {
     *            //...
     *        }
     *    case MSG_MOUSEMOVE:
     *            //...
     *    case MSG_KEYDOWN: {
     *        switch (wParam){
     *        case SCANCODE_F2:
     *            key_board->action.operation = AC_CHANGE_KBD;
     *            key_board->action.next_bkd_id = status_table[md_ime_data_t->key_borad_id][y];
     *            return AC_CHANGE_KBD;
     *            }
     *        }
     *        case MSG_CHAR:
     *                //...
     *        }
     * }
     **/
    int (*proceed_msg)(struct _key_board_t *key_board, HWND hwnd, int message, WPARAM wParam, LPARAM lParam);

    /*
     * clear the current state to default state
     */
    void (*clear)(struct _key_board_t *kb);
} key_board_t;

typedef struct _SOFTKBD_DATA
{
    key_board_t *keyboard;
    int is_opened;
    int is_closing;

    int ime_status_language;
    int ime_status_encode;
    int ime_status_user;

    HWND target_hwnd;
    int current_board_idx;
#ifdef KBD_TOOLTIP
    HWND tooltip_win;
#endif
} SOFTKBD_DATA;

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    int init_en_keyboard(HWND hWnd, key_board_t *kb);
    void destroy_en_keyboard(key_board_t *kb);
    int init_rime_keyboard(HWND hWnd, key_board_t *kb);
    void destroy_rime_keyboard(key_board_t *kb);
    int init_num_keyboard(HWND hWnd, key_board_t *kb);
    void destroy_num_keyboard(key_board_t *kb);
    int init_punct_keyboard(HWND hWnd, key_board_t *kb);
    void destroy_punct_keyboard(key_board_t *kb);

    int get_kbd_bitmap(HDC hdc, PBITMAP pbmp, int id);
    void release_kbd_bitmap(PBITMAP pbmp);

    int get_kbd_mybitmap(PMYBITMAP pmybmp, RGB *pal, int id);
    void release_kbd_mybitmap(PMYBITMAP pmybmp);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // __SOFTKEYBOARD_H__
