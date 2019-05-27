/*!============================================================================
 * @file LanguageListPiece.cc 
 * @Synopsis The implementation of language list piece. 
 * @author Vincent Wei
 * @version 1.0
 * @date 19/05/2018
 *
 *  Copyright (C) 2018 Beijing Joobot Technogolies Inc.
 */

#include "global.h"
#include "resource.h"
#include "utilities.h"

#include "RuntimeConfService.hh"
#include "LanguageListPiece.hh"

static void mLanguageListPiece_construct (mLanguageListPiece *self, DWORD add_data)
{
    Class(mTableViewPiece).construct ((mTableViewPiece*)self, NCS_TABLEVIEW_INDEX_STYLE);
    self->lang_settings = (LanguageSettings*) add_data;
}

static int mLanguageListPiece_numberOfSections (mLanguageListPiece* self)
{
    return 1;
}

static int mLanguageListPiece_numberOfRowsInSection (mLanguageListPiece* self, int section)
{
    return self->lang_settings->langs.size ();
}

#define SIZE_MARK_CHECK_W   19
#define SIZE_MARK_CHECK_H   13

static mHotPiece* create_item_userpiece (mLanguageListPiece* self, int idx)
{
    if (idx == 0) {
        self->it = self->lang_settings->langs.begin ();
    }
    else {
        self->it ++;
    }

    if (self->it == self->lang_settings->langs.end ()) {
        return NULL;
    }

    RECT rc;

    mPanelPiece* piece_panel = NEWPIECE (mPanelPiece);
    SetRect (&rc, 0, 0, ACTIVITY_CONTENT_ITEM_W, ACTIVITY_CONTENT_ITEM_H);
    _c(piece_panel)->setRect (piece_panel, &rc);

    /* label piece */
    mHotPiece *piece_lang_desc = (mHotPiece*)NEWPIECE (mLabelPiece);
    SetRect (&rc, 0, 0, ACTIVITY_CONTENT_ITEM_W - SIZE_MARK_CHECK_W - 4 - 4 - 4, ACTIVITY_CONTENT_ITEM_H);
    _c(piece_lang_desc)->setRect (piece_lang_desc, &rc);
    _c(piece_lang_desc)->setProperty (piece_lang_desc, NCSP_LABELPIECE_LABEL, (DWORD)self->it->second.c_str());
    _c(piece_lang_desc)->setProperty (piece_lang_desc, NCSP_LABELPIECE_ALIGN, (DWORD)NCS_ALIGN_LEFT);
    mLabelPiece_setWordBreak ((mLabelPiece*)piece_lang_desc, FALSE);

    _c(piece_panel)->addContent (piece_panel, piece_lang_desc, 4, 0);

    if (strcmp (self->it->first.c_str(), self->lang_settings->current) == 0) {
        mHotPiece *piece_mark_check = (mHotPiece*)NEWPIECE (mImagePiece);
        SetRect (&rc, 0, 0, SIZE_MARK_CHECK_W, ACTIVITY_CONTENT_ITEM_H);
        _c(piece_mark_check)->setRect (piece_mark_check, &rc);
        _c(piece_mark_check)->setProperty (piece_mark_check, NCSP_IMAGEPIECE_IMAGE, (DWORD)GetResource (Str2Key (GLOBAL_IMG_MARK_CHECK)));

        _c(piece_panel)->addContent (piece_panel, piece_mark_check, ACTIVITY_CONTENT_ITEM_W - SIZE_MARK_CHECK_W - 4 - 4, 0);
    }

    return (mHotPiece*)piece_panel;
}

static mTableViewItemPiece* mLanguageListPiece_createItemForRow (mLanguageListPiece* self, const mIndexPath* indexpath)
{
    mTableViewItemPiece *item = NEWPIECEEX (mTableViewItemPiece, 0);
    RECT rc = {0, 0, ACTIVITY_CONTENT_ITEM_W, ACTIVITY_CONTENT_ITEM_H};
    mHotPiece *user_item;

    _c(item)->setRect (item, &rc);
    user_item = create_item_userpiece (self, (int)indexpath->row);
    if (user_item) {
        _c(item)->setUserPiece (item, user_item);
    }

    /* set background color here */
    mShapeTransRoundPiece* bk = item->bkgndPiece;
    _c(bk)->setProperty (bk, NCSP_TRANROUND_BKCOLOR, COLOR_SCROLL_ITEM);
    return item;
}

static void mLanguageListPiece_rowDidSelectAtIndexPath (mLanguageListPiece* self, const mIndexPath* indexpath)
{
    /* simply use the index of the row clicked as the notification code */
    ncsNotifyParent (self->ctrl_ctnr, (int)indexpath->row);
}

static BOOL mLanguageListPiece_willSelectRowAtIndexPath (mLanguageListPiece* self, const mIndexPath* indexpath)
{
    /* do not highlight the item */
    return FALSE;
}

static void mLanguageListPiece_destroy (mLanguageListPiece *self)
{
    Class(mTableViewPiece).destroy ((mTableViewPiece*)self);
}

BEGIN_MINI_CLASS(mLanguageListPiece, mTableViewPiece)
	CLASS_METHOD_MAP(mLanguageListPiece, construct)
	CLASS_METHOD_MAP(mLanguageListPiece, numberOfSections)
	CLASS_METHOD_MAP(mLanguageListPiece, numberOfRowsInSection)
	CLASS_METHOD_MAP(mLanguageListPiece, createItemForRow)
	CLASS_METHOD_MAP(mLanguageListPiece, rowDidSelectAtIndexPath)
	CLASS_METHOD_MAP(mLanguageListPiece, willSelectRowAtIndexPath)
	CLASS_METHOD_MAP(mLanguageListPiece, destroy)
END_MINI_CLASS

