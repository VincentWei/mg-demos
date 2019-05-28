/*
** This file is a part of mg-demos package.
**
** Copyright (C) 2010 ~ 2019 FMSoft (http://www.fmsoft.cn).
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
 * @file utility.h
 * @Synopsis
 * @author DongKai
 * @version 1.0
 */

#ifndef  __UTILITY_H
#define  __UTILITY_H

#ifdef __cplusplus
extern "C" {
#endif

HDC getRefMemDC32b (void);
void releaseRefMemDC32b (void);

/* \brief struct of resource node */
typedef struct _RES_NODE {
    const char *res_name;
    int         type;
    int         bits;
    int         tag;
} RES_NODE;

int loadResByTag (const RES_NODE *node_list, int tag);
int unloadResByTag (const RES_NODE *node_list, int tag);

#define loadAllRes(node_list) stepLoadRes (node_list, 0)

HDC forceGetShadowDC (HWND hwnd, BOOL set_foreground);

void humanSize (char* buff, int len, float size);

#ifdef __cplusplus
}
#endif

#endif   /* __UTILITY_H */

