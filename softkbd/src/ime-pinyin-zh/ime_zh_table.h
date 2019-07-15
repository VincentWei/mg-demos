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

#ifndef __IME_ZH_TABLE_H__
#define __IME_ZH_TABLE_H__

typedef struct _IME_ZH_ITEM {
    unsigned int key1;
    unsigned int key2;
    unsigned short ch;
    unsigned short freq;
} IME_ZH_ITEM;

typedef struct {
    const char* ename;
    const char* encoding;

    unsigned char  key_map[128];
    unsigned char  key_name[64];
    unsigned short key_indices[64];

    int nr_keys;
    int nr_chars;
    int nr_words;
    int nr_associated;

    const IME_ZH_ITEM *items;
    const unsigned int *word_offsets;
    const unsigned char* words;
    const unsigned short* associated;

    // runtime fields
    int associate_mode;
    int max_dup_sel;
} IME_ZH_TABLE;

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* __IME_ZH_TABLE_H__ */
