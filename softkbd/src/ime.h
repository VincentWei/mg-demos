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

#ifndef __IME_H
#define __IME_H

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

#define NR_INPUT_METHOD_MAX    2

#define PTI_CASE_ABC                    0x01
#define PTI_CASE_Abc                    0x02
#define PTI_CASE_abc                    0x03

int cb_pti_init(void* filename);
int cb_pti_match_keystrokes(const char *strokes, char *buffer, int buffer_len, int index, int mode);

int cb_pinyin_translate_word(const char *strokes, char *buffer, int buffer_len, int index, int mode);

int cb_pinyin_predict_word(const char *strokes, char *buffer, int buffer_len, int index, int mode);

int cb_pinyin_init(void* filename);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  /* __IME_H */

