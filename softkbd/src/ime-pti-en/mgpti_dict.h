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
** The header of dictionary for predictive text input.
**
** This file is created by 'words2c' by FMSoft (http://www.fmsoft.cn).
** Please do not modify it manually.
*/

#ifndef __PTI_DICT_H
    #define __PTI_DICT_H

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

#define MGPTI_2ND_HASH_TABLE_SIZE   100

typedef struct _MGPTI_2ND_HASH_BUCKET {
    int nr_words;
    int* word_indices;
} MGPTI_2ND_HASH_BUCKET;

typedef struct _MGPTI_1ST_HASH_BUCKET {
    int indices [26];
    MGPTI_2ND_HASH_BUCKET* buckets;
} MGPTI_1ST_HASH_BUCKET;

typedef struct _MGPTI_DICTIONARY {
    char** sorted_words;
    int max_index;
    int max_len_word;
    MGPTI_1ST_HASH_BUCKET* buckets [26];
} MGPTI_DICTIONARY;

extern MGPTI_DICTIONARY __mgpti_dict;

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  /* __PTI_DICT_H */

