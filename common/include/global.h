/*!============================================================================
 * @file global.h 
 * @Synopsis  
 * @author DongKai
 * @version 1.0
 *  Company: Beijing FMSoft Technologies Co., Ltd.
 */

#ifndef  __GLOBAL_INC
#define  __GLOBAL_INC

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <sys/times.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#include <mgeff/mgeff.h>
#include <mgncs/mgncs.h>
#include <mgncs4touch/mgncs4touch.h>

// Intent actions
#define ACTION_MAIN     0       // main action (generally means starting from the desktop)
#define ACTION_VIEW     1       // view action (view from other APP)
#define ACTION_EDIT     2       // edit action (edit from other APP)

#define MSG_USER_APP_START          (MSG_USER + 700)
#define MSG_USER_APP_STOP           (MSG_USER + 701)
#define MSG_USER_RETURN_TO_DESKTOP  (MSG_USER + 702)
#define MSG_USER_RETURN_KEY_PRESSED (MSG_USER + 703)
#define MSG_USER_ITEM_CLICKED       (MSG_USER + 704)

#define MSG_USER_APP_DATA           (MSG_USER + 705)
#define MSG_USER_APP_READY          (MSG_USER + 706)
    // wParam of MSG_USER_APP_READY message
    #define ACTIVITY_PUSHED         0
    #define ACTIVITY_POPPED         1

#define MSG_USER_IDLE	            (MSG_USER + 707)
#define MSG_USER_CLICK	            (MSG_USER + 708)
#define MSG_USER_BACK	            (MSG_USER + 709)
#define MSG_USER_RETURN_TO_STANDBY  (MSG_USER + 710)
#define MSG_USER_STATEMACHINE_EVENT (MSG_USER + 711)

#define MSG_ASYNC_TASK_FINISHED     (MSG_USER + 1000)
#define MSG_ASYNC_TASK_CANCELED     (MSG_USER + 1001)

#ifndef BITBLT
    #define BITBLT(shdc, sx, sy, sw, sh, dhdc, dx, dy, rop) \
    do { \
        if (0 != sw && 0 != sh) BitBlt(shdc, sx, sy, sw, sh, dhdc, dx, dy, rop); \
    } while(0)
#endif

#ifndef STRETCHBLT
    #define STRETCHBLT(shdc, sx, sy, sw, sh, dhdc, dx, dy, dw, dh, rop) \
    do { \
        if (0 != sw && 0 != sh && 0 != dw && 0 != dh)   \
        StretchBlt(shdc, sx, sy, sw, sh, dhdc, dx, dy, dw, dh, rop); \
    } while(0)
#endif

#ifdef __cplusplus
extern "C" {
#endif

const char* mygettext (const char* msgid);

#ifdef __cplusplus
}
#endif

#define _(string) mygettext(string)
#define N_(string) string

#endif   /* ----- #ifndef __GLOBAL_INC  ----- */

