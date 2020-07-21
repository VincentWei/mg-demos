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
#include "ime-rime/ime_rime.h"

int cb_pti_init(void *filename)
{
    return ptiInitialize(filename);
}

int cb_pti_match_keystrokes(const char *strokes, char *buffer, int buffer_len, int index, int mode)
{
    return ptiMatchKeystokes(strokes, buffer, buffer_len, index, mode);
}

int cb_rime_match_keystrokes(const char *strokes, char *buffer, int buffer_len, int index, int mode)
{
    return RimeMatchKeystokes(strokes, buffer, buffer_len, index);
}

int cb_rime_translate_word(const char *strokes, char *buffer, int buffer_len, int index, int mode)
{
    return rime_translate_word(NULL, strokes, buffer, buffer_len, index);
}

int cb_rime_predict_word(const char *strokes, char *buffer, int buffer_len, int index, int mode)
{
    return rime_predict_word(NULL, strokes, buffer, buffer_len, index);
}

int cb_rime_init(void *filename)
{
    return ime_rime_init();
}
