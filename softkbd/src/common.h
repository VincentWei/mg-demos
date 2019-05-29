
#ifndef __COMMON_H__
#define __COMMON_H__

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include "softkeyboard.h"

#define CN 1
#define EN 2
#define PTI_CASE_ABC                    0x01
#define PTI_CASE_Abc                    0x02
#define PTI_CASE_abc                    0x03

#define VIEWWIN_DATA_PTR(vw) ((vw_add_data_t*)(vw->data))
#define KEYWIN_DATA_PTR(kw)  ((kw_add_data_t*)(kw->data))

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

void softkey_reset (void);
int get_pti_case_mode (void);

BOOL try_next(ime_callback func, const char* strokes, int len, int index,
        int mode);

void vw_proceed_hit(HWND hwnd, view_window_t* view_window,
        stroke_window_t* stroke_window, action_t* action,
        BOOL pressed, POINT p, int flag, md_ime_t* ime);

void vw_set_elements (struct _view_window_t* view_window, HWND hwnd);
void vw_clear_elements (struct _view_window_t* view_window);
        vw_element_t* vw_get_element (struct _view_window_t* view_window, POINT p);

void kw_proceed_hit(HWND hwnd, view_window_t* view_window,
        stroke_window_t* stroke_window, md_key_t* key, action_t* action,
        BOOL pressed, POINT p, int flag, md_ime_t* ime,WPARAM wParam,
        LPARAM lParam);

//void keyboard_update(key_board_t *kb, HWND hWnd, RECT* rect);
void keyboard_update(key_board_t *kb, HWND hWnd, WPARAM wParam, RECT* rect);

int symbol_proc_msg (key_board_t* key_board, HWND hwnd,
        int message, WPARAM wParam, LPARAM lParam);

void share_key_update(md_key_t *mk, HWND hWnd);
void EraseBbGround(HWND hWnd, RECT* rc);

void clear_keyboard(key_board_t *kb);
void reset_mouse_state(HWND hwnd);
void proceed_move(HWND hwnd, key_board_t* key_board, BOOL pressed, POINT p, int flag);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif // __COMMON_H__
