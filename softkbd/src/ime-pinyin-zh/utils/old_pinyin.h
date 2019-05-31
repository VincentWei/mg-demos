
#define PINYIN_MAGIC_NUMBER          "CCEGB"
#define PINYIN_ENAME_LENGTH          24
#define PINYIN_CNAME_LENGTH          16
#define PINYIN_WORD_LENGTH           16
#define PINYIN_SELECT_KEY_LENGTH     16
#define PINYIN_KEY_LENGTH            16
#define PINYIN_SEL_LENGTH            58

typedef struct {
    unsigned int key1;
    unsigned int key2;
    unsigned short off_idx;
    unsigned short freq;
}NCS_PINYIN_ITEM;

typedef struct {
    unsigned int off_begin;
    unsigned int off_end;
}NCS_PINYIN_PHOFFSET;

typedef struct {
    char magic_number[sizeof(PINYIN_MAGIC_NUMBER)];
    char ename[PINYIN_ENAME_LENGTH];
    char cname[PINYIN_CNAME_LENGTH];
    char selkey[PINYIN_SELECT_KEY_LENGTH];

    char last_full;
    int  totalKey;
    int  maxPress;
    int  maxDupSel;
    int  totalChar;

    unsigned char  keyMap[128];
    unsigned char  keyName[64];
    unsigned short keyIndex[64];

    int    wordNum;
    char*  wordFile;
    char*  assocFile;
    NCS_PINYIN_ITEM * item;
}NCS_PINYIN_IME_TABLE;

