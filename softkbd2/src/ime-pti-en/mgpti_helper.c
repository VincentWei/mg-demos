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
** The helper for predictive text input.
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include <minigui/common.h>

#include "mgpti.h"
#include "mgpti_helper.h"

static const char *digit_letters_map[] = {
    "abc",
    "def",
    "ghi",
    "jkl",
    "mno",
    "pqrs",
    "tuv",
    "wxyz",
};

inline static const char *does_letter_match_keystoke(char letter, char keystoke)
{
    return strchr(digit_letters_map[keystoke - '2'], tolower(letter));
}

int _pti_does_word_match_keystokes(const char *word, const char *keystokes)
{
    while (*keystokes)
    {

        if (*word == '\0')
            return 0;

        if (does_letter_match_keystoke(*word, *keystokes))
            return 0;

        keystokes++;
        word++;
    }

    return 1;
}
