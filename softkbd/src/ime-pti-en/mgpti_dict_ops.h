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
** The header of operations on the system dictionary.
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

