/*
** The helper for predictive text input.
**
** Copyright (C) 2007 FMSoft
**
** All right reserved by FMSoft.
**
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include <minigui/common.h>

#include "mgpti.h"
#include "mgpti_helper.h"

static const char* digit_letters_map [] = {
    "abc",
    "def",
    "ghi",
    "jkl",
    "mno",
    "pqrs",
    "tuv",
    "wxyz",
};

inline static const char* does_letter_match_keystoke (char letter, char keystoke)
{
    return strchr (digit_letters_map [keystoke - '2'], tolower (letter));
}

int _pti_does_word_match_keystokes (const char* word, const char* keystokes)
{
    while (*keystokes) {

        if (*word == '\0')
            return 0;

        if (does_letter_match_keystoke (*word, *keystokes))
            return 0;

        keystokes ++;
        word ++;
    }

    return 1;
}

