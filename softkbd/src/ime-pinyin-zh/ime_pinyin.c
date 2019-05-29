#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#include "ime_pinyin.h"
#include "ime_tab_pinyin.h"

static void FindMatchKey(void);
static void FillMatchChars(int j);
static void FillAssociateChars(int index);
static void FindAssociateKey(int index);
static ime_input_table* IntCode_Init(void);
static int CurIME;


static ime_input_table *input_table[NR_INPUTMETHOD],*cur_table = NULL;
static char seltab[16][MAX_PHRASE_LENGTH];

static int CurSelNum=0;   /* Current Total Selection Number */
static unsigned long InpKey[MAX_INPUT_LENGTH],save_InpKey[MAX_INPUT_LENGTH];
   /* Input key buffer */
static int InputCount,InputMatch, StartKey,EndKey;
static int WORK_TIM;
static int save_StartKey,save_EndKey, save_MultiPageMode,
           save_NextPageIndex, save_CurrentPageIndex;
static int NextPageIndex,CurrentPageIndex,MultiPageMode;
/* When enter MultiPageMode:
   StartKey .... CurrentPageIndex .... NextPageIndex .... EndKey
*/
static unsigned long val1, val2,key1,key2;
static int IsAssociateMode;
static int CharIndex[MAX_INPUT_LENGTH];   // starting index of matched char/phrases

static char *key_map[]=
{
    "abc",
    "def",
    "ghi",
    "jkl",
    "mno",
    "pqrs",
    "tuv",
    "wxyz"
};

static int UseAssociateMode = 0;

/* 6 bit a key mask */
static const unsigned long mask[]=
{
  0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
  0x3F000000, 0x3FFC0000, 0x3FFFF000, 0x3FFFFFC0, 0x3FFFFFFF, 0x3FFFFFFF,
  0x3FFFFFFF, 0x3FFFFFFF, 0x3FFFFFFF, 0x3FFFFFFF, 0x3FFFFFFF, 0x3FFFFFFF
};

/***************************************************************************
 *                          some local functions                           *
 ***************************************************************************/

static ime_input_table* load_input_method (void)
{
#if MGUI_BYTEORDER == MGUI_BIG_ENDIAN
    int i;
#endif

    ime_input_table *table;
    table= (ime_input_table*)ime_tab;
    table->item=(ITEM*)(table+1);

#if MGUI_BYTEORDER == MGUI_BIG_ENDIAN

    table->TotalKey = ArchSwap32(table->TotalKey);
    table->MaxPress = ArchSwap32(table->MaxPress);
    table->MaxDupSel= ArchSwap32(table->MaxDupSel);
    table->TotalChar= ArchSwap32(table->TotalChar);
    table->PhraseNum= ArchSwap32(table->PhraseNum);

    for (i = 0; i < 64; i++)
    {
        table->KeyIndex [i] = ArchSwap16(table->KeyIndex[i]);
    }

#endif

    if( strcmp(MAGIC_NUMBER, table->magic_number) )
    {
        goto fail;
    }

#if MGUI_BYTEORDER == MGUI_BIG_ENDIAN
    for (i = 0; i < table->TotalChar; i++)
    {
        table->item[i].key1 = ArchSwap32(table->item[i].key1);
        table->item[i].key2 = ArchSwap32(table->item[i].key2);
        table->item[i].frequency = ArchSwap16(table->item[i].frequency);
    }

#endif
    table->PhraseFile =(FILE *) ime_tab_phr;
    table->AssocFile  =(FILE *)ime_tab_lx;
    return table;

fail:
    return NULL;
}


static void ClrIn (void)
{
    memset (InpKey, 0, sizeof ( InpKey));
    memset ( seltab, 0, sizeof ( seltab));

    MultiPageMode    = 0;
    NextPageIndex    = 0;
    CurrentPageIndex = 0;
    CurSelNum        = 0;
    InputCount       = 0;
    InputMatch       = 0;
    IsAssociateMode  = 0;   /* lian xiang */
}

static void FindAssociateKey (int index)
{
    char *fp =(char*)cur_table->AssocFile;
    int  ofs[2], offset;

    if (index < 0xB0A1)
    {
        StartKey = EndKey = 0;
        return;  /* no match */
    }

    offset = (index / 256 - 0xB0) * 94 + index % 256 - 0xA1;
    fp    +=(offset *sizeof (int));
    memcpy (ofs, fp, (sizeof (int)*2));

#if MGUI_BYTEORDER == MGUI_BIG_ENDIAN
    ofs[0] = ArchSwap32(ofs[0]);
    ofs[1] = ArchSwap32(ofs[1]);
#endif

    StartKey = 72 * 94 + 1 + ofs[0];
    EndKey = 72 * 94 + 1 + ofs[1];
}

static void load_phrase (int phrno, char *tt)
{
    char *fp =(char*)cur_table->PhraseFile;
    int  ofs[2], len;

    fp +=((phrno +1)<<2);
    memcpy (ofs, fp, (sizeof(int)*2));

#if MGUI_BYTEORDER == MGUI_BIG_ENDIAN
    ofs[0] = ArchSwap32(ofs[0]);
    ofs[1] = ArchSwap32(ofs[1]);
#endif
    len =ofs[1] -ofs[0];

    if (len > 128 || len <= 0 )
    {
        strcpy( tt, "error" );
        return;
    }

    ofs[0] +=( cur_table->PhraseNum + 1)<< 2;
 /* Add the index area length */
    fp     =(char*)cur_table->PhraseFile;
    fp    +=ofs[0];
    memcpy (tt, fp, len);

    tt[len] = 0;
}

static void putstr (unsigned char *p)
{
    int index,len =strlen(( char*)p);
//    printf ("++++++%s\n", p);
    if (InputCount <= InputMatch)  /* All Match */
    {
        index = (int)p[len-2] * 256 + p[len-1];
        ClrIn();

        if (UseAssociateMode)
        {
            FindAssociateKey(index);
            CurrentPageIndex = StartKey;
            MultiPageMode = 0;
            FillAssociateChars(StartKey);

            if (CurSelNum > 0)
            {
                IsAssociateMode = 1;
            }
        }
    }else
    {
        int nCount = InputCount - InputMatch,nMatch = InputMatch,i;
        MultiPageMode = NextPageIndex = CurrentPageIndex = 0;
        InputCount = InputMatch = 0;

        for (i =0; i <nCount; i++)
            save_InpKey[i] =InpKey[nMatch+i];

        memset (InpKey, 0, sizeof(InpKey));

        for (i =1; i <=nCount; i++)  /* feed the additional keys */
        {
            InpKey[InputCount] =save_InpKey[InputCount];
            InputCount++;

            if (InputCount <=InputMatch+1)
            {
                FindMatchKey ();
                MultiPageMode    =0;
                CurrentPageIndex =StartKey;
                FillMatchChars (StartKey);
            }

        }

        if (InputMatch ==0)    /* left key has no match, delete */
        {
            ClrIn();
            return;
        }

    }
}


/* After add/delete a char, search the matched char/phrase, update the
   StartKey/EndKey key,  save the related keys at first, if no match
   is found, we may restore its original value
*/
static void FindMatchKey (void)
{
//    save_StartKey           = StartKey;
//    save_EndKey           = EndKey;
//    save_MultiPageMode    = MultiPageMode;
//    save_NextPageIndex    = NextPageIndex;
//    save_CurrentPageIndex = CurrentPageIndex;

    save_StartKey           = 0;
    save_EndKey           = 0;
    save_MultiPageMode    = 0;
    save_NextPageIndex    = 0;
    save_CurrentPageIndex = 0;

    val1 = InpKey[4] | (InpKey[3]<<6) | (InpKey[2]<<12) | (InpKey[1]<<18) | (InpKey[0]<<24);
    val2 = InpKey[9] | (InpKey[8]<<6) | (InpKey[7]<<12) | (InpKey[6]<<18) | (InpKey[5]<<24);

    if (InputCount == 1)
        StartKey = cur_table->KeyIndex[InpKey[0]];
    else
        StartKey = CharIndex[InputCount-1];

    EndKey = cur_table->KeyIndex[InpKey[0]+1];

    for (; StartKey <EndKey; StartKey++)
    {
        key1 = (cur_table->item[StartKey].key1 & mask[InputCount+5]);
        key2 = (cur_table->item[StartKey].key2 & mask[InputCount]);

        if (key1 > val1) break;
        if (key1 < val1) continue;
        if (key2 < val2) continue;
        break;
    }
    CharIndex[InputCount] = StartKey;
}

/*  Find the matched chars/phrases and fill it into SelTab
    The starting checked index is j

    The Selection Line 1xxx 2xxx,  80-20=60 60/2=30 chinese chars only
    0-9 Selection can contain only 30 chinese chars
*/

static void FillAssociateChars (int index)
{
    unsigned char str[25];
    int PhraseNo, CurLen = 0;
    char *fp =(char*)cur_table->AssocFile;

    CurSelNum = 0;

    while ( CurSelNum < cur_table->MaxDupSel && index < EndKey && CurLen < MAX_SEL_LENGTH)
    {
        fp =(char*)cur_table->AssocFile;
        fp +=(index<<2);
        memcpy (&PhraseNo, fp, sizeof (int));
#if MGUI_BYTEORDER == MGUI_BIG_ENDIAN
        PhraseNo = ArchSwap32(PhraseNo);
#endif
        load_phrase( PhraseNo, (char *)str );
        strcpy(seltab[CurSelNum],(char *)(str+2));
        CurLen += strlen(seltab[CurSelNum++]);
        index++;
    }

    /* check if more than one page */
    if ( index < EndKey && CurSelNum == cur_table->MaxDupSel )
    {
      /* has another matched key, so enter MultiPageMode, has more pages */
        NextPageIndex = index;
        MultiPageMode = 1;
    }
    else if (MultiPageMode)
    {
        NextPageIndex = StartKey; /* rotate selection */
    }
    else
        MultiPageMode = 0;
    }

int pinyin_predict_pord(void *method, const char *lookfor, char * buffer, int buffer_len, int index)
{
    int    lenth = 0;
    static char lookfor_bk[4];


    lenth =0;
    {
        sprintf (lookfor_bk, "%s", lookfor);
        bzero (seltab, 16*MAX_PHRASE_LENGTH);
        putstr ((unsigned char *)lookfor);
    }

    buffer[0] = 0;
    while (1)
    {
        FillAssociateChars (StartKey+index/10 *10);

        lenth +=(strlen (seltab[index])+1);
        if (0==strlen (seltab[index]))
            return -1;

        if (lenth>=buffer_len )
            return index;
        strcat (buffer, seltab[index]);
        strcat (buffer, " ");
        index++;
    }
    return -1;


}
static void FillMatchChars (int j)
{
    int SelNum = 0, CurLen = 0;
    //bzero( seltab, sizeof( seltab ) );

    while ((cur_table->item[j].key1 & mask[InputCount+5])==val1&&
           (cur_table->item[j].key2 & mask[InputCount])==val2&&
           SelNum <cur_table->MaxDupSel&&j<EndKey&&
           CurLen < MAX_SEL_LENGTH
          )
    {
        unsigned short ch = cur_table->item[j].ch;
#if MGUI_BYTEORDER == MGUI_BIG_ENDIAN
        ch = ArchSwap16 (ch);
#endif

        if (ch < 0xA1A1)
            load_phrase (ch, seltab[SelNum]);
        else
        {
            memcpy (&seltab[SelNum], &(cur_table->item[j].ch), 2);
            seltab [SelNum][2] = '\0';
        }

        CurLen += strlen(seltab[SelNum++]);
        j++;
    }

    if (SelNum == 0)  /* some match found */
    {
        StartKey = save_StartKey;
        EndKey = save_EndKey;
        MultiPageMode = save_MultiPageMode;
        NextPageIndex = save_NextPageIndex;
        CurrentPageIndex = save_CurrentPageIndex;
        return;    /* keep the original selection */
    }

    CurSelNum = SelNum;

    for (SelNum = CurSelNum; SelNum < 16; SelNum++)
        seltab[SelNum][0] = '\0';  /* zero out the unused area */

    InputMatch = InputCount; /* until now we have some matches */

    /* check if more than one page */
    if (j < EndKey && (cur_table->item[j].key1 & mask[InputCount+5]) == val1 &&
        (cur_table->item[j].key2 & mask[InputCount] ) == val2 &&
        CurSelNum == cur_table->MaxDupSel
       )
    {
      /* has another matched key, so enter MultiPageMode, has more pages */
        NextPageIndex = j;
        MultiPageMode = 1;
    }
    else if (MultiPageMode)
    {
        NextPageIndex = StartKey; /* rotate selection */
    }
    else
        MultiPageMode = 0;
}

int pinyin_match_keystokes (void *method, const char* keystokes, char* buff, int buffer_len, int cursor);
/* return value: Has output? */

BOOL char_match_exist (const char *match)
{
    int len = strlen (match);
    int idex = 0;
    int key = 0;

    bzero (InpKey, sizeof (long)*InputCount);
    FindMatchKey ();
    ClrIn();

    for (idex=0; idex<len; idex++)
    {
       key =(char)(cur_table->KeyMap[(int)(*(match+idex))]);
       InpKey[InputCount++] = key;
    }

    FindMatchKey();
    CurrentPageIndex = StartKey;
    MultiPageMode    = 1;
    FillMatchChars(StartKey);

    if (EndKey== 0)
    {
       return FALSE;
    }
    else
    {
       return TRUE;
    }

}
int pinyin_translate_word(void *method, const char *strokes, char *buffer, int buffer_len, int index)
{
    static char strokes_bk[32];
    static int  length = 0;
    static int  page = 0;
    int         idex, key;
    int         lensum;
    int         len;

    if(index < 0)
        return -1;

    len = strlen(strokes);
    if (len != length || 0!=bcmp (strokes_bk, strokes, length)||WORK_TIM || page!=index/10)
    {
        sprintf (strokes_bk, "%s", strokes);
        WORK_TIM = FALSE;
        page = 0;
        length = len;
        ClrIn ();

        for (idex = 0; idex<length && idex < MAX_INPUT_LENGTH ; idex++)
        {
            key = cur_table->KeyMap[(int)(*(strokes+idex))];
            InpKey [InputCount++] = key;
        }

        FindMatchKey ();
        CurrentPageIndex = StartKey;
        MultiPageMode    = 1;
        FillMatchChars(StartKey);
        if (InputMatch<InputCount)
               return -1;
    }

    lensum  =0;
    buffer[0] =0;
    MultiPageMode =1;

    while (MultiPageMode)
    {
        if (page!=index/10)
        {
            page= index/10;
            FillMatchChars (StartKey+page*10);
        }

        if (InputMatch<InputCount)
               return -1;
        lensum+=(strlen(seltab[index%10])+1);

        if (lensum>buffer_len)
           return index;
        if (0==strlen (seltab[index%10]))
            break;
        strcat (buffer, seltab[index%10]);
        strcat (buffer, " ");
        index++;

    }
   return -1;

}

int pinyin_match_keystokes (void *method, const char* keystokes, char* buff, int buffer_len, int cursor)
{
    static int  len = 0;
    static char match_buff[2][100][20];
    static int  match_idex = 0;
    static int  max = 1;
    int    idex, idex1, idex2, idex3;
    char   *keys;


    if (cursor == -1)
        return -1;

    WORK_TIM = TRUE;
    len =strlen (keystokes);
    bzero (match_buff, 2*100*20);

    idex2 = 0;
    idex3 = 0;
    match_idex = 0;
    max = 1;

    for (idex = 0; idex < len; idex++)
    {
        keys = key_map[*(keystokes+idex)-'2'];

        for (idex1 = 0; idex1<max; idex1++)
        {
            for (idex2 = 0; idex2<strlen (keys); idex2++)
            {
                sprintf (match_buff[(match_idex+1)%2][idex3], "%s%c", match_buff[match_idex][idex1], *(keys+idex2));
                if (char_match_exist (match_buff[(match_idex+1)%2][idex3]))
                    idex3++;
            }
        }
    max   = idex3;
    idex3 = 0;
    match_idex = (match_idex+1)%2;
    }


    {
        int lensum =0;
        buff[0]    =0;

        while (cursor<max)
        {
            lensum+=(strlen (match_buff[match_idex][cursor++])+1);
            if (lensum>buffer_len)
            {
                return (cursor-1);
            }
            strcat (buff, match_buff[match_idex][cursor-1]);
            strcat (buff, " ");
        }
        return -1;
    }
    return -1;

}

BOOL ime_pinyin_init (void)
{
    CurIME          = 1;
    input_table[0]  = IntCode_Init();
    input_table [1] = load_input_method();
    cur_table       = input_table [1];
    return TRUE;
}


static ime_input_table *IntCode_Init (void)
{
    ime_input_table *table;
    static
    ime_input_table my_table;
    int i,index;

    UseAssociateMode = 1;  /* force to no associate */
    table            = &my_table;

   /* reset to zero. */
    memset (table, 0, sizeof (ime_input_table));
    strcpy(table->magic_number,MAGIC_NUMBER);
    strcpy(table->ename, "IntCode");
    strcpy(table->cname, "¡¾ÄÚÂë¡¿");
    strcpy(table->selkey, "0123456789abcdef");
    table->last_full = 1;

    for(i = 0; i < 128; i++)
    {
        table->KeyMap[i] = 0;

        if ((i >= '0' && i <= '9') || (i >= 'a' && i <= 'f'))
        {
            if (i >= '0' && i <= '9')
                index = i - '0';
            else index = i -'a' + 10;
                table->KeyMap[i] = index;
            table->KeyName[index] = toupper(i);
        }
    }
    return table;
}
