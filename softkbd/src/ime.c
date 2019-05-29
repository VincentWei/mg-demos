/*
** $Id: ime.c 312 2008-07-16 03:26:04Z xkwu $
**
** taskbar.c:  Feynman PMP taskbar control.
**
** Copyright (C) 2006 FMSoft.
**
** All right reserved by FMSoft.
**
** Current maintainer: Liu Peng.
**
** Create date: 2006/11/17
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>

#include "ime.h"
#include "ime-pti-en/mgpti.h"
#include "ime-pinyin-zh/ime_pinyin.h"

int cb_pti_init(void* filename)
{
    return ptiInitialize (filename);
}

int cb_pti_match_keystrokes(const char *strokes, char *buffer, int buffer_len, int index, int mode)
{
    return ptiMatchKeystokes(strokes, buffer, buffer_len, index, mode);
}

int cb_pinyin_translate_word(const char *strokes, char *buffer, int buffer_len, int index, int mode)
{
    return pinyin_translate_word (NULL, strokes, buffer, buffer_len, index);
}

int cb_pinyin_predict_word(const char *strokes, char *buffer, int buffer_len, int index, int mode)
{
    return pinyin_predict_pord(NULL, strokes, buffer, buffer_len, index);
}

int cb_pinyin_init(void* filename)
{
    return ime_pinyin_init();
}
