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

#ifndef __IME_INPUT_H__
#define __IME_INPUT_H__

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

#define INPUT_BGCOLOR           8
  /* LightBlack */
#define INPUT_FGCOLOR           15
  /* LightWhite */
#define INPUT_AREAY          ((18 * 24)+10)

#define MAX_INPUT_LENGTH       32

#define MAGIC_NUMBER            "CCEGB"
#define CIN_ENAME_LENGTH        24
#define CIN_CNAME_LENGTH        16
#define MAX_PHRASE_LENGTH       20
#define SELECT_KEY_LENGTH       20
#define END_KEY_LENGTH          16
#define MAX_SEL_LENGTH          58

#define InputAreaX              16
/* key of toggle input method */
#define NR_INPUTMETHOD   10

typedef struct {
    unsigned long key1;        /* sizeof(ITEM=12) */
    unsigned long key2;
    unsigned short ch;
    unsigned short frequency;
} ITEM;

typedef struct {
    char magic_number[sizeof(MAGIC_NUMBER)];    /* magic number */
    char ename[CIN_ENAME_LENGTH];               /* ascii name */
    char cname[CIN_CNAME_LENGTH];               /* prompt */
    char selkey[SELECT_KEY_LENGTH];             /* select keys */

    char last_full;         /* last full code need a more SPACE? */
    int TotalKey;           /* number of keys needed */
    int MaxPress;           /* Max len of keystroke */
    int MaxDupSel;          /* how many keys used to select */
    int TotalChar;          /* Defined characters */

    unsigned char KeyMap[128];     /* Map 128 chars to 64(6 bit) key index */
    unsigned char KeyName[64];     /* Map 64 key to 128 chars */
    unsigned short KeyIndex[64];   /* 64 key first index of TotalChar */

    int PhraseNum;          /* Total Phrase Number */
    FILE *PhraseFile;       /* *.tab.phr Phrase File */
    FILE *AssocFile;        /* *.tab.lx LianXiang File */
    ITEM *item;             /* item */
} ime_input_table;

typedef struct {
   short l, t, r, b;
} hit_addr;

extern int pinyin_match_keystokes (void *method, const char* keystokes, char* buff, int buffer_len, int cursor);
extern int pinyin_translate_word(void *method, const char *strokes, char *buffer, int buffer_len, int index);
extern int pinyin_predict_pord(void *method, const char *lookfor, char * buffer, int buffer_len, int index);
extern  BOOL ime_pinyin_init (void);

#ifdef WIN32
extern int fread_long_win32(char* buff, size_t size, int count, FILE* fp);
#define fread fread_long_win32

#define F_OK 00
#endif

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif
