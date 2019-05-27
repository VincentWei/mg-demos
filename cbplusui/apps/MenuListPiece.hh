/*!============================================================================
 * @file MenuListPiece.hh 
 * @Synopsis  
 * @author Vincent Wei
 * @version 1.0
 * @date 05/06/2018
 *
 *  Copyright (C) 2018 Beijing Joobot Technogolies Inc.
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

