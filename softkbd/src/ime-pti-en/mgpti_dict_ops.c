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
** The operations of system dictionary.
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include <minigui/common.h>

#undef _DEBUG_MSG

#include "mgpti.h"
#include "mgpti_dict.h"
#include "mgpti_dict_ops.h"

/* The first hash function for a word in the system dictionary */
inline static unsigned int word_first_hash (const char* key)
{
    return tolower (key [0]) - 'a';
}

/* The second hash function for a word in the system dictionary */
static unsigned int word_second_hash (const char* key)
{
  const char *p = key + 1; /* skip the first letter */
  unsigned int h = *p;

    if (h) {
        for (p += 1; *p != '\0'; p++) {
            h = (h << 5) - h + *p;
        }
    }
    return h % MGPTI_2ND_HASH_TABLE_SIZE;
}

#define _USE_BINARY_SEARCH 1

int _pti_dict_retrive_word (const char* word)
{
#if _USE_BINARY_SEARCH
    int low, mid, high, mid_idx, cmp;
#else
    int i;
#endif
    unsigned int _2nd_hash;
    MGPTI_2ND_HASH_BUCKET* _2nd_hash_bucket;

    _2nd_hash_bucket = __mgpti_dict.buckets[word_first_hash (word)]->buckets;
    _2nd_hash = word_second_hash (word);
    _2nd_hash_bucket += _2nd_hash;

#if _USE_BINARY_SEARCH
    /* a binary search ... */
    low = 0;
    high = _2nd_hash_bucket->nr_words - 1;

    while (low <= high) {
        mid = (low + high)/2;
        mid_idx = _2nd_hash_bucket->word_indices [mid];

        _MY_PRINTF ("the word in system dictionary: %s, "
                "the retriving word: %s\n",
                __mgpti_dict.sorted_words [mid_idx], word);

        /* skip the first letter when comparing */
        //cmp = strcasecmp (word + 1, __mgpti_dict.sorted_words [mid_idx] + 1);
        cmp = strcmp (word + 1, __mgpti_dict.sorted_words [mid_idx] + 1);

        if (cmp == 0)
            return mid_idx;
        else if (cmp < 0)
            high = mid - 1;
        else
            low = mid + 1;
    }
#else
    /* general search ... */
    for (i = 0; i < _2nd_hash_bucket->nr_words; i++) {
        int idx = _2nd_hash_bucket->word_indices [i];
        int cmp;

        if (word [1] < __mgpti_dict.sorted_words [idx] [1])
            return -1;

        _MY_PRINTF ("    the word in system dictionary: %s, "
                "the retriving word: %s\n",
                __mgpti_dict.sorted_words [idx], word);

        /* skip the first letter when comparing */
        //cmp = strcasecmp (word + 1, __mgpti_dict.sorted_words [idx] + 1);
        cmp = strcmp (word + 1, __mgpti_dict.sorted_words [idx] + 1);

        if (cmp == 0)
            return idx;
        else if (cmp < 0)
            break;
    }
#endif

    return -1;
}

int _pti_dict_retrive_first_word (const char* leading)
{
    int len = strlen (leading);

    if (len == 0) 
        return -1;
    else if (len == 1) {
        int i, idx;
        MGPTI_1ST_HASH_BUCKET* _1st_hash_bucket;

        _1st_hash_bucket = __mgpti_dict.buckets[word_first_hash (leading)];

        for (i = 0, idx = -1; (i < 26 && idx < 0); i++) {
            idx = _1st_hash_bucket->indices [i];
        }

        return idx;
    }
    else {
        int i, j, low, mid, high, cmp;
        int hash = word_first_hash (leading);
        char ch = tolower (leading [1]);

        /* get the low index */
        low  = __mgpti_dict.buckets[hash]->indices [ch - 'a'];
        if (low == -1) 
            return -1;

        /* get the high index */
        if (ch != 'z') {
            i = hash;
            j = ch - 'a' + 1;
        }
        else {
            i = hash + 1;
            j = 0;
        }

        high = -1;
        for (; i < 26; i++) {
            for (high = -1; (j < 26 && high < 0); j++) {
                high = __mgpti_dict.buckets[i]->indices [j];
            }

            if (high >= 0)
                break;

            /* reset j to zero */
            j = 0;
        }

        if (high < 0) {
            high = __mgpti_dict.max_index;
        }

        mid = -1;
        /* binary search in the sorted-words array */
        while (low <= high) {
            mid = (low + high)/2;

            _MY_PRINTF ("new low, new mid, new high: %d, %d, %d (%s, %s, %s).\n", 
                low, mid, high, 
                __mgpti_dict.sorted_words [low], 
                __mgpti_dict.sorted_words [mid],
                __mgpti_dict.sorted_words [high]);

            if (!__mgpti_dict.sorted_words [mid])
                break;

            //cmp = strcasecmp (leading, __mgpti_dict.sorted_words [mid]);
            cmp = strcmp (leading, __mgpti_dict.sorted_words [mid]);
            if (cmp == 0) 
                return mid;
            else if (cmp < 0)
                high = mid - 1;
            else
                low = mid + 1;

        }

        if (__mgpti_dict.sorted_words [low] &&
                //strncasecmp (leading, __mgpti_dict.sorted_words [low], len) == 0) {
                strncmp (leading, __mgpti_dict.sorted_words [low], len) == 0) {
            return low;
        }
    }
    return -1;
}

