/*!============================================================================
 * @file misc.h 
 * @Synopsis Header for misc interfaces.
 * @author Vincent Wei
 * @version 1.0
 *
 * Copyright (C) 2018 Beijing Joobot Technologies Inc.
 * All rights reserved.
 */

#ifndef  __MISC_H
#define  __MISC_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * The task_proc will be called after the logo GIF animation played.
 */
typedef void (*ASYNC_TASK_PROC) (HWND);
void splashScreen (HWND hosting, ASYNC_TASK_PROC task_proc);

void softKeyboard (HWND hosting, char* buff, int len_min, int len_max);

void screenLock (HWND hosting, BOOL reset_passwd);

#define MS_AUTOCANCEL        0x10000000
/*
 * messageScreen runs like MiniGUI's MessageBox, but only the following styles supported:
 *
 *  - MB_OK
 *      Create a message screen within only one button, which acts as "Ok".
 *  - MB_OKCANCEL
 *      Create a message screen within two buttons which act as "OK" and "Cancel" respectively.
 *  - MB_RETRYCANCEL
 *      Create a message screen within two buttons which act as "Retry" and "Cancel" respectively.
 * 
 * Or'd the following flags if you want to enable auto cancel if there is no confirm in 5 seconds:
 *  - MB_AUTOCANCEL
 */
int messageScreen (HWND hosting, const char* title, const char* text, DWORD style);

/*
 * toastWindow shows a message in a window and automatically hides itself after some time (1.0s).
 */
HWND createToastWindow (HWND hosting);
BOOL destroyToastWindow (void);

int showToastWindow (const char* title, const char* text, DWORD color, BOOL autoHide);
void hideToastWindow (void);

#ifdef __cplusplus
}
#endif

#endif   /* __MISC_H */

