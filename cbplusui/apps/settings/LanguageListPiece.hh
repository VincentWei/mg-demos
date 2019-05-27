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
