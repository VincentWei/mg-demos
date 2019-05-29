/*
** The dictionary for predictive text input.
**
** Copyright (C) 2007 FMSoft
**
** All right reserved by FMSoft.
**
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>

#undef _DEBUG_MSG
//#define _DEBUG_MSG

#include "mgpti.h"
#include "mgpti_dict.h"
#include "mgpti_dict_ops.h"
#include "mgpti_user_dict.h"
#include "mgpti_helper.h"

static MGPTI_USER_DICTIONARY* _pti_user_dict=NULL;
#if 0
void debug_info(char * text)
{
    static int y=0;
    if (strlen(text) < 1)
        return;
    TextOut(HDC_SCREEN, 250, y, text);
    y+=20;
    if (y>320)
        y=0;
}
#else
void debug_info(char * text)
{

}
#endif
int ptiInitialize (const char* user_dict_file) 
{
    FILE* fp;
    int rv = 0;

    if (_pti_user_dict == NULL) {
        _pti_user_dict = (MGPTI_USER_DICTIONARY*)calloc (1, sizeof(MGPTI_USER_DICTIONARY));

        if (_pti_user_dict == NULL)
            return PTI_ERR_MEMORY;

        _MY_PRINTF ("The size of user dictionary is %d\n", 
                sizeof (MGPTI_USER_DICTIONARY));

        fp = fopen (user_dict_file, "r");
        if (fp == NULL)
            return PTI_ERR_FILE;

        rv = _pti_user_dict_load (_pti_user_dict, fp);

        fclose (fp);
    }

    return rv;
}

int ptiTerminate (const char* user_dict_file) 
{
    int rv = 0;

    if (_pti_user_dict) {
        FILE* fp = fopen (user_dict_file, "w");
        if (fp == NULL) {
            free (_pti_user_dict);
            _pti_user_dict = NULL;
            return PTI_ERR_FILE;
        }

        rv = _pti_user_dict_save (_pti_user_dict, fp);

        fclose (fp);
        free (_pti_user_dict);
        _pti_user_dict = NULL;
    }

    return rv;
}

int ptiSaveUserWord (const char* user_word) 
{
    int rv = PTI_ERR_NO_USER_DICT;

    if (_pti_user_dict) {
        rv = _pti_user_dict_add_word (_pti_user_dict, user_word);
    }

    return rv;
}

/* 
 * The first 1024 bytes are used to store the matched completed words,
 * the left 512 bytes are used to store the matched incompleted words.
 */
#define SIZE_CW_SPACE   1024
#define SIZE_ICW_SPACE  512

static char _matched_buffer [SIZE_CW_SPACE + SIZE_ICW_SPACE + 2];
static int _left_cw_space;
static int _left_icw_space;

static int copy_words_from_source (const char* source, char* buff, 
        int buff_len, int copied, int case_mode)
{
    int i, len;
    char* space=NULL;

    while (source) {
        space = strchr (source, ' ');
        if (space == NULL) {
            len = strlen (source) + 1;
            if (len == 1)
                return copied;

            if (copied + len < buff_len) {
                strncpy (buff + copied, source, strlen(source));
                switch (case_mode) {
                    case PTI_CASE_ABC:
                        for (i = 0; i < len - 1; i++)
                            buff [copied + i] = toupper (buff [copied + i]);
                        break;
                    case PTI_CASE_Abc:
                        buff [copied] = toupper (buff [copied]);
                        break;
                    default:
                        /* do nothing */
                        break;
                }

                copied += len;
                buff [copied - 1] = ' ';
            }

            return copied;
        }
        else {
            len = space - source + 1;
            if (len == 1)
                return copied;

            if (copied + len < buff_len) {
                strncpy (buff + copied, source, len);
                switch (case_mode) {
                    case PTI_CASE_ABC:
                        for (i = 0; i < len - 1; i++)
                            buff [copied + i] = toupper (buff [copied + i]);
                        break;
                    case PTI_CASE_Abc:
                        buff [copied] = toupper (buff [copied]);
                        break;
                    default:
                        /* do nothing */
                        break;
                }

                copied += len;
                source += len;
            }
            else
                return copied;
        }
    }

    return copied;
}

static intptr_t write_matched_words (char* buff, int buff_len, char* cursor, int case_mode)
{
    int copied = 0;

    if (_matched_buffer [0] == '\0' 
            && _matched_buffer [SIZE_CW_SPACE + 1] == '\0')
        return -1;

    if (cursor == NULL)
        cursor = _matched_buffer;
    else if (cursor < _matched_buffer || 
            cursor >= _matched_buffer + sizeof (_matched_buffer))
        return -1;

    if (cursor < _matched_buffer + SIZE_CW_SPACE) {
        copied = copy_words_from_source (cursor, buff, buff_len, 0, case_mode);
        if (copied != 0 && cursor [copied] != '\0') {
            buff [copied] = '\0';
            cursor += copied;

            _MY_PRINTF ("write_matched_words: 1=%s\n", buff);
            return (intptr_t)cursor;
        }
    }

    if (copied < buff_len) {
        int new_copied;

        if (cursor < _matched_buffer + SIZE_CW_SPACE)
            cursor = _matched_buffer + SIZE_CW_SPACE + 1;

        new_copied = copy_words_from_source (cursor, buff, buff_len, copied, case_mode);

        buff [new_copied] = '\0';
        cursor += new_copied - copied;
    }

    _MY_PRINTF ("write_matched_words: 2=%s\n", buff);
    if (buff [0])
        return (intptr_t) cursor;

    return -1;
}

static int add_word_to_matched_buffer (const char* word)
{
    char* dest;
    int len = strlen (word) + 1;

    if (_left_cw_space - len < 0) {
        _MY_PRINTF ("The matched buffer is full, discard the word: %s\n", word);
        return 1;
    }

    dest = strncpy (_matched_buffer + SIZE_CW_SPACE - _left_cw_space, word, strlen(word));
    dest [len - 1] = ' ';
    _left_cw_space -= len;

    _MY_PRINTF ("A result word added to the matched buffer: %s\n", word);
    return 0;
}

static int add_leading_word_to_matched_buffer (const char* word)
{
    char* dest;
    int len = strlen (word) + 2;

    if (_left_icw_space - len < 0) {
        _MY_PRINTF ("The matched buffer is full, "
                "discard the leading word: %s\n", word);
        return 1;
    }

    dest = strncpy (_matched_buffer + 
                    SIZE_CW_SPACE + 1 + SIZE_ICW_SPACE - _left_icw_space, word, strlen(word));
    dest [len - 2] = '*';
    dest [len - 1] = ' ';
    _left_icw_space -= len;

    _MY_PRINTF ("A result leading word added to the matched buffer: %s\n", word);
    return 0;
}

typedef struct _DIGIT_TO_LETTERS {
    const char* letters;
    int nr_letters;
} DIGIT_TO_LETTERS;

static DIGIT_TO_LETTERS digit_letters_map [] = {
    {"abc", 3},
    {"def", 3},
    {"ghi", 3},
    {"jkl", 3},
    {"mno", 3},
    {"pqrs", 4},
    {"tuv", 3},
    {"wxyz", 4},
};

static char _last_keystokes [MAX_LEN_KEYSTOKES + 1];
static int _nr_digits;
static char _try_word [MAX_LEN_KEYSTOKES + 1];

static void make_try_word (int i)
{
    int j;
    int map_idx;

    if (i == _nr_digits) {
        int result;

        _MY_PRINTF ("\n\n  Trying word: %s\n", _try_word);

        /* search in the system dictionary */
        result = _pti_dict_retrive_word (_try_word);
        if (result != -1) {
            char* const* word = __mgpti_dict.sorted_words + result;
            if (add_word_to_matched_buffer (*word))
                return;

            if (result < __mgpti_dict.max_index) {
                //if (strncasecmp (*word, *(word + 1), _nr_digits) == 0) {
                if (strncmp (*word, *(word + 1), _nr_digits) == 0) {
                    add_leading_word_to_matched_buffer (_try_word);
                }
            }
        }
        else if (_pti_dict_retrive_first_word (_try_word) != -1) {
            add_leading_word_to_matched_buffer (_try_word);
        }

        return;
    }
    else if (_nr_digits > 3 && i > 2) {
        if (_pti_dict_retrive_first_word (_try_word) == -1) {
            _MY_PRINTF ("  Giving up the leading word: %s\n", _try_word);
            return;
        }
    }

    map_idx = _last_keystokes [i] - '2';
    for (j = 0; j < digit_letters_map [map_idx].nr_letters; j++) {
        _try_word [i] = digit_letters_map [map_idx].letters[j];
        if (i < _nr_digits)
            make_try_word (i + 1);
    }
}

static void match_digit_keystokes (void)
{
    /* match keystokes in the user dictionary */
    if (_pti_user_dict) {
        char* const* result = _pti_user_dict_match_key_strokes (
                _pti_user_dict, _last_keystokes);

        if (result && strlen (*result) == _nr_digits) {
            add_word_to_matched_buffer (*result);

            result ++;
            while (*result) {
                int result_len = strlen (*result);

                if (result_len == _nr_digits) {
                    if (_nr_digits > 2 &&
                            _pti_does_word_match_keystokes (*result + 2, 
                            _last_keystokes + 2))
                        add_word_to_matched_buffer (*result);
                    else
                        add_word_to_matched_buffer (*result);
                }

                result ++;
            }
        }
    }

    /* match keystokes in the system dictionary */
    memset (_try_word, 0, MAX_LEN_KEYSTOKES + 1);
    make_try_word (0);
}

static void match_leading_word (void)
{
    /* match keystokes in the user dictionary */
    if (_pti_user_dict) {
        char* const* result = _pti_user_dict_retrive_first_word (
                _pti_user_dict, _last_keystokes);

        while (result && *result) {
            //if (strncasecmp (*result, _last_keystokes, _nr_digits) == 0)
            if (strncmp (*result, _last_keystokes, _nr_digits) == 0)
                add_word_to_matched_buffer (*result);
            else
                break;
            result ++;
        }
    }

    /* match keystokes in the system dictionary */
    {
        int result;

        result = _pti_dict_retrive_first_word (_last_keystokes);
        while (result >= 0 && result <= __mgpti_dict.max_index) {
            char* const* word = __mgpti_dict.sorted_words + result;
            //if (strncasecmp (*word, _last_keystokes, _nr_digits) == 0) {
            if (strncmp (*word, _last_keystokes, _nr_digits) == 0) {
                if (add_word_to_matched_buffer (*word))
                    break;
            }
            else
                break;
            result ++;
        }
    }
}

intptr_t ptiMatchKeystokes (const char* keystokes, char* buff, int buff_len,
        intptr_t cursor, int case_mode) 
{
	if(!keystokes || !buff)
		return -1;

    int nr_digits = strlen (keystokes);

    if (nr_digits < 1 || nr_digits > MAX_LEN_KEYSTOKES)
        return -1;

    if ((isdigit (*keystokes) 
                && strcmp (keystokes, _last_keystokes) == 0)//) {
				|| (isalpha (*keystokes) 
              && strcmp (keystokes, _last_keystokes)/*, nr_digits - 1)*/ == 0)) {

        /* return the result from matched_buffer */
        return write_matched_words (buff, buff_len, (char*)cursor, case_mode);
    } else {
		if (isalpha (*keystokes)  && strncmp (keystokes, _last_keystokes, nr_digits - 1) == 0) {
			//do nothing; 
		} else {
			/* force to reset the cursor */
			cursor = 0;
		}
    }

    if (isalpha (*keystokes) && keystokes [nr_digits - 1] != '*')
        return -1;

    memset (buff, 0, buff_len);

    /* empty the matched buffer */
    memset (_matched_buffer, 0, sizeof (_matched_buffer));
    _left_cw_space = SIZE_CW_SPACE;
    _left_icw_space = SIZE_ICW_SPACE;

    /* save the last keystokes */
    strncpy (_last_keystokes, keystokes, MAX_LEN_KEYSTOKES);
    _last_keystokes [MAX_LEN_KEYSTOKES] = '\0';
    _nr_digits = strlen (_last_keystokes);

    if (isdigit (_last_keystokes [0])) {
        /* a real digital keystokes */
        match_digit_keystokes ();
        _MY_PRINTF ("\nThe matched completed words: \n\"%s\"\n", _matched_buffer);
        _MY_PRINTF ("The matched incompleted words: \n\"%s\"\n", 
                _matched_buffer + SIZE_CW_SPACE + 1);
    }
    else {
        /* a leading word */
        _last_keystokes [_nr_digits - 1] = '\0';
        _nr_digits --;

        match_leading_word ();
    }

    return write_matched_words (buff, buff_len, (char*)cursor, case_mode);
}
