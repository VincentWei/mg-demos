#ifndef __INSTRUCTION_H__
#define __INSTRUCTION_H__

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

#define FRAME_INSTRCT   0
#define MOVE_INSTRCT    1
#define SCALE_INSTRCT   2
#define ALPHA_INSTRCT   3
#define GOTO_INSTRCT    4
#define LOOP_INSTRCT    5
#define BLANK_INSTRCT   6

#define INSTRCT_KIND_NUM    7

#define INVALID_INSTRCT 0xFF

#define FRAME_NUM_INVALID   0xFF
#define FRAME_NUM_NOSTOP    0xFE

#define FRAME_INSTRCT_SIZE   5
#define MOVE_INSTRCT_SIZE    10 
#define SCALE_INSTRCT_SIZE   10
#define ALPHA_INSTRCT_SIZE   6 
#define GOTO_INSTRCT_SIZE    3
#define LOOP_INSTRCT_SIZE    1
#define BLANK_INSTRCT_SIZE   2

#define REAL_INSTRCT(ins) ((ins) & 0x7F)

#define VALUE_KEEP_OLD  0x7FFF

#define MIN_SWORD  -32768
#define MAX_SWORD   32766 

#define MIN_BYTE   0 
#define MAX_BYTE   255

#define GET_BYTE(cp) ((BYTE)(*(cp)++))

#define SAVE_BYTE(cp, byte)  (*(cp)++ = (BYTE)(byte))
#define STORE_BYTE(cp, byte)  (byte = GET_BYTE(cp))

#define SAVE_INSTRCT(cp, instrct, is_index) \
    (*(cp)++ = (instrct) | ((is_index)?0x80:0 ) )

#define STORE_INSTRCT(cp, instrct, is_index) \
{ \
    instrct = GET_BYTE(cp); \
    is_index = instrct & 0x80; \
    instrct &= 0x7F; \
}

#define SAVE_WORD(cp, word) \
{ \
    *(cp)++ = (word) & 0xFF; \
    *(cp)++ = (((WORD)(word))>>8) & 0xFF; \
}


#define STORE_WORD(cp, word) \
{ \
    (word) = GET_BYTE(cp); \
    (word) |= GET_BYTE(cp) << 8; \
}

#define SAVE_DWORD(cp, dword) \
{ \
    *(cp)++ = ((DWORD)(dword)) & 0xFF; \
    *(cp)++ = ((DWORD)(dword)>>8) & 0xFF; \
    *(cp)++ = ((DWORD)(dword)>>16) & 0xFF; \
    *(cp)++ = ((DWORD)(dword)>>24) & 0xFF; \
}

#define STORE_DWORD(cp, dword) \
{ \
    (dword) = GET_BYTE(cp); \
    (dword) |= GET_BYTE(cp) << 8; \
    (dword) |= GET_BYTE(cp) << 16; \
    (dword) |= GET_BYTE(cp) << 24; \
}

typedef struct {
    char id;
    BOOL is_index;
    unsigned char frame_num;
    union {
        struct {
            void* img;
        } frame_param;

        struct {
            short x0;
            short y0;
            short x1;
            short y1;
        } move_param;

        struct {
            short w0;
            short h0;
            short w1;
            short h1;
        } scale_param;

        struct {
            unsigned short alpha0;
            unsigned short alpha1;
        } alpha_param;

        struct {
            short ins_pos;
        } goto_param;

    } u;
} instrct_t;

int anim_get_instrct_size (char instrct);
int anim_parse_instrct (char* ins_str, instrct_t* instrct);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif
