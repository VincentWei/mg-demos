/*!============================================================================
 * @file SoftKeyboard.c 
 * @Synopsis Soft keyboard
 * @author Vincent Wei
 * @version 1.0
 *
 * Copyright (C) 2018 Beijing Joobot Technologies Inc.
 * All rights reserved.
 */

#ifndef _GNU_SOURCE
#   define _GNU_SOURCE
#endif

#include "global.h"
#include "utilities.h"
#include "misc.h"

#include "resource.h"

#define PAGE_DIGITS     0
#define PAGE_SYMBOLS    1

#define KEY_TYPE_CHARS      0
#define KEY_TYPE_CAPLOCK    1
#define KEY_TYPE_SYMBOLS    2
#define KEY_TYPE_BACK       3
#define KEY_TYPE_NULL       4

#define IDC_INPUTBOX        11
#define IDC_DELETE          12

#define IDC_CANDIDATE_MIN   21
#define IDC_CANDIDATE_0     (IDC_CANDIDATE_MIN + 0)
#define IDC_CANDIDATE_1     (IDC_CANDIDATE_MIN + 1)
#define IDC_CANDIDATE_2     (IDC_CANDIDATE_MIN + 2)
#define IDC_CANDIDATE_3     (IDC_CANDIDATE_MIN + 3)
#define IDC_CANDIDATE_4     (IDC_CANDIDATE_MIN + 4)
#define IDC_CANDIDATE_MAX   (IDC_CANDIDATE_4)

#define IDC_PROPSHEET       31

#define IDC_DIGIT_MIN       100
#define IDC_DIGIT_MAX       109

#define IDC_SYMBOL_MIN      201
#define IDC_SYMBOL_MAX      209

#define IDC_CAPSLOCK        301
#define IDC_GO_SYMBOLS      302
#define IDC_RETURN_DIGITS   303

#define KEY_W               55
#define KEY_H               46
#define BACK_KEY_H          146
#define MARGIN              4

typedef struct _KEY {
    int type;
    int id;
    const char* res_name;
    const char* chars;
} KEY;

typedef struct _SK_INFO {
    int caps_lock;
    char *caller_buff;
    char *buff;
    int len_min;
    int len_max;
} SK_INFO;

static KEY _digit_keys [] = {
    {KEY_TYPE_CHARS,    IDC_DIGIT_MIN + 0,  "images/soft-keyboard/digit-0.png", "0!@#."},
    {KEY_TYPE_CHARS,    IDC_DIGIT_MIN + 1,  "images/soft-keyboard/digit-1.png", "1$%&"},
    {KEY_TYPE_CHARS,    IDC_DIGIT_MIN + 2,  "images/soft-keyboard/digit-2.png", "2abc"},
    {KEY_TYPE_CHARS,    IDC_DIGIT_MIN + 3,  "images/soft-keyboard/digit-3.png", "3def"},
    {KEY_TYPE_CAPLOCK,  IDC_CAPSLOCK,       "images/soft-keyboard/caps-off.png", ""},
    {KEY_TYPE_CHARS,    IDC_DIGIT_MIN + 4,  "images/soft-keyboard/digit-4.png", "4ghi"},
    {KEY_TYPE_CHARS,    IDC_DIGIT_MIN + 5,  "images/soft-keyboard/digit-5.png", "5jkl"},
    {KEY_TYPE_CHARS,    IDC_DIGIT_MIN + 6,  "images/soft-keyboard/digit-6.png", "6mno"},
    {KEY_TYPE_SYMBOLS,  IDC_GO_SYMBOLS,     "images/soft-keyboard/go-symbols.png", ""},
    {KEY_TYPE_CHARS,    IDC_DIGIT_MIN + 7,  "images/soft-keyboard/digit-7.png", "7pqrs"},
    {KEY_TYPE_CHARS,    IDC_DIGIT_MIN + 8,  "images/soft-keyboard/digit-8.png", "8tuv"},
    {KEY_TYPE_CHARS,    IDC_DIGIT_MIN + 9,  "images/soft-keyboard/digit-9.png", "9wxyz"},
    {KEY_TYPE_NULL,     0,                  "images/soft-keyboard/caps-on.png", NULL},
    {KEY_TYPE_NULL,     0,                  "images/soft-keyboard/delete.png", NULL},
    {KEY_TYPE_NULL,     0,                  "images/soft-keyboard/done.png", NULL},
};

static KEY _symbol_keys [] = {
    {KEY_TYPE_BACK,     IDC_RETURN_DIGITS,  "images/soft-keyboard/return-digits.png", ""},
    {KEY_TYPE_CHARS,    IDC_SYMBOL_MIN + 0, "images/soft-keyboard/symbol-1.png", "~`!@"},
    {KEY_TYPE_CHARS,    IDC_SYMBOL_MIN + 1, "images/soft-keyboard/symbol-2.png", "#$%^"},
    {KEY_TYPE_CHARS,    IDC_SYMBOL_MIN + 2, "images/soft-keyboard/symbol-3.png", "&*()"},
    {KEY_TYPE_CHARS,    IDC_SYMBOL_MIN + 3, "images/soft-keyboard/symbol-4.png", "-+_="},
    {KEY_TYPE_CHARS,    IDC_SYMBOL_MIN + 4, "images/soft-keyboard/symbol-5.png", "{}[]"},
    {KEY_TYPE_CHARS,    IDC_SYMBOL_MIN + 5, "images/soft-keyboard/symbol-6.png", "\\/?|"},
    {KEY_TYPE_CHARS,    IDC_SYMBOL_MIN + 6, "images/soft-keyboard/symbol-7.png", "<>,."},
    {KEY_TYPE_CHARS,    IDC_SYMBOL_MIN + 7, "images/soft-keyboard/symbol-8.png", "'\":;"},
    {KEY_TYPE_CHARS,    IDC_SYMBOL_MIN + 8, "images/soft-keyboard/symbol-9.png", " "},
};

static void toggle_candidates (HWND hwnd_mainwin, BOOL upper)
{
    int cand_id;
    char cand_str [2] = {'\0', '\0'};
    char new_char;

    for (cand_id = IDC_CANDIDATE_MIN; cand_id <= IDC_CANDIDATE_MAX; cand_id++) {
        HWND cand_hwnd = GetDlgItem (hwnd_mainwin, cand_id);
        GetWindowText (cand_hwnd, cand_str, 1);

        if (upper) {
            new_char = toupper (cand_str [0]);
        }
        else {
            new_char = tolower (cand_str [0]);
        }

        if (new_char != cand_str [0]) {
            cand_str [0] = new_char;
            SetWindowText (cand_hwnd, cand_str);
        }
    }
}

static BOOL my_onCommand (mWidget* self, int id, int nc, HWND hCtrl)
{
    HWND hwnd_mainwin = GetMainWindowHandle (hCtrl);
    mSlEdit *edit = (mSlEdit *)ncsGetChildObj (hwnd_mainwin, IDC_INPUTBOX);
    mPropSheet *propsheet = (mPropSheet*)ncsGetChildObj (hwnd_mainwin, IDC_PROPSHEET);
    SK_INFO* sk_info = (SK_INFO*) GetWindowAdditionalData (hwnd_mainwin);

    if (nc == NCSN_WIDGET_CLICKED) {
        if (id >= IDC_DIGIT_MIN && id <= IDC_DIGIT_MAX) {
            const char* chars = GetWindowCaption (hCtrl);
            int nr_chars = strlen (chars);

            int cand_id;
            char cand_str [2] = {'\0', '\0'};
            for (cand_id = IDC_CANDIDATE_MIN; cand_id <= IDC_CANDIDATE_MAX; cand_id++) {
                HWND cand_hwnd = GetDlgItem (hwnd_mainwin, cand_id);
                int idx = cand_id - IDC_CANDIDATE_MIN;

                if (idx < nr_chars) {
                    if (sk_info->caps_lock)
                        cand_str [0] = toupper (chars[idx]);
                    else
                        cand_str [0] = tolower (chars[idx]);
                }
                else {
                    cand_str [0] = '\0';
                }

                SetWindowText (cand_hwnd, cand_str);
            }
        }
        else if (id >= IDC_SYMBOL_MIN && id <= IDC_SYMBOL_MAX) {
            const char* chars = GetWindowCaption (hCtrl);
            int nr_chars = strlen (chars);

            if (nr_chars == 1) {
                _c(edit)->append (edit, chars, 0, -1);
            }
            else {
                int cand_id;
                char cand_str [2] = {'\0', '\0'};
                for (cand_id = IDC_CANDIDATE_MIN; cand_id <= IDC_CANDIDATE_MAX; cand_id++) {
                    HWND cand_hwnd = GetDlgItem (hwnd_mainwin, cand_id);
                    int idx = cand_id - IDC_CANDIDATE_MIN;

                    if (idx < nr_chars) {
                        cand_str [0] = chars[idx];
                    }
                    else {
                        cand_str [0] = '\0';
                    }

                    SetWindowText (cand_hwnd, cand_str);
                }
            }
        }
        else if (id >= IDC_CANDIDATE_MIN && id <= IDC_CANDIDATE_MAX) {
            const char* chars = GetWindowCaption (hCtrl);
            if (chars [0])
                _c(edit)->append (edit, chars, 0, -1);
        }
        else if (id == IDOK) {
            SendNotifyMessage (self->hwnd, MSG_USER, 0, 0);
        }
        else if (id == IDC_DELETE) {
            SendMessage (edit->hwnd, MSG_KEYDOWN, SCANCODE_END, 0);
            SendMessage (edit->hwnd, MSG_KEYDOWN, SCANCODE_BACKSPACE, 0);
        }
        else if (id == IDC_CAPSLOCK) {
            if (sk_info->caps_lock) {
                mStatic *caps_lock = (mStatic*)ncsObjFromHandle (hCtrl);
                _c(caps_lock)->setProperty (caps_lock, NCSP_IMAGE_IMAGE, 
                    (DWORD)GetResource (Str2Key ("images/soft-keyboard/caps-off.png")));

                toggle_candidates (hwnd_mainwin, FALSE);

                sk_info->caps_lock = 0;
            }
            else {
                mStatic *caps_lock = (mStatic*)ncsObjFromHandle (hCtrl);
                _c(caps_lock)->setProperty (caps_lock, NCSP_IMAGE_IMAGE, 
                    (DWORD)GetResource (Str2Key ("images/soft-keyboard/caps-on.png")));

                toggle_candidates (hwnd_mainwin, TRUE);

                sk_info->caps_lock = 1;
            }
        }
        else if (id == IDC_GO_SYMBOLS) {
            _c(propsheet)->setProperty (propsheet, NCSP_PRPSHT_ACTIVEPAGEIDX, 1);
        }
        else if (id == IDC_RETURN_DIGITS) {
            _c(propsheet)->setProperty (propsheet, NCSP_PRPSHT_ACTIVEPAGEIDX, 0);
        }
    }

    return TRUE;
}

static void mypage_onInitPage (mWidget* self, DWORD add_data)
{
    unsigned u;

    NCS_PROP_ENTRY props [] = {
        { NCSP_IMAGE_IMAGE, 0 },
        { 0, 0 }
    };

    NCS_WND_TEMPLATE ctrl_templ = {
        class_name: NCSCTRL_IMAGE,
        style: WS_VISIBLE | NCSS_NOTIFY,
        ex_style: WS_EX_NONE,
        props: props,
        w: KEY_W,
        h: KEY_H,
    };

    if (add_data == PAGE_DIGITS) {
        for (u = 0; u < TABLESIZE (_digit_keys); u++) {
            if (_digit_keys[u].type == KEY_TYPE_NULL) {
                continue;
            }

            props [0].value = (DWORD)GetResource (Str2Key (_digit_keys[u].res_name));

            ctrl_templ.id = _digit_keys[u].id;
            ctrl_templ.caption = _digit_keys[u].chars;
            ctrl_templ.x = MARGIN + (KEY_W + MARGIN) * (u % 4);
            ctrl_templ.y = MARGIN + (KEY_H + MARGIN) * (u / 4);

            ncsCreateWindowIndirect (&ctrl_templ, self->hwnd);
        }
    }
    else if (add_data == PAGE_SYMBOLS) {
        props [0].value = (DWORD)GetResource (Str2Key (_symbol_keys[0].res_name));
        ctrl_templ.id = _symbol_keys[0].id;
        ctrl_templ.caption = _symbol_keys[0].chars;
        ctrl_templ.x = MARGIN;
        ctrl_templ.y = MARGIN;
        ctrl_templ.h = BACK_KEY_H;

        ncsCreateWindowIndirect (&ctrl_templ, self->hwnd);

        ctrl_templ.h = KEY_H;
        for (u = 1; u < TABLESIZE (_symbol_keys); u++) {
            if (_symbol_keys[u].type == KEY_TYPE_NULL) {
                continue;
            }

            props [0].value = (DWORD)GetResource (Str2Key (_symbol_keys[u].res_name));

            ctrl_templ.id = _symbol_keys[u].id;
            ctrl_templ.caption = _symbol_keys[u].chars;
            ctrl_templ.x = MARGIN + (KEY_W + MARGIN) * ((u - 1) % 3 + 1);
            ctrl_templ.y = MARGIN + (KEY_H + MARGIN) * ((u - 1) / 3);

            ncsCreateWindowIndirect (&ctrl_templ, self->hwnd);
        }
    }

    return;
}

static int mypage_onShowPage (mWidget* self, HWND hwnd, int show_cmd)
{
    return 1;
}

static int mypage_onSheetCmd (mWidget* self, DWORD wParam, DWORD lParam)
{
    if (wParam == IDOK)
        return 1;
    return 0;
}

static NCS_EVENT_HANDLER mypage_handlers[] = {
    { MSG_INITPAGE, mypage_onInitPage },
    { MSG_SHOWPAGE, mypage_onShowPage },
    { MSG_COMMAND, my_onCommand },
    { MSG_SHEETCMD, mypage_onSheetCmd },
    { 0 , NULL }
};

static BOOL init_propsheet (mComponent* self)
{
    DLGTEMPLATE page_info = {
        WS_NONE,
        WS_EX_NONE,
        0, 0, 0, 0,
        "",
        0, 0,
        0, NULL,
        0
    };

    mPropSheet *propsheet = (mPropSheet*)(_c(self)->getChild (self, IDC_PROPSHEET));
    if (propsheet == NULL) {
        _ERR_PRINTF ("SoftKeyboard: failed to get property sheet control.\n");
        return FALSE;
    }

    mPage* page;

    page_info.caption = "Digits";
    page_info.dwAddData = PAGE_DIGITS;
    page = _c(propsheet)->addPage (propsheet, &page_info, mypage_handlers);
    SetWindowBkColor (((mWidget*)page)->hwnd, PIXEL_black);

    page_info.caption = "Symbols";
    page_info.dwAddData = PAGE_SYMBOLS;
    page = _c(propsheet)->addPage (propsheet, &page_info, mypage_handlers);
    SetWindowBkColor (((mWidget*)page)->hwnd, PIXEL_black);

    return TRUE;
}

static BOOL load_res (void)
{
    unsigned u;
    RES_NODE res_list [] = {
        { NULL, RES_TYPE_IMAGE, 16, 0},
        { NULL},
    };

    for (u = 0; u < TABLESIZE (_digit_keys); u++) {
        res_list [0].res_name = _digit_keys[u].res_name;
        if (loadResByTag (res_list, 0) == 0) {
            _ERR_PRINTF ("SoftKeyboard: failed to load resource.\n");
            return FALSE;
        }
    }

    for (u = 0; u < TABLESIZE (_symbol_keys); u++) {
        res_list [0].res_name = _symbol_keys[u].res_name;
        if (loadResByTag (res_list, 0) == 0) {
            _ERR_PRINTF ("SoftKeyboard: failed to load resource.\n");
            return FALSE;
        }
    }

    return TRUE;
}

static BOOL init_others (mComponent* self)
{
    int id;

    mWidget* ctrl_ok = (mWidget*)(_c(self)->getChild (self, IDOK));
    _c(ctrl_ok)->setProperty (ctrl_ok, NCSP_IMAGE_IMAGE, 
            (DWORD)GetResource (Str2Key ("images/soft-keyboard/done.png")));

    mWidget* ctrl_delete = (mWidget*)(_c(self)->getChild (self, IDC_DELETE));
    _c(ctrl_delete)->setProperty (ctrl_delete, NCSP_IMAGE_IMAGE, 
            (DWORD)GetResource (Str2Key ("images/soft-keyboard/delete.png")));

    /* set the background color of candidate control to #aaaaaa */
    for (id = IDC_CANDIDATE_MIN; id <= IDC_CANDIDATE_MAX; id++) {
        mWidget* ctrl = (mWidget*)(_c(self)->getChild (self, id));
        ncsSetElement (ctrl, NCS_BGC_WINDOW, 0xFFAAAAAA);
        ncsSetElement (ctrl, NCS_FGC_WINDOW, 0xFF000000);
        ncsSetFont (ctrl->hwnd, GLOBAL_FONT_XL);
        SetWindowBkColor (((mWidget*)ctrl)->hwnd, RGB2Pixel(HDC_SCREEN, 0xAA, 0xAA, 0xAA));
    }

    return TRUE;
}

static BOOL _sk_onCreated (struct _NCS_CREATE_NOTIFY_INFO* info, mComponent* self, DWORD special_id)
{

    if (!load_res ())
        return FALSE;

    if (!init_propsheet (self))
        return FALSE;

    if (!init_others (self))
        return FALSE;

    return TRUE;
}

static BOOL _sk_onCreate (mMainWnd* self, DWORD add_data)
{
    return TRUE;
}

static void _sk_onTimer (mMainWnd *self, int id, DWORD count)
{
}

static BOOL _sk_onKeyDown (mMainWnd* self, int message, int code, DWORD key_status)
{
    if (code == SCANCODE_ESCAPE) {
        SK_INFO* sk_info = (SK_INFO*) GetWindowAdditionalData (self->hwnd);
        sk_info->buff [0] = '\0';
        SendNotifyMessage (self->hwnd, MSG_CLOSE, 0, 0);
    }

    return FALSE;
}

static LRESULT _sk_onDone (mMainWnd* self, LINT message, WPARAM wparam, LPARAM lparam)
{
    SK_INFO* sk_info = (SK_INFO*) GetWindowAdditionalData (self->hwnd);
    mSlEdit *edit = (mSlEdit *)ncsGetChildObj (self->hwnd, IDC_INPUTBOX);
    _c(edit)->getContent (edit, sk_info->buff, sk_info->len_max, 0, -1);

    if (sk_info->len_min > 0 && strlen (sk_info->buff) < sk_info->len_min) {
        char* text;
        int ret = asprintf (&text, _("You need to enter at least %d letters, numbers, or characters."), sk_info->len_min);
        if (ret < 0) {
            _ERR_PRINTF ("SoftKeyboard: failed to call asprintf.\n");
        }
        else {
            messageScreen (self->hwnd, _("Input Error"), text, MB_OK);
            free (text);
        }
    }
    else {
        strcpy(sk_info->caller_buff, sk_info->buff);
        SendNotifyMessage (self->hwnd, MSG_CLOSE, 0, 0);
    }

    return 0;
}

static BOOL _sk_onDestroy (mMainWnd* self, int message)
{
    unsigned u;
    RES_NODE res_list [] = {
        { NULL, RES_TYPE_IMAGE, 16, 0},
        { NULL},
    };

    for (u = 0; u < TABLESIZE (_digit_keys); u++) {
        res_list [0].res_name = _digit_keys[u].res_name;
        unloadResByTag (res_list, 0);
    }

    for (u = 0; u < TABLESIZE (_symbol_keys); u++) {
        res_list [0].res_name = _symbol_keys[u].res_name;
        unloadResByTag (res_list, 0);
    }

    _MG_PRINTF ("softKeyboard: _sk_onDestroy called\n");
    return TRUE;
}

static NCS_CREATE_NOTIFY_INFO _sk_create_notify_info = {
    onCreated: _sk_onCreated,
};

static NCS_EVENT_HANDLER _sk_handlers[] = {
    { MSG_CREATE, _sk_onCreate },
    { MSG_TIMER, _sk_onTimer },
    { MSG_KEYDOWN, _sk_onKeyDown },
    { MSG_COMMAND, my_onCommand },
    { MSG_USER, _sk_onDone },
    { MSG_DESTROY, _sk_onDestroy },
    { 0, NULL }
};

static NCS_WND_TEMPLATE _sk_ctrl_templ[] =
{
    {
        class_name: NCSCTRL_IMAGE,
        id: IDOK,
        x: 0,
        y: 0,
        w: 40,
        h: 40,
        style: WS_VISIBLE | NCSS_NOTIFY,
        ex_style: WS_EX_NONE,
        caption: "",
    },
    {
        class_name: NCSCTRL_SLEDIT,
        id: IDC_INPUTBOX,
        x: 45,
        y: 0,
        w: SCREEN_W - 90,
        h: 40,
        style: WS_VISIBLE | WS_DISABLED,
        ex_style: WS_EX_NONE,
        caption: "",
    },
    {
        class_name: NCSCTRL_IMAGE,
        id: IDC_DELETE,
        x: SCREEN_W - 40,
        y: 0,
        w: 40,
        h: 40,
        style: WS_VISIBLE | NCSS_NOTIFY,
        ex_style: WS_EX_NONE,
        caption: "",
    },
    {
        class_name: NCSCTRL_STATIC,
        id: IDC_CANDIDATE_0,
        x: 0,
        y: 40,
        w: 48,
        h: 46,
        style: WS_VISIBLE | NCSS_NOTIFY,
        ex_style: WS_EX_NONE,
        caption: "",
    },
    {
        class_name: NCSCTRL_STATIC,
        id: IDC_CANDIDATE_1,
        x: 0 + 48,
        y: 40,
        w: 48,
        h: 46,
        style: WS_VISIBLE | NCSS_NOTIFY,
        ex_style: WS_EX_NONE,
        caption: "",
    },
    {
        class_name: NCSCTRL_STATIC,
        id: IDC_CANDIDATE_2,
        x: 0 + 48 + 48,
        y: 40,
        w: 48,
        h: 46,
        style: WS_VISIBLE | NCSS_NOTIFY,
        ex_style: WS_EX_NONE,
        caption: "",
    },
    {
        class_name: NCSCTRL_STATIC,
        id: IDC_CANDIDATE_3,
        x: 0 + 48 + 48 + 48,
        y: 40,
        w: 48,
        h: 46,
        style: WS_VISIBLE | NCSS_NOTIFY,
        ex_style: WS_EX_NONE,
        caption: "",
    },
    {
        class_name: NCSCTRL_STATIC,
        id: IDC_CANDIDATE_4,
        x: 0 + 48 + 48 + 48 + 48,
        y: 40,
        w: 48,
        h: 46,
        style: WS_VISIBLE | NCSS_NOTIFY,
        ex_style: WS_EX_NONE,
        caption: "",
    },
    {
        class_name: NCSCTRL_PROPSHEET,
        id: IDC_PROPSHEET,
        x: 0,
        y: 40 + 46,
        w: SCREEN_W + 4,
        h: SCREEN_H,
        style: WS_VISIBLE | NCSS_PRPSHT_BOTTOM,
        ex_style: WS_EX_NONE,
        caption: "",
    },
};

//define the main window template
static NCS_MNWND_TEMPLATE _sk_templ = {
    NCSCTRL_DIALOGBOX, 
    1,
    0, 0, SCREEN_W, SCREEN_H,
    WS_NONE, WS_EX_AUTOSECONDARYDC,
    "Soft Keyboard",
    NULL,
    NULL,
    _sk_handlers,
    _sk_ctrl_templ,
    TABLESIZE(_sk_ctrl_templ),
    0, 0, NULL, 0, 0,
    &_sk_create_notify_info,
    0
};

void softKeyboard (HWND hosting, char* buff, int len_min, int len_max)
{
    SK_INFO sk_info = {0, buff, NULL, len_min, len_max};

    sk_info.buff = calloc(len_max + 1, sizeof(char));
    strcpy(sk_info.buff, sk_info.caller_buff);

    _sk_ctrl_templ[1].caption = buff;
    _sk_templ.user_data = (DWORD)&sk_info;

    mDialogBox* mydlg = (mDialogBox *)ncsCreateMainWindowIndirect (&_sk_templ, hosting);
    _c(mydlg)->doModal (mydlg, TRUE);

    free (sk_info.buff);
}

