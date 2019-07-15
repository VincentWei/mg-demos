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

#ifndef __IME_PINYIN_H__
#define __IME_PINYIN_H__

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

int pinyin_match_keystokes(void *method, const char* keystokes,
        char *buffer, int buffer_len, int cursor);
int pinyin_translate_word(void *method, const char *strokes,
        char *buffer, int buffer_len, int index);
int pinyin_predict_word(void *method, const char *lookfor,
        char *buffer, int buffer_len, int index);
BOOL ime_pinyin_init(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __IME_PINYIN_H__ */
