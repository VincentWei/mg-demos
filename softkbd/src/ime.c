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
    return pinyin_predict_word(NULL, strokes, buffer, buffer_len, index);
}

int cb_pinyin_init(void* filename)
{
    return ime_pinyin_init();
}
