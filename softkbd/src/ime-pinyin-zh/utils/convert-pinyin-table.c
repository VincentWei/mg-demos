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
** Copyright (C) 2019 FMSoft (http://www.fmsoft.cn).
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
/*!============================================================================
** convert-pinyin-table.c:
**      A tool converting old pinyin table to a new architecture-independent table.
**
** Created by Vincent Wei at 31/05/2019
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <minigui/common.h>

#include <iconv.h>

#include "old_pinyin.h"
#include "old_pinyin_table.h"

static NCS_PINYIN_IME_TABLE* init_pinyin_dict(void)
{
    NCS_PINYIN_IME_TABLE *pinyin_dict;

    pinyin_dict = (NCS_PINYIN_IME_TABLE*)ime_tab;
    pinyin_dict->item=(NCS_PINYIN_ITEM*)(pinyin_dict+1);

    pinyin_dict->wordFile   = (char *)ime_tab_phr;
    pinyin_dict->assocFile  = (char *)ime_tab_lx;

    return pinyin_dict;
}

static void output_items(NCS_PINYIN_IME_TABLE *pinyin_dict, iconv_t ic)
{
    int i;

    printf("static const IME_ZH_ITEM items[] = {\n");
    for (i = 0; i < pinyin_dict->totalChar; i++) {
        printf("    {0x%08x, 0x%08x, 0x%04x, 0x%04x},\n",
                pinyin_dict->item[i].key1, pinyin_dict->item[i].key2,
                pinyin_dict->item[i].off_idx, pinyin_dict->item[i].freq);
    }
    printf("};\n");
    printf("\n");
}

static void output_words(NCS_PINYIN_IME_TABLE *pinyin_dict, iconv_t ic)
{
    int i;
    unsigned int* offsets = (unsigned int*)pinyin_dict->wordFile;

    offsets++;
    printf("static const unsigned int word_offsets[] = {\n");
    for (i = 0; i < pinyin_dict->wordNum; i++) {
        if ((i % 8) == 0)
            printf("   ");
        printf(" 0x%08x,", offsets[i]);
        if (((i + 1) % 8) == 0)
            printf("\n");
    }
    if (i % 8)
        printf("\n");
    printf("};\n");
    printf("\n");

    int words_len = sizeof(ime_tab_phr) - ((pinyin_dict->wordNum + 1) << 2);
    unsigned char* words = (unsigned char*)pinyin_dict->wordFile +
                ((pinyin_dict->wordNum + 1) << 2);

    printf("static const unsigned char words[] = {\n");
    for (i = 0; i < words_len; i++) {
        if ((i % 8) == 0)
            printf("   ");
        printf(" 0x%02x,", words[i]);
        if (((i + 1) % 8) == 0)
            printf("\n");
    }
    if (i % 8)
        printf("\n");
    printf("};\n");
    printf("\n");
}

static void output_associated(NCS_PINYIN_IME_TABLE *pinyin_dict, iconv_t ic)
{
    int i;
    int nr = sizeof (ime_tab_lx) / sizeof (unsigned int);
    unsigned int* associated = (unsigned int*)pinyin_dict->assocFile;

    printf("static const unsigned short associated[] = {\n");
    for (i = 0; i < nr; i++) {
        if ((i % 4) == 0)
            printf("   ");
        printf(" 0x%04x,", associated[i]);
        if (((i + 1) % 4) == 0)
            printf("\n");
    }

    if (i % 4)
        printf("\n");

    printf("};\n");
    printf("\n");
}

static void output_table(NCS_PINYIN_IME_TABLE *pinyin_dict)
{
    int i;
    int nr_associated = sizeof (ime_tab_lx) / sizeof (unsigned int);

    printf("IME_ZH_TABLE _ime_zh_tab_pinyin = {\n");
    printf("    \"Pinyin\",\n");
    printf("    \"GB2312\",\n");

    printf("    {\n");
    for (i = 0; i < 128; i++) {
        if ((i % 8) == 0)
            printf("       ");
        printf(" 0x%02x,", pinyin_dict->keyMap[i]);
        if (((i + 1) % 8) == 0)
            printf("\n");
    }
    printf("    },\n");

    printf("    {\n");
    for (i = 0; i < 64; i++) {
        if ((i % 8) == 0)
            printf("       ");
        printf(" 0x%02x,", pinyin_dict->keyName[i]);
        if (((i + 1) % 8) == 0)
            printf("\n");
    }
    printf("    },\n");

    printf("    {\n");
    for (i = 0; i < 64; i++) {
        if ((i % 8) == 0)
            printf("       ");
        printf(" 0x%04x,", pinyin_dict->keyIndex[i]);
        if (((i + 1) % 8) == 0)
            printf("\n");
    }
    printf("    },\n");

    printf("    %d,\n", pinyin_dict->totalKey);
    printf("    %d,\n", pinyin_dict->totalChar);
    printf("    %d,\n", pinyin_dict->wordNum);
    printf("    %d,\n", nr_associated);
    printf("    items,\n");
    printf("    word_offsets,\n");
    printf("    words,\n");
    printf("    associated,\n");
    printf("};\n");
    printf("\n");
}

int main (int argc, const char* argv[])
{
    char buff[1024];
    NCS_PINYIN_IME_TABLE *pinyin_dict;

    pinyin_dict = init_pinyin_dict();

    iconv_t ic = iconv_open ("UTF-8", "GB18030");
    if (ic == (iconv_t)-1) {
        perror ("iconv_open");
        exit (1);
    }

    char* src = pinyin_dict->cname;
    size_t src_len = strlen(pinyin_dict->cname);
    char* dst = buff;
    size_t dst_len = 1024;
    size_t ret = iconv(ic, &src, &src_len, &dst, &dst_len);
    if (ret == (size_t)-1) {
        perror ("iconv");
        exit (2);
    }

    _WRN_PRINTF ("magic_number: %s", pinyin_dict->magic_number);
    _WRN_PRINTF ("ename: %s", pinyin_dict->ename);
    _WRN_PRINTF ("totalKey: %d", pinyin_dict->totalKey);
    _WRN_PRINTF ("totalChar: %d", pinyin_dict->totalChar);
    _WRN_PRINTF ("keyMap: %s", pinyin_dict->keyMap);
    _WRN_PRINTF ("keyName: %s", pinyin_dict->keyName);
    _WRN_PRINTF ("wordNum: %d", pinyin_dict->wordNum);
    _WRN_PRINTF ("maxPress: %d", pinyin_dict->maxPress);
    _WRN_PRINTF ("maxDupSel: %d", pinyin_dict->maxDupSel);

    printf("/*\n");
    printf("** This file is generated by `convert-pinyin-table`.\n");
    printf("** Do not edit this file manually.\n");
    printf("*/\n");
    printf("\n");
    printf("#include \"ime_zh_table.h\"\n");
    printf("\n");

    output_items(pinyin_dict, ic);

    output_words(pinyin_dict, ic);

    output_associated(pinyin_dict, ic);

    output_table(pinyin_dict);

    iconv_close(ic);

    return 0;
}

