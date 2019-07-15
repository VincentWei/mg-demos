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
** Copyright (C) 2018 Beijing Joobot Technogolies Inc.
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
 * @file MenuListPiece.cc
 * @Synopsis The implementation of menu list piece.
 * @author Vincent Wei
 * @version 1.0
 * @date 19/05/2018
 */

#include "global.h"
#include "resource.h"
#include "utilities.h"

#include "MenuListPiece.hh"

static void mMenuListPiece_construct (mMenuListPiece *self)
{
    Class(mTableViewPiece).construct ((mTableViewPiece*)self, NCS_TABLEVIEW_INDEX_STYLE);
}

static int mMenuListPiece_numberOfSections (mMenuListPiece* self)
{
    return 1;
}

static int mMenuListPiece_numberOfRowsInSection (mMenuListPiece* self, int section)
{
    return self->nr_entries;
}

static void set_switch_colors (mSlideSwitchPiece* piece)
{
    ARGB color_b[]   = {0xffffffff, 0xffdedede, 0xffc4c7c9, 0xffd0d3d4, 0xffd0d3d4};
    float pos_b[]    = {0.0, 0.68, 0.85, 0.93, 1.0};

    ARGB color_bb[]  = {0xffb4b4b4, 0xffadadad, 0xff808080, 0xff707070, 0xffa9a9a9, 0xffb9b9b9};
    float pos_bb[]   = {0.0, 0.68, 0.85, 0.93, 0.95, 1.0};

    mColorTable colorTable_b, colorTable_bb;
    ShapeTransRoundPiece_setColorTable (&colorTable_b, color_b, pos_b, TABLESIZE(color_b));
    ShapeTransRoundPiece_setColorTable (&colorTable_bb, color_bb, pos_bb, TABLESIZE(color_bb));

    _c(piece)->setButton (piece, 1, -1, &colorTable_b, &colorTable_bb);
}

static BOOL onSwitchStateChanged (mMenuListPiece* self, mSlideSwitchPiece *switch_piece, int event_id, DWORD param)
{
    if (event_id == NCSN_SLIDESWITCHPIECE_OFF || event_id == NCSN_SLIDESWITCHPIECE_ON) {
        HWND hParent = GetParent (self->ctrl_ctnr->hwnd);
        SendNotifyMessage (hParent, MSG_USER, switch_piece->addData, event_id);
        return TRUE;
    }

    return FALSE;
}

#define ITEM_ACT_H          ACTIVITY_ITEM_WITH_BRIEF_H
    #define ITEM_NAME_H     32
    #define ITEM_VALUE_H    28
#define ITEM_SWITCH_H       ACTIVITY_CONTENT_ITEM_H

#define SWITCH_SIZE_W       60
#define SWITCH_SIZE_H       24

#define ARROW_SIZE_W        20
#define MARGIN_H            5
#define MARGIN_V            5

#define SIZE_MARK_CHECK_W   20

static mHotPiece* create_item_userpiece (mMenuListPiece* self, int row)
{
    RECT rc;
    const MENU_ENTRY* entry = self->menu_entries + row;
    mPanelPiece* piece_panel = NULL;

    /* user Piece */
    piece_panel = NEWPIECEEX (mPanelPiece, (DWORD)row);
    if (entry->type == TYPE_MENU_ENTRY_DESC) {

        RECT rcText = {0, 0, ACTIVITY_CONTENT_ITEM_W - MARGIN_H * 2, ACTIVITY_CONTENT_H};
        SelectFont (HDC_SCREEN, (PLOGFONT)GetResource (Str2Key (GLOBAL_FONT_SM)));
        DrawText (HDC_SCREEN, entry->name, -1, &rcText, DT_WORDBREAK | DT_LEFT | DT_TOP | DT_CALCRECT);
        SetRect (&rc, 0, 0, ACTIVITY_CONTENT_ITEM_W, rcText.bottom + MARGIN_V * 3);
        _c(piece_panel)->setRect (piece_panel, &rc);

        /* name piece */
        mHotPiece *name_piece = (mHotPiece*)NEWPIECE (mTextPiece);
        SetRect (&rc, 0, 0, ACTIVITY_CONTENT_ITEM_W - MARGIN_H * 2, rcText.bottom);
        _c(name_piece)->setRect (name_piece, &rc);
        _c(name_piece)->setProperty (name_piece, NCSP_TEXTPIECE_TEXTCOLOR, (DWORD)0xFFAAAAAA);
        _c(name_piece)->setProperty (name_piece, NCSP_TEXTPIECE_LOGFONT, (DWORD)GetResource (Str2Key (GLOBAL_FONT_SM)));
        _c(name_piece)->setProperty (name_piece, NCSP_LABELPIECE_ALIGN, (DWORD)NCS_ALIGN_LEFT);
        _c(name_piece)->setProperty (name_piece, NCSP_LABELPIECE_VALIGN, (DWORD)NCS_VALIGN_TOP);
        _c(name_piece)->setProperty (name_piece, NCSP_LABELPIECE_AUTOWRAP, (DWORD)TRUE);
        _c(name_piece)->setProperty (name_piece, NCSP_LABELPIECE_WORDBREAK, (DWORD)TRUE);
        _c(name_piece)->setProperty (name_piece, NCSP_LABELPIECE_LABEL, (DWORD)entry->name);
        _c(piece_panel)->addContent (piece_panel, name_piece, MARGIN_H, MARGIN_V);
    }
    else if (entry->type != TYPE_MENU_ENTRY_SWITCH) {
        int item_h = ITEM_ACT_H;
        int name_h = ITEM_NAME_H;
        if (entry->type == TYPE_MENU_ENTRY_CHECKABLE || entry->value == NULL) {
            item_h = ITEM_SWITCH_H;
            name_h = ITEM_SWITCH_H;
        }

        SetRect (&rc, 0, 0, ACTIVITY_W, item_h);
        _c(piece_panel)->setRect (piece_panel, &rc);

        if (entry->type == TYPE_MENU_ENTRY_ACT && entry->activity != NULL) {
            /* arrow piece */
            mHotPiece* arrow_piece = (mHotPiece*)NEWPIECE (mImagePiece);
            SetRect (&rc, 0, 0, ARROW_SIZE_W, item_h);
            _c(arrow_piece)->setRect (arrow_piece, &rc);
            _c(arrow_piece)->setProperty (arrow_piece, NCSP_IMAGEPIECE_IMAGE, (DWORD)GetResource (Str2Key (GLOBAL_IMG_ARROW_RIGHT)));
            _c(piece_panel)->addContent (piece_panel, arrow_piece, ACTIVITY_CONTENT_ITEM_W - ARROW_SIZE_W, 0);
        }
        else if (entry->type == TYPE_MENU_ENTRY_CHECKABLE && strcasecmp (entry->value, SWITCH_ON) == 0) {
            mHotPiece *piece_mark_check = (mHotPiece*)NEWPIECE (mImagePiece);
            SetRect (&rc, 0, 0, SIZE_MARK_CHECK_W, item_h);
            _c(piece_mark_check)->setRect (piece_mark_check, &rc);
            _c(piece_mark_check)->setProperty (piece_mark_check, NCSP_IMAGEPIECE_IMAGE, (DWORD)GetResource (Str2Key (GLOBAL_IMG_MARK_CHECK)));
            _c(piece_panel)->addContent (piece_panel, piece_mark_check, ACTIVITY_CONTENT_ITEM_W - SIZE_MARK_CHECK_W, 0);
        }

        /* name piece */
        mHotPiece *name_piece = (mHotPiece*)NEWPIECE (mTextPiece);
        SetRect (&rc, 0, 0, ACTIVITY_CONTENT_ITEM_W - ARROW_SIZE_W - MARGIN_H, name_h);
        _c(name_piece)->setRect (name_piece, &rc);
        _c(name_piece)->setProperty (name_piece, NCSP_TEXTPIECE_TEXTCOLOR, (DWORD)0xFFFFFFFF);
        _c(name_piece)->setProperty (name_piece, NCSP_TEXTPIECE_LOGFONT, (DWORD)GetResource (Str2Key (GLOBAL_FONT_NM)));
        _c(name_piece)->setProperty (name_piece, NCSP_LABELPIECE_LABEL, (DWORD)entry->name);
        _c(name_piece)->setProperty (name_piece, NCSP_LABELPIECE_ALIGN, (DWORD)NCS_ALIGN_LEFT);
        _c(name_piece)->setProperty (name_piece, NCSP_LABELPIECE_WORDBREAK, (DWORD)FALSE);
        _c(piece_panel)->addContent (piece_panel, name_piece, MARGIN_H, 0);

        if (entry->type != TYPE_MENU_ENTRY_CHECKABLE && entry->value != NULL) {
            /* value piece */
            mHotPiece *value_piece = (mHotPiece*)NEWPIECE (mTextPiece);
            SetRect (&rc, 0, 0, ACTIVITY_CONTENT_ITEM_W - ARROW_SIZE_W - MARGIN_H, ITEM_VALUE_H);
            _c(value_piece)->setRect (value_piece, &rc);
            _c(value_piece)->setProperty (value_piece, NCSP_TEXTPIECE_TEXTCOLOR, (DWORD)0xFFAAAAAA);
            _c(value_piece)->setProperty (value_piece, NCSP_TEXTPIECE_LOGFONT, (DWORD)GetResource (Str2Key (GLOBAL_FONT_SM)));
            _c(value_piece)->setProperty (value_piece, NCSP_LABELPIECE_LABEL, (DWORD)entry->value);
            _c(value_piece)->setProperty (value_piece, NCSP_LABELPIECE_ALIGN, (DWORD)NCS_ALIGN_LEFT);
            _c(value_piece)->setProperty (value_piece, NCSP_LABELPIECE_WORDBREAK, (DWORD)FALSE);
            _c(piece_panel)->addContent (piece_panel, value_piece, MARGIN_H, ITEM_ACT_H - ITEM_VALUE_H);
        }
    }
    else {
        int event_ids[] = { NCSN_SLIDESWITCHPIECE_OFF, NCSN_SLIDESWITCHPIECE_ON, 0 };

        SetRect (&rc, 0, 0, ACTIVITY_W, ITEM_SWITCH_H);
        _c(piece_panel)->setRect (piece_panel, &rc);

        /* name piece */
        mHotPiece *name_piece = (mHotPiece*)NEWPIECE (mTextPiece);
        SetRect (&rc, 0, 0, ACTIVITY_CONTENT_ITEM_W - SWITCH_SIZE_W - MARGIN_H, ITEM_SWITCH_H);
        _c(name_piece)->setRect (name_piece, &rc);
        _c(name_piece)->setProperty (name_piece, NCSP_TEXTPIECE_TEXTCOLOR, (DWORD)0xFFFFFFFF);
        _c(name_piece)->setProperty (name_piece, NCSP_TEXTPIECE_LOGFONT, (DWORD)GetResource (Str2Key (GLOBAL_FONT_NM)));
        _c(name_piece)->setProperty (name_piece, NCSP_LABELPIECE_LABEL, (DWORD)entry->name);
        _c(name_piece)->setProperty (name_piece, NCSP_LABELPIECE_ALIGN, (DWORD)NCS_ALIGN_LEFT);
        _c(name_piece)->setProperty (name_piece, NCSP_LABELPIECE_WORDBREAK, (DWORD)FALSE);
        _c(piece_panel)->addContent (piece_panel, name_piece, MARGIN_H, 0);

        /* switch piece */
        mSlideSwitchPiece* switch_piece = (mSlideSwitchPiece*)NEWPIECEEX (mSlideSwitchPiece, (DWORD)row);
        set_switch_colors (switch_piece);

        SetRect (&rc, 0, 0, SWITCH_SIZE_W, SWITCH_SIZE_H);
        _c(switch_piece)->setRect (switch_piece, &rc);
        _c(switch_piece)->setProperty(switch_piece, NCSP_SLIDESWITCH_RADIUS, 5);
        if (strcmp (entry->value, SWITCH_ON) == 0)
            _c(switch_piece)->setState (switch_piece, NCSN_SLIDESWITCHPIECE_ON);
        else
            _c(switch_piece)->setState (switch_piece, NCSN_SLIDESWITCHPIECE_OFF);
        ncsAddEventListeners ((mObject*)switch_piece, (mObject*)self,
                (NCS_CB_ONPIECEEVENT)onSwitchStateChanged, event_ids);

        _c(piece_panel)->addContent (piece_panel, (mHotPiece*)switch_piece,
                ACTIVITY_CONTENT_ITEM_W - SWITCH_SIZE_W - MARGIN_H, (ITEM_SWITCH_H - SWITCH_SIZE_H)/2);
    }

    return (mHotPiece*)piece_panel;
}

static mTableViewItemPiece* mMenuListPiece_createItemForRow (mMenuListPiece* self, const mIndexPath* indexpath)
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

static void mMenuListPiece_rowDidSelectAtIndexPath (mMenuListPiece* self, const mIndexPath* indexpath)
{
    /* simply use the index of the row clicked as the notification code */
    ncsNotifyParent (self->ctrl_ctnr, (int)indexpath->row);
}

static BOOL mMenuListPiece_willSelectRowAtIndexPath (mMenuListPiece* self, const mIndexPath* indexpath)
{
    /* do not highlight the item */
    return FALSE;
}

static void mMenuListPiece_destroy (mMenuListPiece *self)
{
    Class(mTableViewPiece).destroy ((mTableViewPiece*)self);
}

BEGIN_MINI_CLASS(mMenuListPiece, mTableViewPiece)
	CLASS_METHOD_MAP(mMenuListPiece, construct)
	CLASS_METHOD_MAP(mMenuListPiece, numberOfSections)
	CLASS_METHOD_MAP(mMenuListPiece, numberOfRowsInSection)
	CLASS_METHOD_MAP(mMenuListPiece, createItemForRow)
	CLASS_METHOD_MAP(mMenuListPiece, rowDidSelectAtIndexPath)
	CLASS_METHOD_MAP(mMenuListPiece, willSelectRowAtIndexPath)
	CLASS_METHOD_MAP(mMenuListPiece, destroy)
END_MINI_CLASS

mMenuListPiece* createMenuListView (mContainerCtrl* ctrl_ctnr, RECT* rc, const MENU_ENTRY* menu_entries, int nr_entries)
{
    mMenuListPiece* menuListPiece;
    mPanelPiece* panel;

    menuListPiece = (mMenuListPiece *)NEWPIECE (mMenuListPiece);
    menuListPiece->ctrl_ctnr = ctrl_ctnr;
    menuListPiece->nr_entries = nr_entries;
    menuListPiece->menu_entries = menu_entries;

    _c(menuListPiece)->setRect (menuListPiece, rc);
    _c(menuListPiece)->reloadData (menuListPiece);

    panel = NEWPIECE (mPanelPiece);
    _c(panel)->addContent (panel, (mHotPiece*)menuListPiece, 0, 0);
    _c(panel)->setRect (panel, rc);

    _c(ctrl_ctnr)->setBody (ctrl_ctnr, (mHotPiece*)panel);

    /* set scrollbar color here */
    mHotPiece *scrollbar;
    scrollbar = menuListPiece->m_scrollbar->piece;
    _c(scrollbar)->setProperty(scrollbar, NCSP_TRANROUND_BKCOLOR, COLOR_SCROLLBAR);

    return menuListPiece;
}

