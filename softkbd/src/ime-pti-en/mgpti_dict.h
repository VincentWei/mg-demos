/*
** The header of dictionary for predictive text input.
**
** This file is created by 'words2c' by FMSoft (http://www.fmsoft.cn).
** Please do not modify it manually.
**
** Copyright (C) 2007 FMSoft
**
** All right reserved by FMSoft.
**
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

