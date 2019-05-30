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
** pinyin_kbd.c: This file include pinyin soft keyboard code.
**
** Create date: 2008/07/15
*/

#include "common.h"
#include "ime.h"
#include "tooltip.h"

static md_key_t py_key_pads[] = {
    { RECT_EN_KEY_1 , 'q', SCANCODE_Q,          KEY_PAD_CHAR, NULL, share_key_update},
    { RECT_EN_KEY_2 , 'w', SCANCODE_W,          KEY_PAD_CHAR, NULL, share_key_update},
    { RECT_EN_KEY_3 , 'e', SCANCODE_E,          KEY_PAD_CHAR, NULL, share_key_update},
    { RECT_EN_KEY_4 , 'r', SCANCODE_R,          KEY_PAD_CHAR, NULL, share_key_update},
    { RECT_EN_KEY_5 , 't', SCANCODE_T,          KEY_PAD_CHAR, NULL, share_key_update},
    { RECT_EN_KEY_6 , 'y', SCANCODE_Y,          KEY_PAD_CHAR, NULL, share_key_update},
    { RECT_EN_KEY_7 , 'u', SCANCODE_U,          KEY_PAD_CHAR, NULL, share_key_update},
    { RECT_EN_KEY_8 , 'i', SCANCODE_I,          KEY_PAD_CHAR, NULL, share_key_update},
    { RECT_EN_KEY_9 , 'o', SCANCODE_O,          KEY_PAD_CHAR, NULL, share_key_update},
    { RECT_EN_KEY_10, 'p', SCANCODE_P,          KEY_PAD_CHAR, NULL, share_key_update},
    { RECT_EN_KEY_11, 'a', SCANCODE_A,          KEY_PAD_CHAR, NULL, share_key_update},
    { RECT_EN_KEY_12, 's', SCANCODE_S,          KEY_PAD_CHAR, NULL, share_key_update},
    { RECT_EN_KEY_13, 'd', SCANCODE_D,          KEY_PAD_CHAR, NULL, share_key_update},
    { RECT_EN_KEY_14, 'f', SCANCODE_F,          KEY_PAD_CHAR, NULL, share_key_update},
    { RECT_EN_KEY_15, 'g', SCANCODE_G,          KEY_PAD_CHAR, NULL, share_key_update},
    { RECT_EN_KEY_16, 'h', SCANCODE_H,          KEY_PAD_CHAR, NULL, share_key_update},
    { RECT_EN_KEY_17, 'j', SCANCODE_J,          KEY_PAD_CHAR, NULL, share_key_update},
    { RECT_EN_KEY_18, 'k', SCANCODE_K,          KEY_PAD_CHAR, NULL, share_key_update},
    { RECT_EN_KEY_19, 'l', SCANCODE_L,          KEY_PAD_CHAR, NULL, share_key_update},
    { RECT_EN_KEY_20, ' ', SCANCODE_LEFTSHIFT,  KEY_PAD_FUNC, NULL, share_key_update},
    { RECT_EN_KEY_21, 'z', SCANCODE_Z,          KEY_PAD_CHAR, NULL, share_key_update},
    { RECT_EN_KEY_22, 'x', SCANCODE_X,          KEY_PAD_CHAR, NULL, share_key_update},
    { RECT_EN_KEY_23, 'c', SCANCODE_C,          KEY_PAD_CHAR, NULL, share_key_update},
    { RECT_EN_KEY_24, 'v', SCANCODE_V,          KEY_PAD_CHAR, NULL, share_key_update},
    { RECT_EN_KEY_25, 'b', SCANCODE_B,          KEY_PAD_CHAR, NULL, share_key_update},
    { RECT_EN_KEY_26, 'n', SCANCODE_N,          KEY_PAD_CHAR, NULL, share_key_update},
    { RECT_EN_KEY_27, 'm', SCANCODE_M,          KEY_PAD_CHAR, NULL, share_key_update},
    { RECT_EN_KEY_28, ' ', SCANCODE_BACKSPACE,  KEY_PAD_FUNC, NULL, share_key_update},
    { RECT_EN_KEY_29, ' ', SCANCODE_TONUM,      KEY_PAD_FUNC, NULL, share_key_update},
    { RECT_EN_KEY_30, ' ', SCANCODE_TOEN,       KEY_PAD_FUNC, NULL, share_key_update},
    { RECT_EN_KEY_31, ' ', SCANCODE_SPACE,      KEY_PAD_CHAR, NULL, share_key_update},
    { RECT_EN_KEY_32, ' ', SCANCODE_ENTER,      KEY_PAD_FUNC, NULL, share_key_update},
};

static md_ime_t py_ime = {
    100,
    "pinyin",
    NULL,
    cb_pinyin_translate_word,
    cb_pinyin_predict_word,
    cb_pinyin_init
};

static md_key_t* get_py_key(key_window_t *kw, POINT p)
{
    int i;
    md_key_t* keys = kw->key;

    for(i = 0; i < kw->key_num; i++) {
        if (PtInRect(&keys[i].bound, p.x, p.y))
            return &keys[i];
    }

    return NULL;
}

static void kw_update(key_window_t *kw, HWND hWnd)
{
    //TODO
    //printf("==== update key window ====\n");
}

static void sw_update(stroke_window_t* sw, HWND hWnd)
{
    HDC hdc;
    PLOGFONT old_font;
    gal_pixel old_tecolor;

    EraseBbGround(hWnd, &sw->bound);
    if (strlen(sw->str) != 0) {
        hdc = GetDC(hWnd);

        SetBkMode(hdc, BM_TRANSPARENT);
        old_tecolor = SetTextColor(hdc, RGB2Pixel(hdc, 155, 50, 155));
        old_font = SelectFont(hdc, sw->stroke_font);
        DrawText(hdc, sw->str, -1, &(sw->bound), DT_LEFT);
        SelectFont(hdc, old_font);
        SetTextColor(hdc, old_tecolor);

        ReleaseDC(hdc);
    }
}

static void vw_update(view_window_t *vw, HWND hWnd, vw_element_t* element)
{
    int i;
    RECT r;
#ifndef __FILL_DIRECT__
    RECT rc;
#endif
    PBITMAP pbmp;
    static int old_style = 0;
    HDC hdc = GetDC(hWnd);

    r.left = vw->key_pg_up.right;
    r.top  = vw->bound.top;
    r.right = vw->key_pg_down.left;
    r.bottom = vw->bound.bottom;

    SelectFont(hdc, vw->view_font);
    SetBkMode (hdc, BM_TRANSPARENT);
    if(vw->style & VW_DRAW_ELMTS) {
        if(element == NULL) {
            element = vw->elements;
            EraseBbGround(hWnd, &r);
            for (i=0; i<vw->element_num; i++) {
#ifndef __FILL_DIRECT__
                SetTextColor(hdc, RGB2Pixel(hdc, 190, 190, 190));
                DrawText(hdc, element[i].string, -1, &element[i].bound, 0);

                rc = element[i].bound;
                rc.top --;
                rc.bottom --;
                rc.left --;
                rc.right --;
                SetTextColor(hdc, RGB2Pixel(hdc, 155, 50, 155));
                DrawText(hdc, element[i].string, -1, &rc, 0);
#else
                DrawText(hdc, element[i].string, -1, &element[i].bound, 0);
#endif
            }
        } else {
            if(vw->style & VW_EL_PRESSED) {
                if (vw->data)
#ifdef __FILL_DIRECT__
    #if 0
                    FillBoxWithBitmap(hdc,element->bound.left,
                            element->bound.top,
                            element->bound.right - element->bound.left,
                            element->bound.bottom - element->bound.top,
                            &(VIEWWIN_DATA_PTR(vw)->sel_bk) );
    #else
                //SetBrushColor(hdc, RGB2Pixel(hdc, 120, 187, 250));
                SetBrushColor(hdc, RGB2Pixel(hdc, 61, 89, 160));
                FillBox(hdc,element->bound.left, element->bound.top,
                            element->bound.right - element->bound.left,
                            element->bound.bottom - element->bound.top);
    #endif
                //SetTextColor(hdc, COLOR_lightwhite);
                SetTextColor(hdc, RGB2Pixel(hdc, 155, 50, 155));
                DrawText(hdc, element->string, -1, &element->bound, 0);
#else
                rc = element->bound;
                rc.top --;
                rc.left --;
                EraseBbGround(hWnd, &rc);
                rc.top += 2;
                rc.left += 2;
                rc.bottom ++;
                rc.right ++;
                SetTextColor(hdc, RGB2Pixel(hdc, 100, 50, 100));
                DrawText(hdc, element->string, -1, &rc, 0);
#endif
            } else {
                EraseBbGround(hWnd, &vw->bound);
            }

        }
    } else if(old_style & VW_DRAW_ELMTS) {
        EraseBbGround(hWnd, &r);
    }

    EraseBbGround(hWnd, &vw->key_pg_up);

    if(vw->style & VW_SHOW_PU) {
        if(vw->style & VW_PU_PRESSED) {
            pbmp = &(VIEWWIN_DATA_PTR(vw)->pu_press);
        } else {
            pbmp = &(VIEWWIN_DATA_PTR(vw)->pu_normal);
        }
        FillBoxWithBitmap (hdc, vw->key_pg_up.left, vw->key_pg_up.top,
                vw->key_pg_up.right-vw->key_pg_up.left,
                vw->key_pg_up.bottom-vw->key_pg_up.top, pbmp);
    }

    EraseBbGround(hWnd, &vw->key_pg_down);

    if(vw->style & VW_SHOW_PD) {
        if(vw->style & VW_PD_PRESSED) {
            pbmp = &(VIEWWIN_DATA_PTR(vw)->pd_press);
        } else {
            pbmp = &(VIEWWIN_DATA_PTR(vw)->pd_normal);
        }
        FillBoxWithBitmap (hdc, vw->key_pg_down.left, vw->key_pg_down.top,
                vw->key_pg_down.right-vw->key_pg_down.left,
                vw->key_pg_down.bottom-vw->key_pg_down.top, pbmp);
    }

    old_style = vw->style;
    ReleaseDC(hdc);
}

int pinyin_proceed_msg(key_board_t* key_board, HWND hwnd,
        int message, WPARAM wParam, LPARAM lParam)
{
    static int lbuttondown = 0;
    static POINT p;
    static md_key_t *key = NULL;

    switch(message) {
        case MSG_LBUTTONDOWN:
            p.x = LOSWORD(lParam);
            p.y = HISWORD(lParam);
            lbuttondown = 1;
            key_board->action.operation = AC_NULL;

            if (PtInRect(&key_board->view_window->bound, p.x, p.y)) {
                vw_proceed_hit(hwnd, key_board->view_window,
                        key_board->stroke_window, &key_board->action,
                        TRUE, p, CN, key_board->ime);
                break;
            }

            if (PtInRect(&key_board->key_window->bound, p.x, p.y)) {
                key = key_board->key_window->get_key
                    (key_board->key_window, p);
                if(key == NULL) {
                    key_board->action.operation = AC_NULL;
                    break;
                }

                kw_proceed_hit(hwnd, key_board->view_window,
                        key_board->stroke_window, key, &key_board->action,
                        TRUE, p, CN, key_board->ime, wParam, lParam);

                break;
            }

            break;
        case MSG_LBUTTONUP:
            if (lbuttondown == 0) {
                key_board->action.operation = AC_NULL;
                break;
            }

            lbuttondown = 0;
            p.x = LOSWORD(lParam);
            p.y = HISWORD(lParam);

            if (PtInRect(&key_board->close_rc, p.x, p.y)) {
                key_board->action.operation = AC_NULL;
                SendMessage (hwnd, MSG_IME_CLOSE, 0, 0);
                return AC_NULL;
            }

            if (PtInRect(&key_board->view_window->bound, p.x, p.y)) {
                vw_proceed_hit(hwnd, key_board->view_window,
                        key_board->stroke_window, &key_board->action,
                        FALSE, p, CN, key_board->ime);
                break;
            }

            if (PtInRect(&key_board->key_window->bound, p.x, p.y)) {
                 key = key_board->key_window->get_key
                    (key_board->key_window, p);
                if(key == NULL) {
                    key_board->action.operation = AC_NULL;
                    break;
                }

                kw_proceed_hit(hwnd, key_board->view_window,
                        key_board->stroke_window, key, &key_board->action,
                        FALSE, p, CN, key_board->ime, wParam, lParam);

                break;
            }
            break;

        case MSG_MOUSEMOVE:
            p.x = LOSWORD(lParam);
            p.y = HISWORD(lParam);
            proceed_move(hwnd, key_board, lbuttondown, p, EN);
            break;
        case MSG_NCMOUSEMOVE:
            if(key && HT_OUT == wParam) {
                key->style &= ~KEY_PAD_PRESSED;
                key->update(key, hwnd);
#ifdef KBD_TOOLTIP
                HideToolTip((HWND)((SOFTKBD_DATA *)GetWindowAdditionalData(hwnd))->tooltip_win);
#endif
                lbuttondown = 0;
                key = NULL;
            }
            return AC_NULL;

    }

    return key_board->action.operation;
}

static int init_py_view_window (HWND hWnd, view_window_t *vw)
{
    vw->bound.left          = SKB_VW_L   ;
    vw->bound.top           = SKB_VW_T + 2;
    vw->bound.right         = SKB_VW_R   ;
    vw->bound.bottom        = SKB_VW_B   ;

    vw->key_pg_up.left      = SKB_VW_PU_L;
    vw->key_pg_up.top       = SKB_VW_PU_T;
    vw->key_pg_up.right     = SKB_VW_PU_R;
    vw->key_pg_up.bottom    = SKB_VW_PU_B;

    vw->key_pg_down.left    = SKB_VW_PD_L;
    vw->key_pg_down.top     = SKB_VW_PD_T;
    vw->key_pg_down.right   = SKB_VW_PD_R;
    vw->key_pg_down.bottom  = SKB_VW_PD_B;

#ifdef SOFTKBD_320_240
    vw->max_str_len = 22;
#elif defined (SOFTKBD_480_272)
    vw->max_str_len = 40;
#elif defined (SOFTKBD_240_320)
    vw->max_str_len = 20;
#endif

    vw->update = vw_update;
    vw->set_elements = vw_set_elements;
    vw->clear_elements = vw_clear_elements;
    vw->get_element = vw_get_element;

    vw->view_font = CreateLogFontByName ("ttf-fixed-rrncnn-*-12-GB2312");

    if (NULL == vw->view_font){
        _MY_PRINTF("create logfont for view window error.\n");
        return -1;
    }

    if (NULL == (vw->data = (void *)calloc (1, sizeof(vw_add_data_t)))){
        _MY_PRINTF("no memory for key window add data.\n");
        return -1;
    }

    if (get_kbd_bitmap(HDC_SCREEN,
                &(VIEWWIN_DATA_PTR(vw)->pu_normal),
                LEFT_ARROW_ENABLE)) {
        _MY_PRINTF ("Fail to get page-up mask bitmap.\n");
        return -1;
    }

    if (get_kbd_bitmap(HDC_SCREEN,
                &(VIEWWIN_DATA_PTR(vw)->pu_press),
                LEFT_ARROW_DISABLE)) {
        _MY_PRINTF ("Fail to get page-up mask bitmap.\n");
        return -1;
    }

    if (get_kbd_bitmap(HDC_SCREEN,
                &(VIEWWIN_DATA_PTR(vw)->pd_normal),
                RIGHT_ARROW_ENABLE)) {
        _MY_PRINTF ("Fail to get page-down mask bitmap.\n");
        return -1;
    }

    if (get_kbd_bitmap(HDC_SCREEN,
                &(VIEWWIN_DATA_PTR(vw)->pd_press),
                RIGHT_ARROW_DISABLE)) {
        _MY_PRINTF ("Fail to get page-down mask bitmap.\n");
        return -1;
    }
/*
    if (get_kbd_bitmap(HDC_SCREEN,
                &(VIEWWIN_DATA_PTR(vw)->sel_bk),
                SELECT_TEXT_BKGND)) {
        _MY_PRINTF ("Fail to get sellect text bkgnd bitmap.\n");
        return -1;
    }
*/
    return 0;
}

static void destroy_py_view_window (view_window_t *vw)
{

    if (vw->data) {
        release_kbd_bitmap(&(VIEWWIN_DATA_PTR(vw)->pu_normal));
        release_kbd_bitmap(&(VIEWWIN_DATA_PTR(vw)->pu_press));
        release_kbd_bitmap(&(VIEWWIN_DATA_PTR(vw)->pd_normal));
        release_kbd_bitmap(&(VIEWWIN_DATA_PTR(vw)->pd_press));
        release_kbd_bitmap(&(VIEWWIN_DATA_PTR(vw)->sel_bk));
        free(vw->data);
    }

    if (vw->view_font)
        DestroyLogFont(vw->view_font);
}

static int init_py_stroke_window(HWND hWnd, stroke_window_t *sw)
{
    sw->bound.left   = SKB_SW_L;
    sw->bound.top    = SKB_SW_T;
    sw->bound.right  = SKB_SW_R;
    sw->bound.bottom = SKB_SW_B;

    memset(sw->str, 0, SW_STR_LEN);

#if 0
    sw->stroke_font = CreateLogFont (NULL, "times", "ISO8859-1",
                        FONT_WEIGHT_BOOK, FONT_SLANT_ROMAN,
                        FONT_SETWIDTH_NORMAL, FONT_OTHER_NIL,
                        FONT_UNDERLINE_NONE, FONT_STRUCKOUT_NONE,
                        9, 0);
#endif
    sw->stroke_font = CreateLogFontByName ("ttf-times-rrncnn-*-9-ISO8859-1");
    if (NULL == sw->stroke_font){
        _MY_PRINTF("create logfont for stroke window error.\n");
        return -1;
    }

    sw->update = sw_update;

    return 0;
}

static void destroy_py_stroke_window(stroke_window_t *sw)
{
    if (sw->stroke_font)
        DestroyLogFont(sw->stroke_font);
}

static int init_py_key_window(HWND hWnd, key_window_t *kw)
{
    int i;
    md_key_t *key;

    kw->bound.left   = SKB_KW_L;
    kw->bound.top    = SKB_KW_T;
    kw->bound.right  = SKB_KW_R;
    kw->bound.bottom = SKB_KW_B;

    kw->press.x = 0;
    kw->press.y = 0;

    kw->key = py_key_pads;
    kw->key_num = TABLESIZE(py_key_pads);

    kw->get_key = get_py_key;

    if (NULL == (kw->data = (void *)calloc (1, sizeof(kw_add_data_t)))){
        _MY_PRINTF("no memory for key window add data.\n");
        return -1;
    }

    if (get_kbd_bitmap(HDC_SCREEN,
                &(KEYWIN_DATA_PTR(kw)->char_key_press),
                CHAR_KEY_MASK)) {
        _MY_PRINTF ("Fail to get py char key mask bitmap.\n");
        return -1;
    }

    if (get_kbd_bitmap(HDC_SCREEN,
                &(KEYWIN_DATA_PTR(kw)->func_key_press),
                FUNC_KEY_MASK)) {
        _MY_PRINTF ("Fail to get py func key mask bitmap.\n");
        return -1;
    }

    kw->update = kw_update;

    /*use the shared bitmap for press key painting.*/
    key = kw->key;
    for (i = 0; i < kw->key_num; i++){
        if(key->style == KEY_PAD_CHAR)
            key->data = &(KEYWIN_DATA_PTR(kw)->char_key_press);
        else
            key->data = &(KEYWIN_DATA_PTR(kw)->func_key_press);
        key++;
    }
    return 0;
}

static void destroy_py_key_window(key_window_t *kw)
{
    if (kw->data) {
        release_kbd_bitmap(&(KEYWIN_DATA_PTR(kw)->char_key_press));
        release_kbd_bitmap(&(KEYWIN_DATA_PTR(kw)->func_key_press));
        free(kw->data);
    }
}

int init_py_keyboard(HWND hWnd, key_board_t *kb)
{
    kb->close_rc.left   = SKB_CLOSE_L;
    kb->close_rc.top    = SKB_CLOSE_T;
    kb->close_rc.right  = SKB_CLOSE_R;
    kb->close_rc.bottom = SKB_CLOSE_B;

    /*initalize for view window.*/
    kb->view_window = (view_window_t *)calloc(1, sizeof(view_window_t));
    if(kb->view_window == NULL){
        _MY_PRINTF("no memory for view window.\n");
        return -1;
    }
    if (init_py_view_window (hWnd, kb->view_window)){
        _MY_PRINTF("error for initalizing view window.\n");
        return -1;
    }

    /*initalize for stroke window.*/
    kb->stroke_window = (stroke_window_t *)calloc(1, sizeof(stroke_window_t));
    if (kb->stroke_window == NULL){
        _MY_PRINTF("no memory for view window.\n");
        return -1;
    }

    if (init_py_stroke_window (hWnd, kb->stroke_window)){
        _MY_PRINTF("error for initalizing stroke window.\n");
        return -1;
    }

    /*initalize for key window.*/
    kb->key_window = (key_window_t *)calloc(1, sizeof(key_window_t));
    if (kb->key_window == NULL){
        _MY_PRINTF("no memory for key window.\n");
        return -1;
    }
    if (init_py_key_window(hWnd, kb->key_window)) {
        _MY_PRINTF("error for initalizing key window.\n");
        return -1;
    }

    /*initalize for bk image.*/
    kb->data = calloc(1, sizeof(BITMAP));
    if (kb->data == NULL) {
        _MY_PRINTF("no memory for bk image.\n");
        return -1;
    }

    if (get_kbd_bitmap(HDC_SCREEN, (PBITMAP)(kb->data), PINYIN_KBD_BKG)) {
        _MY_PRINTF ("Fail to get pinyin kbd bkgnd bitmap.\n");
        return -1;
    }

    kb->update = keyboard_update;

    kb->ime = &py_ime;
    kb->ime->init(NULL);

    memset (&(kb->action), 0, sizeof(action_t));

    kb->proceed_msg = pinyin_proceed_msg;
    kb->clear = clear_keyboard;

    return 0;
}

void destroy_py_keyboard(key_board_t *kb)
{
    /*view window*/
    destroy_py_view_window(kb->view_window);
    free(kb->view_window);

    /*stroke window*/
    destroy_py_stroke_window(kb->stroke_window);
    free(kb->stroke_window);

    /*ky window*/
    destroy_py_key_window(kb->key_window);
    free(kb->key_window);

    /*add data*/
    release_kbd_bitmap((PBITMAP)(kb->data));
    free(kb->data);
}
