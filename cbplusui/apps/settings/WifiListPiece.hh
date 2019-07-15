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
#ifndef _WIFI_LIST_PIECE_HH_
#define _WIFI_LIST_PIECE_HH_

#define RES_NAME_WIFI_LOCK  "images/select-wifi/wifi-lock.png"

#define RES_NAME_WIFI_F     "images/select-wifi/wifi-%d.png"
#define RES_NAME_WIFI_0     "images/select-wifi/wifi-0.png"
#define RES_NAME_WIFI_1     "images/select-wifi/wifi-1.png"
#define RES_NAME_WIFI_2     "images/select-wifi/wifi-2.png"
#define RES_NAME_WIFI_3     "images/select-wifi/wifi-3.png"
#define RES_NAME_WIFI_4     "images/select-wifi/wifi-4.png"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_LEN_SSID 255

typedef struct _mWifiListPieceClass mWifiListPieceClass;
typedef struct _mWifiListPiece mWifiListPiece;

#define mWifiListPieceHeader(clss)  \
    mTableViewPieceHeader(clss)     \
    mContainerCtrl* ctrl_ctnr; \
    const WIFI_HOTSPOTS_INFO* wifi_info;

#define mWifiListPieceClassHeader(clss, superCls) \
    mTableViewPieceClassHeader(clss, superCls)

struct _mWifiListPiece
{
    mWifiListPieceHeader(mWifiListPiece)
};

struct _mWifiListPieceClass
{
    mWifiListPieceClassHeader(mWifiListPiece, mTableViewPiece)
};

MGNCS_EXPORT extern mWifiListPieceClass g_stmWifiListPieceCls;

#ifdef __cplusplus
}
#endif

#endif /*_WIFI_LIST_PIECE_HH_*/
