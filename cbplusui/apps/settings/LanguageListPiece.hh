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
#ifndef _LANGUAGE_LIST_PIECE_HH_
#define _LANGUAGE_LIST_PIECE_HH_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _mLanguageListPieceClass mLanguageListPieceClass;
typedef struct _mLanguageListPiece mLanguageListPiece;

#define mLanguageListPieceHeader(clss)  \
    mTableViewPieceHeader(clss)     \
    mContainerCtrl* ctrl_ctnr; \
    LanguageSettings* lang_settings; \
    LangMap::iterator it;

#define mLanguageListPieceClassHeader(clss, superCls) \
    mTableViewPieceClassHeader(clss, superCls)

struct _mLanguageListPiece
{
    mLanguageListPieceHeader(mLanguageListPiece)
};

struct _mLanguageListPieceClass
{
    mLanguageListPieceClassHeader(mLanguageListPiece, mTableViewPiece)
};

MGNCS_EXPORT extern mLanguageListPieceClass g_stmLanguageListPieceCls;

#ifdef __cplusplus
}
#endif

#endif /*_LANGUAGE_LIST_PIECE_HH_*/
