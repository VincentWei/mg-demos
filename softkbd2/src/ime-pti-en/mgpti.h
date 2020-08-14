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
** The external header for Predictive Text Input.
*/

#ifndef __PTI_H
#define __PTI_H

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* The maximal length of one keystokes */
#define MAX_LEN_KEYSTOKES 30

/*
 * Error codes
 */
#define PTI_ERR_MEMORY 1
#define PTI_ERR_FILE 2
#define PTI_ERR_NO_USER_DICT 3
#define PTI_ERR_WORD_DUP 4
#define PTI_ERR_WORD_LEN 5
#define PTI_ERR_WORD_NOSPACE 6

    /*
 * This function initializes the Predictive Text Input system.
 * It reads and initiailizes the user dictionary from the file
 * specified by dict_file.
 *
 * The file storing the user dictionary is a text file,
 * one line for one word.
 *
 * This function should be called when MiniGUI starts up.
 *
 * This function returns zero on success, non-zero on failure.
 */
    int ptiInitialize(const char *dict_file);

    /*
 * This function terminates the Predictive Text Input system.
 * It will save the user dictionary into the file sepcified by dict_file.
 *
 * This function should be called when MiniGUI quits.
 *
 * The file storing the user dictionary is a text file,
 * one line for one word.
 *
 * Note that the file specified by dict_file will be truncated.
 *
 * This function returns zero on success, non-zero on failure.
 */
    int ptiTerminate(const char *dict_file);

#ifndef MAX_LEN_USER_WORD
#define MAX_LEN_USER_WORD 12
#endif

    /*
 * This function saves the user spelled word into the user dictionary.
 *
 * Note that the length of one user word shoule be larger than 1
 * and less than (MAX_LEN_USER_WORD + 1).
 *
 * This function returns zero on success, non-zero on failure.
 */
    int ptiSaveUserWord(const char *user_word);

    /*
 * The buffer size to retrive the result from Predictive Text Input system.
 * Note that the size should be not less than the maximal length of the words
 * in the system dictionary and user dictionary.
 *
 * A number which is larger than 32 is a reasonable number.
 */

#ifndef PTI_RESULT_BUFF_LONG
#define PTI_RESULT_BUFF_LONG PHONE_RESULT_BUFF_LONG
#endif

#define PTI_CASE_ABC 0x01
#define PTI_CASE_Abc 0x02
#define PTI_CASE_abc 0x03
#define PTI_CASE_num 0x04
#define PTI_CASE_MAX (PTI_CASE_num + 1)

    /*
 * This function returns the candidate list for a given keystokes.
 * The result will be saved into the buffer pointed to by buff,
 * which is MGPTI_RESULT_BUFF_LEN long at least.
 *
 * The lenght of a keystokes should be larger than 1, and there is
 * should not '0' or '1'.
 *
 * The valid samples are:
 *  - "2345", "234", "462", "987", ...
 *  - "aco*", "dad*", "tw*", "zy*", ...
 *
 * The following samples are invalid:
 *  - "1", "2", "3451", "50", ...
 *  - "c*", "a*", "t*", "z*", ...
 *
 * Because the candidate list may be very long, the function will
 * only return a limited list of all candidates for one calling.
 * The return value of the function is the cursor in the complete
 * candidate list. If you want to get the next candidates, you should
 * call this function by pass 'cursor' with the last returned value.
 * For the first call, you should specify 'cursor' as zero.
 *
 * The case_mode can be one of the following values:
 *  - PTI_CASE_ABC: Convert all letters to upper case.
 *  - PTI_CASE_Abc: Convert the first letter to upper case.
 *  - PTI_CASE_abc: Reserve the letter case.
 *
 * When there is no further result, the function return -1.
 */
    intptr_t ptiMatchKeystokes(const char *keystokes, char *buff, int buff_len,
                               intptr_t cursor, int case_mode);

#if defined(__GNUC__)
#ifdef _DEBUG_MSG
#define _MY_PRINTF(fmt...) fprintf(stderr, fmt)
#else
#define _MY_PRINTF(fmt...)
#endif
#else /* __GNUC__ */
static inline void _MY_PRINTF(const char *fmt, ...)
{
#ifdef _DEBUG_MSG
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);
#endif
}
#endif /* __GNUC__ */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __PTI_H */
