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
 * @file WifiListPiece.cc
 * @Synopsis The implementation of Wifi list piece.
 * @author Vincent Wei
 * @version 1.0
 * @date 19/05/2018
 */

#include "global.h"
#include "resource.h"
#include "utilities.h"

#include "NetworkService.hh"
#include "WifiListPiece.hh"

static void mWifiListPiece_construct (mWifiListPiece *self, DWORD add_data)
{
    Class(mTableViewPiece).construct ((mTableViewPiece*)self, NCS_TABLEVIEW_INDEX_STYLE);
    self->wifi_info = (const WIFI_HOTSPOTS_INFO*) add_data;
}

static int mWifiListPiece_numberOfSections (mWifiListPiece* self)
{
    return 1;
}

static int mWifiListPiece_numberOfRowsInSection (mWifiListPiece* self, int section)
{
    return self->wifi_info->hotspots->size ();
}

#define ICON_LOCK_W     12
#define ICON_SIGN_W     20
#define ICON_MARK_W     20

static mHotPiece* create_item_userpiece (mWifiListPiece* self, int row)
{
    const WifiHotspot* hotspot = self->wifi_info->hotspots->at(row);
    char res_name [128];

    RECT rc;
    mHotPiece* lock_piece = NULL;
    mHotPiece* signal_piece = NULL;
    mPanelPiece* panel_piece = NULL;

    /* lock piece */
    if (!hotspot->is_open) {
        lock_piece = (mHotPiece*)NEWPIECE (mImagePiece);
        SetRect (&rc, 0, 0, ICON_LOCK_W, ACTIVITY_CONTENT_ITEM_H);
        _c(lock_piece)->setRect (lock_piece, &rc);
        _c(lock_piece)->setProperty (lock_piece, NCSP_IMAGEPIECE_IMAGE, (DWORD)GetResource (Str2Key (RES_NAME_WIFI_LOCK)));
    }

    /* signal level */
    int signal_level = hotspot->signal / 20;
    if (signal_level <= 0)
        signal_level = 0;
    if (signal_level > 4)
        signal_level = 4;

    /* signal piece */
    signal_piece = (mHotPiece*)NEWPIECE (mImagePiece);
    SetRect (&rc, 0, 0, ICON_SIGN_W, ACTIVITY_CONTENT_ITEM_H);
    _c(signal_piece)->setRect (signal_piece, &rc);
    sprintf (res_name, RES_NAME_WIFI_F, signal_level);
    _c(signal_piece)->setProperty (signal_piece, NCSP_IMAGEPIECE_IMAGE, (DWORD)GetResource (Str2Key (res_name)));

    /* label piece */
    mHotPiece *txt_piece = (mHotPiece*)NEWPIECE (mLabelPiece);
    SetRect (&rc, 0, 0,
        ACTIVITY_CONTENT_ITEM_W - (4 + ICON_MARK_W + 4) - (4 + ICON_SIGN_W + 4) - (4 + ICON_LOCK_W + 4),  ACTIVITY_CONTENT_ITEM_H);
    _c(txt_piece)->setRect (txt_piece, &rc);
    _c(txt_piece)->setProperty (txt_piece, NCSP_LABELPIECE_LABEL, (DWORD)hotspot->ssid);
    _c(txt_piece)->setProperty (txt_piece, NCSP_LABELPIECE_ALIGN, (DWORD)NCS_ALIGN_LEFT);
    mLabelPiece_setWordBreak ((mLabelPiece*)txt_piece, FALSE);

    /* user Piece */
    panel_piece = NEWPIECE (mPanelPiece);
    SetRect (&rc, 0, 0, ACTIVITY_CONTENT_ITEM_W, ACTIVITY_CONTENT_ITEM_H);
    _c(panel_piece)->setRect (panel_piece, &rc);

    if (lock_piece) {
        _c(panel_piece)->addContent (panel_piece, lock_piece,
            ACTIVITY_CONTENT_ITEM_W - (4 + ICON_SIGN_W + 4) - (4 + ICON_LOCK_W + 4), 0);
    }
    _c(panel_piece)->addContent (panel_piece, signal_piece, ACTIVITY_CONTENT_ITEM_W - (4 + ICON_SIGN_W + 4), 0);
    _c(panel_piece)->addContent (panel_piece, txt_piece, 4 + ICON_MARK_W + 4, 0);

    return (mHotPiece*)panel_piece;
}

static mTableViewItemPiece* mWifiListPiece_createItemForRow (mWifiListPiece* self, const mIndexPath* indexpath)
{
    mTableViewItemPiece *item = NEWPIECEEX (mTableViewItemPiece, 0);
    RECT rc = {0, 0, ACTIVITY_CONTENT_ITEM_W, ACTIVITY_CONTENT_ITEM_H};
    mHotPiece *user_item;

    _c(item)->setRect (item, &rc);
    user_item = create_item_userpiece (self, (int)indexpath->row);
    if (user_item) {
        _c(item)->setUserPiece (item, (mHotPiece*)user_item);
    }

    /* set background color here */
    mShapeTransRoundPiece* bk = item->bkgndPiece;
    _c(bk)->setProperty (bk, NCSP_TRANROUND_BKCOLOR, COLOR_SCROLL_ITEM);
    return item;
}

static void mWifiListPiece_rowDidSelectAtIndexPath (mWifiListPiece* self, const mIndexPath* indexpath)
{
    /* simply use the index of the row clicked as the notification code */
    ncsNotifyParent (self->ctrl_ctnr, (int)indexpath->row);
}

static BOOL mWifiListPiece_willSelectRowAtIndexPath (mWifiListPiece* self, const mIndexPath* indexpath)
{
    /* do not highlight the item */
    return FALSE;
}

static void mWifiListPiece_destroy (mWifiListPiece *self)
{
    Class(mTableViewPiece).destroy ((mTableViewPiece*)self);
}

BEGIN_MINI_CLASS(mWifiListPiece, mTableViewPiece)
	CLASS_METHOD_MAP(mWifiListPiece, construct)
	CLASS_METHOD_MAP(mWifiListPiece, numberOfSections)
	CLASS_METHOD_MAP(mWifiListPiece, numberOfRowsInSection)
	CLASS_METHOD_MAP(mWifiListPiece, createItemForRow)
	CLASS_METHOD_MAP(mWifiListPiece, rowDidSelectAtIndexPath)
	CLASS_METHOD_MAP(mWifiListPiece, willSelectRowAtIndexPath)
	CLASS_METHOD_MAP(mWifiListPiece, destroy)
END_MINI_CLASS

