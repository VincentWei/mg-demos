/*
** The header of operations on the system dictionary.
**
** Copyright (C) 2007 FMSoft
**
** All right reserved by FMSoft.
**
*/

#ifndef __PTI_DICT_OPS_H
	#define __PTI_DICT_OPS_H

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/*
 * Retrive the word in the system dictionary.
 *
 * This function returns the index of the word in 
 * the sorted words arrary on success, -1 on failure.
 */
int _pti_dict_retrive_word (const char* word);

/*
 * Retrive the first word which has the specified leading letters
 * in the system dictionary.
 *
 * This function returns the index of the first word in 
 * the sorted-words arrary on success, -1 on failure.
 */
int _pti_dict_retrive_first_word (const char* leading);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  /* __PTI_DICT_OPS_H */

