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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#include "ime_pinyin.h"
#include "ime_zh_table.h"

#define MAX_INPUT_LENGTH        15
#define MAX_WORD_LENGTH         20
#define SELECT_KEY_LENGTH       16
#define END_KEY_LENGTH          16
#define MAX_SEL_LENGTH          58

static IME_ZH_TABLE* cur_table;

static void find_matched_key(void);
static void fill_matched_chars(int j);
static void fill_associated_chars(int index);
static void find_associated_key(int index);

static char seltab[16][MAX_WORD_LENGTH];
static int curr_sel_num=0;   /* Current Total Selection Number */

/* Input key buffer */
static unsigned int input_key[MAX_INPUT_LENGTH];
static unsigned int saved_input_key[MAX_INPUT_LENGTH];
static int input_count, input_matched, start_key, end_key;

static int WORK_TIM;

/*
** When enter multi_page_mode:
**  start_key .... curr_page_idx .... next_page_idx .... end_key
*/
static int save_start_key,save_end_key, save_multi_page_mode,
           save_next_page_idx, save_curr_page_idx;
static int next_page_idx, curr_page_idx, multi_page_mode;

static unsigned int val1, val2, key1, key2;
static int is_associate_mode;

// starting index of matched char/phrases
static int char_index[MAX_INPUT_LENGTH];

static void clear_input (void)
{
    memset (input_key, 0, sizeof (input_key));
    memset (seltab, 0, sizeof (seltab));

    multi_page_mode     = 0;
    next_page_idx       = 0;
    curr_page_idx       = 0;
    curr_sel_num        = 0;
    input_count         = 0;
    input_matched       = 0;
    is_associate_mode   = 0;
}

static void find_associated_key (int index)
{
    unsigned int offset;
    unsigned short ofs[2];

    if (index < 0xB0A1) {
        start_key = end_key = 0;
        return;  /* no match */
    }

    offset = (index / 256 - 0xB0) * 94 + index % 256 - 0xA1;
    ofs[0] = cur_table->associated[offset];
    ofs[1] = cur_table->associated[offset + 1];

    start_key = 72 * 94 + 1 + ofs[0];
    end_key = 72 * 94 + 1 + ofs[1];
}

static void load_phrase (int phrno, char *tt)
{
    unsigned int ofs[2];
    int len;

    ofs[0] = cur_table->word_offsets[phrno];
    ofs[1] = cur_table->word_offsets[phrno + 1];

    len = ofs[1] - ofs[0];
    if (len > 128 || len <= 0 ) {
        strcpy (tt, "err");
        return;
    }

    memcpy (tt, cur_table->words + ofs[0], len);
    tt[len] = 0;
}

static void putstr (unsigned char *p)
{
    int index,len =strlen(( char*)p);
    if (input_count <= input_matched) {
        /* All Match */
        index = (int)p[len-2] * 256 + p[len-1];
        clear_input();

        if (cur_table->associate_mode) {
            find_associated_key(index);
            curr_page_idx = start_key;
            multi_page_mode = 0;
            fill_associated_chars(start_key);

            if (curr_sel_num > 0) {
                is_associate_mode = 1;
            }
        }
    }
    else {
        int count = input_count - input_matched;
        int matched = input_matched, i;

        multi_page_mode = next_page_idx = curr_page_idx = 0;
        input_count = input_matched = 0;

        for (i =0; i <count; i++)
            saved_input_key[i] =input_key[matched+i];

        memset (input_key, 0, sizeof(input_key));
        for (i = 1; i <= count; i++) {
            /* feed the additional keys */

            input_key[input_count] = saved_input_key[input_count];
            input_count++;

            if (input_count <=input_matched+1)
            {
                find_matched_key ();
                multi_page_mode    =0;
                curr_page_idx =start_key;
                fill_matched_chars (start_key);
            }

        }

        if (input_matched == 0) {
            /* left key has no match, delete */
            clear_input();
            return;
        }

    }
}

/* 6 bit a key mask */
static const unsigned int mask[]=
{
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x3F000000, 0x3FFC0000, 0x3FFFF000, 0x3FFFFFC0, 0x3FFFFFFF, 0x3FFFFFFF,
    0x3FFFFFFF, 0x3FFFFFFF, 0x3FFFFFFF, 0x3FFFFFFF, 0x3FFFFFFF, 0x3FFFFFFF
};


/* After add/delete a char, search the matched char/phrase, update the
   start_key/end_key key, save the related keys at first, if no match
   is found, we may restore its original value
*/
static void find_matched_key (void)
{
    save_start_key       = 0;
    save_end_key         = 0;
    save_multi_page_mode = 0;
    save_next_page_idx   = 0;
    save_curr_page_idx   = 0;

    val1 = input_key[4] | (input_key[3]<<6) | (input_key[2]<<12) |
            (input_key[1]<<18) | (input_key[0]<<24);
    val2 = input_key[9] | (input_key[8]<<6) | (input_key[7]<<12) |
            (input_key[6]<<18) | (input_key[5]<<24);

    if (input_count == 1)
        start_key = cur_table->key_indices[input_key[0]];
    else
        start_key = char_index[input_count-1];

    end_key = cur_table->key_indices[input_key[0]+1];

    for (; start_key < end_key; start_key++) {
        key1 = (cur_table->items[start_key].key1 & mask[input_count+5]);
        key2 = (cur_table->items[start_key].key2 & mask[input_count]);

        if (key1 < val1 || key2 < val2)
            continue;

        break;
    }

    char_index[input_count] = start_key;
}

/*  Find the matched chars/phrases and fill it into SelTab
    The starting checked index is j

    The Selection Line 1xxx 2xxx,  80-20=60 60/2=30 chinese chars only
    0-9 Selection can contain only 30 chinese chars
*/

static void fill_associated_chars (int index)
{
    char str[25];
    int current_length = 0;

    curr_sel_num = 0;

    while (curr_sel_num < cur_table->max_dup_sel &&
            index < end_key && current_length < MAX_SEL_LENGTH) {
        load_phrase(cur_table->associated[index], str);
        strcpy(seltab[curr_sel_num], str+2);
        current_length += strlen(seltab[curr_sel_num++]);
        index++;
    }

    /* check if more than one page */
    if (index < end_key && curr_sel_num == cur_table->max_dup_sel) {
      /* has another matched key, so enter multi_page_mode, has more pages */
        next_page_idx = index;
        multi_page_mode = 1;
    }
    else if (multi_page_mode) {
        next_page_idx = start_key; /* rotate selection */
    }
    else {
        multi_page_mode = 0;
    }
}

int pinyin_predict_word(void *method, const char* lookfor,
        char* buffer, int buffer_len, int index)
{
    int    lenth = 0;
    static char lookfor_bk[64];

    lenth =0;
    sprintf (lookfor_bk, "%s", lookfor);
    bzero (seltab, 16*MAX_WORD_LENGTH);
    putstr ((unsigned char *)lookfor);

    buffer[0] = 0;
    while (1) {
        fill_associated_chars (start_key+index/10 *10);

        lenth +=(strlen (seltab[index])+1);
        if (0==strlen (seltab[index]))
            return -1;

        if (lenth>=buffer_len )
            return index;
        strcat (buffer, seltab[index]);
        strcat (buffer, " ");
        index++;
    }

    return -1;
}

static void fill_matched_chars (int j)
{
    int selected_number = 0, current_length = 0;

    while ((cur_table->items[j].key1 & mask[input_count+5])==val1&&
           (cur_table->items[j].key2 & mask[input_count])==val2&&
           selected_number < cur_table->max_dup_sel && j < end_key &&
           current_length < MAX_SEL_LENGTH) {

        unsigned short ch = cur_table->items[j].ch;
        if (ch < 0xA1A1)
            load_phrase (ch, seltab[selected_number]);
        else {
            memcpy (&seltab[selected_number], &(cur_table->items[j].ch), 2);
            seltab [selected_number][2] = '\0';
        }

        current_length += strlen(seltab[selected_number++]);
        j++;
    }

    if (selected_number == 0) {
        /* some match found */
        start_key = save_start_key;
        end_key = save_end_key;
        multi_page_mode = save_multi_page_mode;
        next_page_idx = save_next_page_idx;
        curr_page_idx = save_curr_page_idx;
        return;    /* keep the original selection */
    }

    curr_sel_num = selected_number;

    for (selected_number = curr_sel_num; selected_number < 16; selected_number++)
        seltab[selected_number][0] = '\0';  /* zero out the unused area */

    input_matched = input_count; /* until now we have some matches */

    /* check if more than one page */
    if (j < end_key && (cur_table->items[j].key1 & mask[input_count+5]) == val1 &&
            (cur_table->items[j].key2 & mask[input_count] ) == val2 &&
            curr_sel_num == cur_table->max_dup_sel) {
      /* has another matched key, so enter multi_page_mode, has more pages */
        next_page_idx = j;
        multi_page_mode = 1;
    }
    else if (multi_page_mode) {
        next_page_idx = start_key; /* rotate selection */
    }
    else
        multi_page_mode = 0;
}

static BOOL char_match_exist (const char *match)
{
    int len = strlen (match);
    int idex = 0;
    int key = 0;

    bzero (input_key, sizeof (unsigned int)*input_count);
    find_matched_key ();
    clear_input();

    for (idex = 0; idex < len; idex++) {
       key = cur_table->key_map[(int)match[idex]];
       input_key[input_count++] = key;
    }

    find_matched_key();
    curr_page_idx = start_key;
    multi_page_mode    = 1;
    fill_matched_chars(start_key);

    if (end_key== 0) {
        return FALSE;
    }

    return TRUE;
}

int pinyin_translate_word(void *method, const char *strokes,
        char *buffer, int buffer_len, int index)
{
    static char strokes_bk[32];
    static int  length = 0;
    static int  page = 0;
    int         idex, key;
    int         lensum;
    int         len;

    if (index < 0)
        return -1;

    len = strlen(strokes);
    if (len != length || 0!=bcmp (strokes_bk, strokes, length)||WORK_TIM || page!=index/10) {
        sprintf (strokes_bk, "%s", strokes);
        WORK_TIM = FALSE;
        page = 0;
        length = len;
        clear_input ();

        for (idex = 0; idex<length && idex < MAX_INPUT_LENGTH ; idex++) {
            key = cur_table->key_map[(int)strokes[idex]];
            input_key [input_count++] = key;
        }

        find_matched_key();
        curr_page_idx = start_key;
        multi_page_mode = 1;
        fill_matched_chars(start_key);
        if (input_matched < input_count)
               return -1;
    }

    lensum  =0;
    buffer[0] =0;
    multi_page_mode =1;

    while (multi_page_mode) {
        if (page!=index/10) {
            page= index/10;
            fill_matched_chars (start_key+page*10);
        }

        if (input_matched < input_count)
            return -1;

        lensum+=(strlen(seltab[index%10])+1);
        if (lensum > buffer_len)
           return index;

        if (0==strlen (seltab[index%10]))
            break;

        strcat (buffer, seltab[index%10]);
        strcat (buffer, " ");
        index++;
    }

    return -1;
}

static const char *digit_to_letter_map[] = {
    "abc",
    "def",
    "ghi",
    "jkl",
    "mno",
    "pqrs",
    "tuv",
    "wxyz"
};

int pinyin_match_keystokes (void *method, const char* keystokes,
        char* buff, int buffer_len, int cursor)
{
    static int  len = 0;
    static char match_buff[2][100][20];
    static int  match_idex = 0;
    static int  max = 1;
    int    idex, idex1, idex2, idex3;
    const char *keys;

    if (cursor == -1)
        return -1;

    WORK_TIM = TRUE;
    len = strlen (keystokes);
    bzero (match_buff, 2*100*20);

    idex2 = 0;
    idex3 = 0;
    match_idex = 0;
    max = 1;

    for (idex = 0; idex < len; idex++) {
        keys = digit_to_letter_map[*(keystokes+idex)-'2'];

        for (idex1 = 0; idex1<max; idex1++) {
            for (idex2 = 0; idex2 < strlen (keys); idex2++) {

                sprintf (match_buff[(match_idex+1)%2][idex3], "%s%c", match_buff[match_idex][idex1], *(keys+idex2));
                if (char_match_exist (match_buff[(match_idex+1)%2][idex3]))
                    idex3++;
            }
        }

        max   = idex3;
        idex3 = 0;
        match_idex = (match_idex+1)%2;
    }

    {
        int lensum =0;
        buff[0]    =0;

        while (cursor<max) {
            lensum += (strlen (match_buff[match_idex][cursor++])+1);

            if (lensum > buffer_len) {
                return (cursor-1);
            }
            strcat (buff, match_buff[match_idex][cursor-1]);
            strcat (buff, " ");
        }

        return -1;
    }

    return -1;
}

extern IME_ZH_TABLE _ime_zh_tab_pinyin;

BOOL ime_pinyin_init (void)
{
    cur_table = &_ime_zh_tab_pinyin;
    cur_table->associate_mode = 1;
    cur_table->max_dup_sel = 10;
    return TRUE;
}

