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

#ifndef __IME_RIME_H__
#define __IME_RIME_H__

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#define MAX_LEN_KEYSTOKES 30

        int RimeMatchKeystokes(const char *keystokes,
                               char *buffer, int buffer_len, int cursor);
        int rime_translate_word(void *method, const char *strokes,
                                char *buffer, int buffer_len, int index);
        int rime_predict_word(void *method, const char *lookfor,
                              char *buffer, int buffer_len, int index);
        int ime_rime_init(void);

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

#endif /* __IME_RIME_H__ */
