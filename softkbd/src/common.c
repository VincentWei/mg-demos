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
** common.c: This file include common functions for soft keyboard.
**
** Create date: 2008/07/13
*/

#include <ctype.h>

#include "softkeywindow.h"
#include "common.h"
#include "tooltip.h"

static int index_curr = 0;
static int index_next = 0;
static int stack[100] = {0};
static int sp = 0;
static ime_callback ime_method = NULL;
static char stroke[SW_STR_LEN] = {0};
static int case_mode = PTI_CASE_abc;

static vw_element_t* _e_down = NULL;
static md_key_t* _key_down = NULL;

int get_pti_case_mode()
{
    return case_mode;
}

void reset_mouse_state(HWND hwnd)
{
    if(_key_down != NULL)
    {
        if((_key_down->style & KEY_PAD_PRESSED) && _key_down->scan_code != SCANCODE_LEFTSHIFT){
            _key_down->style &= ~KEY_PAD_PRESSED;
            if(hwnd != HWND_INVALID)
                _key_down->update(_key_down, hwnd);
        }
        _key_down = NULL;
    }
}

void softkey_reset(void)
{
    index_curr = 0;
    index_next = 0;
    sp = 0;
    memset (stack, 0, sizeof(stack));
    ime_method = NULL;
    memset(stroke ,0 , SW_STR_LEN);
    case_mode = PTI_CASE_abc;
    _e_down = NULL;
    reset_mouse_state(HWND_INVALID);
}

BOOL try_next(ime_callback func, const char* strokes, int len, int index, int mode)
{
    static char buffer[VW_BUFFER_LEN];
    if (func){
        int r;
        r=func(strokes, buffer, len, index, mode);
        return (BOOL)(r != 0 && r != -1);
    }
    return FALSE;
}

static void append(char* str, char c, int len, int flag)
{
    if(str == NULL || len <= 0)
        return;

    if (strlen(str) >= len)
        return;

    while (*str) str++;

    *(str++) = c;
    *str = 0;
}

static void cut(char* str, int flag)
{
    char* orig = str;

    if(str == NULL)
        return ;

    while (*str)
        str++;

    if (orig == str)
        return;

    *(str - 1) = 0;
}

void strcpylower(char* buf, const char* str)
{
    if(str == NULL || buf == NULL)
        return ;

    while (*str) {
        if (*str >= 'A' && *str <= 'Z')
            *(buf++) = *(str++) + 32;
        else
            *(buf++) = *(str++);
    }
    *buf = 0;
}

void vw_proceed_hit(HWND hwnd, view_window_t* view_window,
        stroke_window_t* stroke_window, action_t* action,
        BOOL pressed, POINT p, int flag, md_ime_t* ime)
{
    static char buf[VW_ELMT_LEN];

    action->operation = AC_NULL;

    if (PtInRect(&view_window->key_pg_down, p.x, p.y)) {
        if(!(view_window->style & VW_SHOW_PD))
            return;

        if (pressed) {
            view_window->style |= VW_DRAW_ELMTS;
            view_window->style |= VW_PD_PRESSED;
            view_window->update(view_window, hwnd, NULL);
            return;
        }

        view_window->style &= ~VW_PD_PRESSED;
        view_window->style |= VW_SHOW_PU;

        //get next page
        if(sp >= 0 && sp < 100)
            stack[sp] = index_curr;
        index_curr = index_next;
        if (ime_method)
            index_next = ime_method(stroke, view_window->buffer,
                    view_window->max_str_len, index_curr, case_mode);
        view_window->set_elements(view_window, hwnd);
        view_window->style |= VW_DRAW_ELMTS;

        if (!try_next(ime_method, stroke, view_window->max_str_len, index_next, case_mode)) {
            view_window->style &= ~VW_SHOW_PD;
        } else {
            view_window->style |= VW_SHOW_PD;
            sp ++;
        }

        view_window->update(view_window, hwnd, NULL);
        view_window->style &= ~VW_DRAW_ELMTS;
        return;
    }

    if (PtInRect(&view_window->key_pg_up, p.x, p.y)) {
        if (!(view_window->style & VW_SHOW_PU))
            return;

        if (pressed) {
            view_window->style |= VW_DRAW_ELMTS;
            view_window->style |= VW_PU_PRESSED;
            view_window->update(view_window, hwnd, NULL);
            return;
        }

        view_window->style &= ~VW_PU_PRESSED;
        view_window->style |= VW_SHOW_PD;
        //get prev page
        sp--;
        if (sp >= 0) {
            index_curr = stack[sp];
            if (ime_method)
                index_next = ime_method(stroke, view_window->buffer,
                        view_window->max_str_len, index_curr, case_mode);
            view_window->set_elements(view_window, hwnd);
            view_window->style |= VW_DRAW_ELMTS;
            if (sp == 0)
                view_window->style &= ~VW_SHOW_PU;
        } else {
            sp = 0;
            view_window->style &= ~VW_SHOW_PU;
        }

        view_window->update(view_window, hwnd, NULL);
        view_window->style &= ~VW_DRAW_ELMTS;
        return;
    }

    {
        vw_element_t* e = view_window->get_element(view_window, p);
        if (e == NULL)
            return;

        view_window->style |= VW_DRAW_ELMTS;
        if (pressed) {
            view_window->style |= VW_EL_PRESSED;
            view_window->update(view_window, hwnd, e);
            return;
        }

        sp = 0;
        index_curr = 0;
        index_next = 0;

        view_window->style &= ~VW_EL_PRESSED;
        view_window->update(view_window, hwnd, e);

        if (flag == CN) {
            memset (buf, 0, VW_ELMT_LEN);
            strcpy(buf, e->string);

            action->operation = AC_SEND_CN_STRING;
            action->str = buf;

            memset(stroke_window->str, 0, SW_STR_LEN);
            stroke_window->update(stroke_window, hwnd);

            strncpy(stroke, e->string + e->len - 2, 2);
            ime_method = ime->predict_word;

            index_next = ime_method(stroke, view_window->buffer,
                    view_window->max_str_len, index_curr, case_mode);

            if(view_window->set_elements)
                view_window->set_elements (view_window, hwnd);

            view_window->style = 0;
            view_window->style |= VW_DRAW_ELMTS;

            if (!try_next(ime_method, stroke, view_window->max_str_len, index_next, case_mode)) {
                view_window->style &= ~VW_SHOW_PD;
            } else {
                view_window->style |= VW_SHOW_PD;
                sp ++;
            }

            view_window->update (view_window, hwnd, NULL);
            view_window->style &= ~VW_DRAW_ELMTS;

        } else if (flag == EN) {
            memset (buf, 0, VW_ELMT_LEN);
            strcpy(buf, e->string);
            action->operation = AC_SEND_EN_STRING;
            action->str = buf;
            view_window->style &= ~VW_SHOW_PU;
            view_window->style &= ~VW_SHOW_PD;
            view_window->style |= VW_DRAW_ELMTS;
            view_window->clear_elements(view_window);
            view_window->update(view_window, hwnd, e);

            memset(stroke_window->str, 0, SW_STR_LEN);
            memset(stroke, 0, SW_STR_LEN);
            stroke_window->update(stroke_window, hwnd);
        }
    }
}

void proceed_move(HWND hwnd, key_board_t* key_board, BOOL pressed, POINT p, int flag)
{
    static md_key_t* key = NULL;
    static vw_element_t* e = NULL;

    key_board->action.operation = AC_NULL;

    if (!pressed)
        goto RETURN;

    if (PtInRect(&key_board->key_window->bound, p.x, p.y)) {
        key = key_board->key_window->get_key(key_board->key_window, p);
        if(key && key->style & KEY_PAD_FUNC)
            return ;
        if (_key_down != key) {
            if (_key_down != NULL && _key_down->scan_code != SCANCODE_LEFTSHIFT) {
               if(_key_down->style & KEY_PAD_DRAWED) {
                   _key_down->style &= ~KEY_PAD_DRAWED;
                   SetAutoRepeatMessage (0, 0, 0, 0);
                }
                _key_down->style &= ~KEY_PAD_PRESSED;
                _key_down->update(_key_down, hwnd);
            }

            if (key != NULL) {
                key->style |= KEY_PAD_PRESSED;
                key->update(key, hwnd);
            }
            _key_down = key;
        }
        return;
    }

    if (PtInRect(&key_board->view_window->bound, p.x, p.y)) {
        RECT rect;
        rect.left = key_board->view_window->key_pg_up.right;
        rect.top = key_board->view_window->bound.top;
        rect.right = key_board->view_window->key_pg_down.left;
        rect.bottom = key_board->view_window->bound.bottom;

        if (PtInRect(&rect, p.x, p.y)) {
            e = key_board->view_window->get_element(key_board->view_window, p);
            if (_e_down != e) {
                if (_e_down != NULL) {
                    key_board->view_window->style |= VW_DRAW_ELMTS;
                    key_board->view_window->style &= ~VW_PD_PRESSED;
                    key_board->view_window->update(key_board->view_window, hwnd, NULL);
                }

                if (e != NULL) {
                    vw_proceed_hit(hwnd, key_board->view_window, key_board->stroke_window,
                            &key_board->action, TRUE, p, flag, key_board->ime);
                }
                _e_down = e;
            }
        }
        return;
    }

RETURN:
    if (_e_down != NULL) {
        key_board->view_window->style |= VW_DRAW_ELMTS;
        key_board->view_window->style &= ~VW_PD_PRESSED;
        key_board->view_window->update(key_board->view_window, hwnd, NULL);
        _e_down = NULL;
    }

    if (_key_down != NULL) {
        _key_down->style &= ~KEY_PAD_PRESSED;
        _key_down->update(_key_down, hwnd);
        _key_down = NULL;
    }
}

void kw_proceed_hit(HWND hwnd, view_window_t* view_window,
        stroke_window_t* stroke_window, md_key_t* key, action_t* action,
        BOOL pressed, POINT p, int flag, md_ime_t* ime,WPARAM wParam,
        LPARAM lParam)
{

    if(pressed)
        _key_down = key;
    else
        _key_down = NULL;

    if (key->scan_code == SCANCODE_LEFTSHIFT) {
        action->operation = AC_NULL;
        if (!pressed)
            return;

        if (!(key->style & KEY_PAD_PRESSED)) {
            case_mode = PTI_CASE_ABC;
            key->style |= KEY_PAD_PRESSED;
            key->update(key, hwnd);
        } else {
            case_mode = PTI_CASE_abc;
            key->style &= ~KEY_PAD_PRESSED;
            key->update(key, hwnd);
        }
        return;
    }

    if (key->scan_code == SCANCODE_BACKSPACE) {
        if (pressed) {
            action->operation = AC_NULL;
            key->style |= KEY_PAD_PRESSED;

            if (!(key->style & KEY_PAD_DRAWED)) {
                key->update(key, hwnd);
                key->style |= KEY_PAD_DRAWED;
                SetAutoRepeatMessage (hwnd, MSG_LBUTTONDOWN, wParam, lParam);
            }

            if (strlen(stroke_window->str) > 0) {
                action->operation = AC_NULL;
                //TODO
                // 1. delete a char from stroke_window->str
                cut(stroke_window->str, flag);
                // 2. update stroke_window
                stroke_window->update(stroke_window, hwnd);
                // 3. look up words;
                strcpylower(stroke, stroke_window->str);
                if (flag == EN)
                    strcat(stroke, "*");
                sp = 0;
                index_curr = 0;
                if (ime_method)
                    index_next = ime_method(stroke, view_window->buffer,
                            view_window->max_str_len, index_curr, case_mode);
#if 0
                if(index_next = -1)
                    return;
#endif
                view_window->style = 0;
                if (view_window->set_elements)
                    view_window->set_elements(view_window, hwnd);
                view_window->style |= VW_DRAW_ELMTS;

                if (!try_next(ime_method, stroke,
                            view_window->max_str_len, index_next, case_mode)) {
                    view_window->style &= ~VW_SHOW_PD;
                }
                else {
                    view_window->style |= VW_SHOW_PD;
                    sp ++;
                }

                // 4. update view_window
                view_window->update (view_window, hwnd, NULL);
                view_window->style &= ~VW_DRAW_ELMTS;
                // 5. return
                return;
            } else {
                  //clear view window
                if(view_window->buffer[0]){
                    view_window->buffer[0] = '\0';
                    view_window->style = 0;
                    if (view_window->set_elements)
                        view_window->set_elements(view_window, hwnd);
                    view_window->update(view_window, hwnd, NULL);
                }
                else
                {
                    //send SCANCODE_BACKSPACE to application window
                    action->operation = AC_SEND_MSG;
                    action->message = MSG_KEYDOWN;
                    action->wParam = SCANCODE_BACKSPACE;
                    action->lParam = 0;
                }
            }

            return;
        } //end of pressed
        else {
            action->operation = AC_NULL;
            key->style &= ~KEY_PAD_PRESSED;
            if (key->style & KEY_PAD_DRAWED)
                key->style &= ~KEY_PAD_DRAWED;
            key->update(key, hwnd);
            SetAutoRepeatMessage (0, 0, 0, 0);
        }

        return;
    }

    if (pressed) {
        action->operation = AC_NULL;
        key->style |= KEY_PAD_PRESSED;
        key->update(key, hwnd);
        return;
    }

    if (key->scan_code == SCANCODE_SPACE) {
        static char buf[SW_STR_LEN] = {0};

        key->style &= ~KEY_PAD_PRESSED;
        key->update(key, hwnd);

        if (strlen(stroke_window->str) > 0) {
            if(view_window->element_num != 0)
                strcpy(buf, view_window->elements[0].string);
            else
                strcpy(buf, stroke_window->str);

            view_window->style = VW_DRAW_ELMTS;
            view_window->clear_elements(view_window);
            view_window->update (view_window, hwnd, NULL);

            memset(stroke_window->str, 0, SW_STR_LEN);
            memset(stroke, 0, SW_STR_LEN);
            stroke_window->update(stroke_window, hwnd);

            if (flag == EN)
                action->operation = AC_SEND_EN_STRING;
            else if (flag == CN)
                action->operation = AC_SEND_CN_STRING;
            action->str = buf;
            return;
        } else {
            //else send SCANCODE_SPACE to application window
            action->operation = AC_SEND_MSG;
            action->message = MSG_KEYDOWN;
            action->wParam = SCANCODE_SPACE;
            action->lParam = 0;
        }
        return;
    }

    if (key->scan_code == SCANCODE_ENTER) {
        static char buf[SW_STR_LEN];
        // send stroke_window->str to application
        key->style &= ~KEY_PAD_PRESSED;
        key->update(key, hwnd);
//        if(flag == EN && strlen(stroke_window->str) > 0) {
        if (strlen(stroke_window->str) > 0) {
            view_window->style = VW_DRAW_ELMTS;
            view_window->clear_elements(view_window);
            view_window->update (view_window, hwnd, NULL);

            strcpy(buf, stroke_window->str);
            memset(stroke_window->str, 0, SW_STR_LEN);
            memset(stroke, 0, SW_STR_LEN);
            stroke_window->update(stroke_window, hwnd);

            action->operation = AC_SEND_EN_STRING;
            action->str = buf;
        }
        else {
            action->operation = AC_SEND_MSG;
            action->message = MSG_KEYDOWN;
            action->wParam = SCANCODE_ENTER;
            action->lParam = 0;
        }
        return;
    }

    /*Switch ime status*/
    if (key->scan_code >= SCANCODE_TONUM && key->scan_code <= SCANCODE_TOOP) {
        key->style &= ~KEY_PAD_PRESSED;
        key->update(key, hwnd);
        action->operation = AC_CHANGE_KBD;
        return;
    }

    if (key->style & KEY_PAD_CHAR) {
        char ch;
        action->operation = AC_NULL;

        if (ime_method == ime->predict_word || ime_method == NULL) {
            memset(stroke_window->str, 0, SW_STR_LEN);
            ime_method = ime->translate_word;
        }

        key->style &= ~KEY_PAD_PRESSED;
        key->update(key, hwnd);

        if (case_mode == PTI_CASE_ABC) {
            ch = toupper(key->key_char);
/*            if (flag == CN) {// || *stroke_window->str) {
                append(stroke_window->str, ch, SW_STR_LEN - 1, flag);
                stroke_window->update(stroke_window, hwnd);

                view_window->style = 0;
                //            view_window->style |= VW_DRAW_ELMTS;
                view_window->clear_elements(view_window);
                view_window->update (view_window, hwnd, NULL);
                return;
            }
*/        } else
            ch = key->key_char;

        append(stroke_window->str, ch, SW_STR_LEN - 1, flag);
        stroke_window->update(stroke_window, hwnd);

        strcpylower(stroke, stroke_window->str);
        if (flag == EN)
            strcat(stroke, "*");
        // look up dict and update view window
        sp = 0;
        index_curr = 0;
        view_window->buffer[0] = '\0';
        if (ime_method)
            index_next = ime_method(stroke, view_window->buffer,
                    view_window->max_str_len, index_curr, case_mode);

        view_window->style = 0;
        view_window->style |= VW_DRAW_ELMTS;

        if(strlen(view_window->buffer) >= 0){
            view_window->set_elements(view_window, hwnd);
            if (index_next != -1) {
                if (!try_next(ime_method, stroke, view_window->max_str_len, index_next, case_mode))
                    view_window->style &= ~VW_SHOW_PD;
                else
                    view_window->style |= VW_SHOW_PD;
            }
        }
        else
            view_window->clear_elements(view_window);

        view_window->update(view_window, hwnd, NULL);
        view_window->style &= ~VW_DRAW_ELMTS;
        return;
    }
}

void keyboard_update(key_board_t *kb, HWND hWnd, WPARAM wParam, RECT* rect)
{
    HDC hdc;

    if (kb->data) {
#if 0
        hdc = GetDC (hWnd);
        if(rect != NULL) {
            RECT rcTemp = *rect;
            ScreenToClient(hWnd, &rcTemp.left, &rcTemp.top);
            ScreenToClient(hWnd, &rcTemp.right, &rcTemp.bottom);
            SelectClipRect(hdc, &rcTemp);
        }
        FillBoxWithBitmap(hdc, 0, 0, SKB_WIN_W, SKB_WIN_H, (PBITMAP)(kb->data));
        ReleaseDC(hdc);
#else
        BOOL fGetDC = FALSE;
        hdc = (HDC)wParam;
        if (!hdc){
            hdc = GetDC(hWnd);
            fGetDC = TRUE;
        }
        if (rect != NULL) {
            RECT rcTemp = *rect;
            ScreenToClient(hWnd, &rcTemp.left, &rcTemp.top);
            ScreenToClient(hWnd, &rcTemp.right, &rcTemp.bottom);
            SelectClipRect(hdc, &rcTemp);
        }
        FillBoxWithBitmap(hdc, 0, 0, SKB_WIN_W, SKB_WIN_H, (PBITMAP)(kb->data));
        if (fGetDC)
            ReleaseDC(hdc);
#endif
    }
}

static void symbol_kw_proceed_hit(HWND hwnd, view_window_t* view_window,
        stroke_window_t* stroke_window, md_key_t* key, action_t* action,
        BOOL pressed, POINT p, int flag, md_ime_t* ime, WPARAM wParam, LPARAM lParam)

{
    if (pressed) {
        /* add for BACKSPACE support  */
        if (key->scan_code ==  SCANCODE_BACKSPACE) {
            action->operation = AC_SEND_MSG;
            action->message = MSG_KEYDOWN;
            action->wParam = SCANCODE_BACKSPACE;
            action->lParam = 0;
            key->style |= KEY_PAD_PRESSED;
            if (!(key->style & KEY_PAD_DRAWED)) {
                key->update(key, hwnd);
                key->style |= KEY_PAD_DRAWED;
                SetAutoRepeatMessage(hwnd, MSG_LBUTTONDOWN, wParam, lParam);
            }
        }
        else {
            action->operation = AC_NULL;
            key->style |= KEY_PAD_PRESSED;
            key->update(key, hwnd);
        }

    } else {
        key->style &= ~KEY_PAD_PRESSED;
        key->update(key, hwnd);

        if (key->scan_code >= SCANCODE_TONUM && key->scan_code <= SCANCODE_TOOP) {
            action->operation = AC_CHANGE_KBD;
            return;
        }

        if (key->style & KEY_PAD_CHAR) {
            action->operation = AC_SEND_MSG;
            action->message = MSG_CHAR;
            action->wParam = key->key_char;
            action->lParam = 0;
        } else if (key->style & KEY_PAD_FUNC) {
            switch (key->scan_code) {
            case SCANCODE_TONUM: /* key "123" switch kbd*/
                action->operation = AC_CHANGE_KBD;
                break;
            case SCANCODE_TOOP:
                action->operation = AC_CHANGE_KBD;
                break;
            case SCANCODE_TOEN:
                action->operation = AC_CHANGE_KBD;
                break;
            case SCANCODE_TOPY:
                action->operation = AC_CHANGE_KBD;
                break;
            case SCANCODE_ENTER:
                action->operation = AC_SEND_MSG;
                action->message = MSG_KEYDOWN;
                action->wParam = SCANCODE_ENTER;
                action->lParam = 0;
                break;
            case SCANCODE_BACKSPACE:
                if(key->style & KEY_PAD_DRAWED)
                    key->style &= ~KEY_PAD_DRAWED;
                action->operation = AC_NULL;
                SetAutoRepeatMessage(0, 0, 0, 0);
                break;
            }
        }
    }
    return ;
}

int symbol_proc_msg (key_board_t* key_board, HWND hwnd,
        int message, WPARAM wParam, LPARAM lParam)
{
    static md_key_t *key_down;
    static int lbuttondown = 0;
    static POINT p;
    md_key_t *key;

    switch(message) {
    case MSG_LBUTTONDOWN: /* highlight key */
        p.x = LOSWORD(lParam);
        p.y = HISWORD(lParam);
        lbuttondown = 1;
        key_board->action.operation = AC_NULL;

        if (PtInRect(&key_board->key_window->bound, p.x, p.y)) {
            key = key_board->key_window->get_key(key_board->key_window, p);
            if (key == NULL) {
                key_board->action.operation = AC_NULL;
                break;
            }

            symbol_kw_proceed_hit(hwnd, key_board->view_window,
                    key_board->stroke_window, key, &key_board->action,
                    TRUE, p, EN, key_board->ime, wParam, lParam);
            key_down = key;
            break;
        }
        break;
    case MSG_LBUTTONUP: /* send key */
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

        if (PtInRect(&key_board->key_window->bound, p.x, p.y)) {
            key = key_board->key_window->get_key(key_board->key_window, p);
            if (key == NULL) {
                key_board->action.operation = AC_NULL;
                break;
            }

            symbol_kw_proceed_hit(hwnd, key_board->view_window,
                    key_board->stroke_window, key, &key_board->action,
                    FALSE, p, EN, key_board->ime, wParam, lParam);

            key_down = NULL;
            break;
        }
        break;

    case MSG_MOUSEMOVE :
        p.x = LOSWORD(lParam);
        p.y = HISWORD(lParam);
        if ((lbuttondown != 0) && (PtInRect(&key_board->key_window->bound, p.x, p.y))) {
            key = key_board->key_window->get_key
                (key_board->key_window, p);

            if (key != key_down){
                if(key_down != NULL) {
                    if(key_down->style & KEY_PAD_DRAWED) {
                        key_down->style &= ~KEY_PAD_DRAWED;
                        SetAutoRepeatMessage (0, 0, 0, 0);
                    }
                    key_down->style &= ~KEY_PAD_PRESSED;
                    key_down->update(key_down, hwnd);
#ifdef KBD_TOOLTIP
                    HideToolTip((HWND)((SOFTKBD_DATA *)GetWindowAdditionalData(hwnd))->tooltip_win);
#endif
                    key_board->action.operation = AC_NULL;
                }
                if(key != NULL) {
                    symbol_kw_proceed_hit(hwnd, key_board->view_window,
                            key_board->stroke_window, key, &key_board->action,
                            TRUE, p, EN, key_board->ime, wParam, lParam);
                }
                key_down = key;
            }
            else {
                key_board->action.operation = AC_NULL;
            }
            break;
        } else {
            key_board->action.operation = AC_NULL;
            break;
        }
        break;
    case MSG_NCMOUSEMOVE:
        if(key_down && HT_OUT == wParam) {
            key_board->action.operation = AC_NULL;
            key_down->style &= ~KEY_PAD_PRESSED;
            key_down->update(key_down, hwnd);
#ifdef KBD_TOOLTIP
            HideToolTip((HWND)((SOFTKBD_DATA *)GetWindowAdditionalData(hwnd))->tooltip_win);
#endif
            lbuttondown = 0;
            key_down = NULL;
        }
        return AC_NULL;
    }

    return key_board->action.operation;
}


/*************************************************/
/**  interfaces for processing input strings  **/
/*************************************************/
static int get_words_num(const char* buff)
{
    int n = 0;
    char *p = NULL;

    p = strchr (buff, ' ');
    while (p != NULL) {
        n++;
        p++;
        p = strchr(p, ' ');
    }
    return n;
}

//static char* get_word(const char* buff, int index, char *word)
static char* get_word(const char* buff, int index, vw_element_t* e)
{
    char *p = NULL;
    int n = 0;
    char* word = e->string;

    if (strlen(buff) <= 0)
        return NULL;

    p = strchr(buff, ' ');
    if (index == 0) {
        strncpy(word, buff, p - buff);
        word[p-buff] = '\0';
        e->index = (char*)buff;
        return word;
    }

    while (p != NULL) {
        n++;
        if (n == index) {
            char *t;
            p++;

            t = strchr(p, ' ');
            if (t == NULL) {
                strcpy(word, p);
                e->index = p;
            }
            else {
                strncpy(word, p, t-p);
                word[t-p] = '\0';
                e->index = p;
            }
            return word;
        }
        p++;
        p = strchr(p, ' ');
    }
    return NULL;
}

static BOOL get_substr_pos_ex(HDC hdc, const char* str, vw_element_t* e,
        RECT *rc, int* len, RECT* off)
{
    SIZE textsize, cur_size;
    char* substr = e->string;

    *len = strlen(substr);
    /* maybe string have same word, so need ensure by word length  */

    GetTextExtent(hdc, substr, strlen(substr), &cur_size);
    GetTextExtent(hdc, str, (e->index - str + 1), &textsize);

    SetRect(rc, textsize.cx + off->left, off->top + 1,
            off->left + textsize.cx + cur_size.cx + 1,
            off->bottom);

    return TRUE;
}

void vw_set_elements(struct _view_window_t* view_window, HWND hwnd)
{
    int i;
    HDC hdc = GetDC(hwnd);

    RECT r = view_window->bound;
    r.left = view_window->key_pg_up.right;
    view_window->element_num = get_words_num (view_window->buffer);

    if (view_window->element_num == 0)
        view_window->clear_elements(view_window);

    for (i=0; i<view_window->element_num; i++ ) {
        SelectFont(hdc, view_window->view_font);
        get_word(view_window->buffer, i, &view_window->elements[i]);
        get_substr_pos_ex(hdc, view_window->buffer, &view_window->elements[i],
            &view_window->elements[i].bound, &view_window->elements[i].len, &r);
#if 0
        printf("set element: view_window->elements[%d]: string: [%s] ",i, view_window->elements[i].string);
        int j;
        unsigned char ch;
        for(j=0; j<=strlen(view_window->elements[i].string);j++) {
            ch = (view_window->elements[i].string)[j];
            printf("%02x ", ch);
        }
        printf("   bound: %d %d %d %d\n",
                view_window->elements[i].bound.left,
                view_window->elements[i].bound.top,
                view_window->elements[i].bound.right,
                view_window->elements[i].bound.bottom);
#endif
    }

    ReleaseDC (hdc);
    return;
}

void vw_clear_elements(struct _view_window_t* view_window)
{
    memset(view_window->elements, 0, sizeof(view_window->elements));
    view_window->element_num = 0;
    return;
}

vw_element_t* vw_get_element(struct _view_window_t* view_window, POINT p)
{
    int i;
    for (i = 0; i<view_window->element_num; i++)
        if (PtInRect(&view_window->elements[i].bound, p.x, p.y))
            return &view_window->elements[i];
    return NULL;
}

void share_key_update(md_key_t *mk, HWND hWnd)
{
#ifdef KBD_TOOLTIP
    HWND ttw;
#endif
    HDC hdc ;
    PBITMAP pbmp ;

    if (!mk->data)
        return;

#ifdef KBD_TOOLTIP
    ttw = (HWND)((SOFTKBD_DATA *)GetWindowAdditionalData(hWnd))->tooltip_win;
#endif
    if (mk->style & KEY_PAD_PRESSED) {
        if (mk->style & KEY_PAD_CHAR) {
#ifdef KBD_TOOLTIP
            char key_ch;
            int x, y;

            x = mk->bound.left + RECTW(mk->bound)/2 - TTW_W/2;
            y = mk->bound.top + RECTH(mk->bound)/2 - TTW_H;
            if (islower(mk->key_char) && case_mode == PTI_CASE_ABC)
                key_ch = toupper(mk->key_char);
            else
                key_ch = mk->key_char;
            ClientToScreen(hWnd, &x, &y);
            ShowToolTip(ttw, x , y, key_ch);
#endif
        } else {
            hdc = GetDC(hWnd);
            pbmp = (PBITMAP)mk->data;
            FillBoxWithBitmap(hdc, mk->bound.left, mk->bound.top,
                    RECTW(mk->bound), RECTH(mk->bound), pbmp);
            ReleaseDC(hdc);
        }
    } else {
        //HideToolTip(ttw);
        EraseBbGround(hWnd, &mk->bound);
    }
#if 0
    if (mk->style & KEY_PAD_PRESSED) {
        hdc = GetDC(hWnd);
        pbmp = (PBITMAP)mk->data;
        FillBoxWithBitmap(hdc, mk->bound.left, mk->bound.top, RECTW(mk->bound), RECTH(mk->bound), pbmp);
        ReleaseDC(hdc);
    } else{
        EraseBbGround(hWnd, &mk->bound);
    }
#endif
}

void clear_keyboard(key_board_t *kb)
{
    if(kb == NULL)
        return ;

    memset(&kb->action, 0 , sizeof(action_t));

    if(kb->key_window)
    {
        int i;
        md_key_t* key;
        key = kb->key_window->key;
        for(i=0; i<kb->key_window->key_num; i++){
            key[i].style &= (~(KEY_PAD_PRESSED|KEY_PAD_DRAWED));
        }
    }

    if(kb->view_window){
        kb->view_window->buffer[0] = '\0';
        kb->view_window->style = 0; //clear style
        if(kb->view_window->clear_elements)
            kb->view_window->clear_elements(kb->view_window);
    }
    if(kb->stroke_window){
        kb->stroke_window->str[0] = '\0';
    }
}



