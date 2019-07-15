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
** Copyright (C) 2018 Beijing Joobot Technologies Co., Ltd.
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
 * @file MenuListPiece.hh
 * @Synopsis
 * @author Vincent Wei
 * @version 1.0
 * @date 05/06/2018
 */

#ifndef _MENU_LIST_PIECE_HH_
#define _MENU_LIST_PIECE_HH_

#define TYPE_MENU_ENTRY_ACT         0
#define TYPE_MENU_ENTRY_SWITCH      1
#define TYPE_MENU_ENTRY_CHECKABLE   2
#define TYPE_MENU_ENTRY_DESC        3
#define TYPE_MENU_ENTRY_USER        4

#define IDE_MIN                     100
#define SWITCH_OFF                  "off"
#define SWITCH_ON                   "on"

typedef struct _MENU_ENTRY {
    int id;
    int type;
    const char* activity;
    const char* name;
    const char* value;
    const char* intent_key;
    const char* intent_value;
    DWORD   add_data;
} MENU_ENTRY;

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _mMenuListPieceClass mMenuListPieceClass;
typedef struct _mMenuListPiece mMenuListPiece;

#define mMenuListPieceHeader(clss)  \
    mTableViewPieceHeader(clss)     \
    mContainerCtrl* ctrl_ctnr;      \
    const MENU_ENTRY* menu_entries; \
    int nr_entries;

#define mMenuListPieceClassHeader(clss, superCls) \
    mTableViewPieceClassHeader(clss, superCls)

struct _mMenuListPiece
{
    mMenuListPieceHeader(mMenuListPiece)
};

struct _mMenuListPieceClass
{
    mMenuListPieceClassHeader(mMenuListPiece, mTableViewPiece)
};

MGNCS_EXPORT extern mMenuListPieceClass g_stmMenuListPieceCls;

mMenuListPiece* createMenuListView (mContainerCtrl* ctrl_ctnr, RECT* rc, const MENU_ENTRY* menu_entries, int nr_entries);

#ifdef __cplusplus
}
#endif

#endif /*_MENU_LIST_PIECE_HH_*/

