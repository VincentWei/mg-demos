/*
 * tooltip.h
 * tooltip for every key.
 *
 * wangjian<wangjian@minigui.org>
 * 2008-7-24.
 */

#ifndef __TOOLTIP_H__
#define __TOOLTIP_H__

#ifdef __cplusplus
extern "C" {
#endif

#define TTW_W           30
#define TTW_H           42

#define TTW_MASK_R      0xff
#define TTW_MASK_G      0x0
#define TTW_MASK_B      0x0

#define TTW_PAL_LEN     256

#define MSG_SETCHAR     MSG_USER + 10

typedef struct {
    /* bk gnd bitmap */
    BITMAP      bk_bmp;
    /* the logfont for showing text */
    PLOGFONT    pfont;
    /* mask for window shape */
    MYBITMAP    mask;
    /* color palette */
    RGB         pal[TTW_PAL_LEN];
} TTW_DATA, *TTW_PDATA;

HWND CreateToolTip(HWND host);
int ShowToolTip(HWND hwnd, int x, int y, char key_char);

inline static void HideToolTip(HWND hwnd)
{
    ShowWindow(hwnd, SW_HIDE);
}

#ifdef __cplusplus
}
#endif

#endif /* __TOOLTIP_H__ */

