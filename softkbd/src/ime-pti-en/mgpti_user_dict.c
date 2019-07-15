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
** The user dictionary for predictive text input.
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
#include "mgpti_user_dict.h"
#include "mgpti_helper.h"

inline static char letter_to_key (char ch)
{
    switch (ch) {
        case 'a' ... 'c':
        return '2';
        case 'd' ... 'f':
        return '3';
        case 'g' ... 'i':
        return '4';
        case 'j' ... 'l':
        return '5';
        case 'm' ... 'o':
        return '6';
        case 'p' ... 's':
        return '7';
        case 't' ... 'v':
        return '8';
        case 'w' ... 'z':
        return '9';
    }

    return '2';
}

/* the hash function of a word in the user dictionary */
static int user_word_hash (const char* word)
{
    char ch1 = tolower (word [0]);
    char ch2 = tolower (word [1]);

    ch1 = letter_to_key (ch1);
    ch2 = letter_to_key (ch2);

    return (ch1 - '2') * 10 + (ch2 - '2');
}

static int cmpwordp (const void *p1, const void *p2)
{
    if (*(char* const*)p1 != *(char* const*)p2) {
        if (*(char* const*)p1 == NULL)
            return 1;
        if (*(char* const*)p2 == NULL)
            return -1;
    }
    else
        return 0;

    //return strcasecmp (*(char* const*)p1, *(char* const*)p2);
    return strcmp (*(char* const*)p1, *(char* const*)p2);
}

int _pti_user_dict_load (MGPTI_USER_DICTIONARY* user_dict, FILE* fp)
{
    int i;
    char line_buff [512];

    for (i = 0; i < NR_USER_WORDS; i++) {
        int len;
        char* s;

        if (fgets (line_buff, 511, fp) == NULL) {
            return 0;
        }

        len = strlen (line_buff);
        if (line_buff [len - 1] == '\n') {
            line_buff [len - 1] = '\0';
            len -= 1;
        }

        if (_pti_dict_retrive_word (line_buff) != -1) {
            _MY_PRINTF ("mgpti: the word is already in "
                    "the system dictionary: %s\n", line_buff);
            continue;
        }

        if (len > MAX_LEN_USER_WORD) {
            _MY_PRINTF ("mgpti: the word is too long: %s\n", line_buff);
            continue;
        }

        if (len < 2) {
            _MY_PRINTF ("mgpti: the word is too short: %s\n", line_buff);
            continue;
        }

        s = line_buff;
        while (*s) {
            if (!isalpha (*s)) {
                _MY_PRINTF ("mgpti: not a word: %s\n", line_buff);
                break;
            }
            s ++;
        }

        if (*s == '\0') {
            int j;
            int hash_value = user_word_hash (line_buff);

            _MY_PRINTF ("mgpti: the hash value for the word %s is %d\n", 
                    line_buff, hash_value);

            for (j = 0; j < MAX_NR_WORDS_IN_A_BUCKET; j++) {
#if 0
                if (user_dict->buckets [hash_value] [j] && 
                        strcasecmp (user_dict->buckets [hash_value] [j], 
                                line_buff) == 0) {
#else
                if (user_dict->buckets [hash_value] [j] && 
                        strcmp (user_dict->buckets [hash_value] [j], 
                                line_buff) == 0) {
#endif
                    j = MAX_NR_WORDS_IN_A_BUCKET;
                    break;
                }

                if (user_dict->buckets [hash_value] [j] == NULL)
                    break;
            }

            if (j < MAX_NR_WORDS_IN_A_BUCKET) {
                char* dest = user_dict->user_words [user_dict->last_empty_slot];
                strncpy (dest, line_buff, MAX_LEN_USER_WORD);
                dest [MAX_LEN_USER_WORD] = '\0';
                user_dict->last_empty_slot ++;
                user_dict->buckets [hash_value] [j] = dest;

                _MY_PRINTF ("mgpti: a word loaded: %s\n", dest);
            }
            else {
                _MY_PRINTF ("mgpti: the #%d bucket already is full for "
                        "the word: %s\n", hash_value, line_buff);
            }
        }
    }

    /* sort the buckets in dictionary order */
    for (i = 0; i < 26; i++) {
        if (user_dict->buckets [i][1] != NULL)
            qsort (user_dict->buckets [i], MAX_NR_WORDS_IN_A_BUCKET, 
                    sizeof (char*), cmpwordp);
    }

    return 0;
}

int _pti_user_dict_save (const MGPTI_USER_DICTIONARY* user_dict, FILE* fp)
{
    int i;

    for (i = 0; i < user_dict->last_empty_slot; i++) {
        fputs (user_dict->user_words [i], fp);
        fputs ("\n", fp);
    }

    return 0;
}

int _pti_user_dict_add_word (MGPTI_USER_DICTIONARY* user_dict, 
                const char* user_word)
{
    int len;
    const char* s;

    if (user_dict->last_empty_slot >= NR_USER_WORDS) {
        _MY_PRINTF ("mgpti: the user dictionary is full: %s\n", user_word);
        return PTI_ERR_WORD_NOSPACE;
    }

    len = strlen (user_word);

    if (_pti_dict_retrive_word (user_word) != -1) {
        _MY_PRINTF ("mgpti: the word is already in the system dictionary: %s\n",
                user_word);
        return PTI_ERR_WORD_DUP;
    }

    if (len > MAX_LEN_USER_WORD) {
        _MY_PRINTF ("mgpti: the word is too long: %s\n", user_word);
        return PTI_ERR_WORD_LEN;
    }

    if (len < 2) {
        _MY_PRINTF ("mgpti: the word is too short: %s\n", user_word);
        return PTI_ERR_WORD_LEN;
    }

    s = user_word;
    while (*s) {
        if (!isalpha (*s)) {
            _MY_PRINTF ("mgpti: not a word: %s\n", user_word);
            break;
        }
        s ++;
    }

    if (*s == '\0') {
        int j;
        int hash_value = user_word_hash (user_word);

        _MY_PRINTF ("mgpti: the hash value for the word %s is %d\n", 
                user_word, hash_value);

        for (j = 0; j < MAX_NR_WORDS_IN_A_BUCKET; j++) {
#if 0
            if (user_dict->buckets [hash_value] [j] && 
                    strcasecmp (user_dict->buckets [hash_value] [j], 
                            user_word) == 0) {
#else
            if (user_dict->buckets [hash_value] [j] && 
                    strcmp (user_dict->buckets [hash_value] [j], 
                            user_word) == 0) {
#endif
                j = MAX_NR_WORDS_IN_A_BUCKET;
                break;
            }

            if (user_dict->buckets [hash_value] [j] == NULL)
                break;
        }

        if (j < MAX_NR_WORDS_IN_A_BUCKET) {
            char* dest = user_dict->user_words [user_dict->last_empty_slot];
            strncpy (dest, user_word, MAX_LEN_USER_WORD);
            dest [MAX_LEN_USER_WORD] = '\0';
            user_dict->last_empty_slot ++;
            user_dict->buckets [hash_value] [j] = dest;

            _MY_PRINTF ("mgpti: a word added: %s\n", dest);
        }
        else {
            _MY_PRINTF ("mgpti: the #%d bucket already is full for "
                    "the word: %s\n", hash_value, user_word);

            return PTI_ERR_WORD_NOSPACE;
        }

#if 0
        int i;

        printf ("before sorting...\n");
        for (i = 0; i <= j; i++) {
            printf ("%s, ", user_dict->buckets [hash_value] [i]);
        }
        printf ("\n");
#endif

        if (j > 0)
            qsort (user_dict->buckets [hash_value], j + 1, 
                    sizeof (char*), cmpwordp);

#if 0
        printf ("after sorting...\n");
        for (i = 0; i <= j; i++) {
            printf ("%s, ", user_dict->buckets [hash_value] [i]);
        }
        printf ("\n");
#endif

    }

    return 0;
}

char* const* _pti_user_dict_retrive_word (
                const MGPTI_USER_DICTIONARY* user_dict, const char* word)
{
    int low, mid, high, cmp;
    char* const* bucket;
    int len = strlen (word);

    if (len < 2)
        return NULL;

    bucket = user_dict->buckets [user_word_hash (word)];

    /* binary search ... */
    low = 0;
    high = 0;
    do {
        if (bucket [high]) high++;
    } while (high < MAX_NR_WORDS_IN_A_BUCKET);

    if (high == 0)
        return NULL;

    while (low <= high) {
        mid = (low + high)/2;

        _MY_PRINTF ("the word in the user dictionary: %s, "
                "the retriving word: %s\n", bucket [mid], word);

        if (!bucket[mid])
            break;

        //cmp = strcasecmp (word, bucket [mid]);
        cmp = strcmp (word, bucket [mid]);

        if (cmp == 0)
            return bucket + mid;
        else if (cmp < 0)
            high = mid - 1;
        else
            low = mid + 1;
    }

    return NULL;
}

char* const* _pti_user_dict_retrive_first_word (
                const MGPTI_USER_DICTIONARY* user_dict, const char* leading)
{
    int low, mid, high, cmp;
    char* const* bucket;
    int len = strlen (leading);

    if (len < 2)
        return NULL;

    bucket = user_dict->buckets [user_word_hash (leading)];

    /* binary search ... */
    low = 0;
    high = 0;
    do {
        if (bucket [high]) high++;
        low ++;
    } while (low < MAX_NR_WORDS_IN_A_BUCKET);

    if (high == 0)
        return NULL;

    low = 0;
    while (low <= high) {
        mid = (low + high)/2;

        _MY_PRINTF ("the word in the user dictionary: %s, "
                "the retriving leading word: %s, low:%d, mid:%d, high:%d\n", 
                bucket [low], leading, low, mid, high);

        if (!bucket[mid])
            break;

        //cmp = strcasecmp (leading, bucket [mid]);
        cmp = strcmp (leading, bucket [mid]);

        if (cmp == 0)
            return bucket + mid;
        else if (cmp < 0)
            high = mid - 1;
        else
            low = mid + 1;
    }

    //if (bucket[low] && strncasecmp (leading, bucket [low], len) == 0)
    if (bucket[low] && strncmp (leading, bucket [low], len) == 0)
        return bucket + low;

    return NULL;
}

/* the length of keystokes must be larger than 2 */
char* const* _pti_user_dict_match_key_strokes (
                const MGPTI_USER_DICTIONARY* user_dict, const char* keystokes)
{
    int i;
    char* const* bucket;
    int hash_value;
    
    hash_value = (keystokes [0]- '2') * 10 + (keystokes [1] - '2');
    _MY_PRINTF("_pti_user_dict_match_key_strokes hash_value=%d\n", hash_value);
    bucket = user_dict->buckets [hash_value];

    /* sequencing search ... */
    for (i = 0; i < MAX_NR_WORDS_IN_A_BUCKET; i++) {
        if (bucket [i] == NULL)
            break;

        if (_pti_does_word_match_keystokes (bucket [i] + 2, keystokes + 2))
            return bucket + i;
    }

    return NULL;
}

