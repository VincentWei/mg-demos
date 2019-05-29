/*
** This file is a part of mg-demos package.
**
** Copyright (C) 2008 ~ 2019 FMSoft (http://www.fmsoft.cn).
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

#ifndef __ANIMATE_H__
#define __ANIMATE_H__

#include <minigui/common.h>
#include <minigui/fixedmath.h>

#include "p-code.h"

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

typedef struct _ANIMATE ANIMATE;

typedef void (*DRAW_ANIMATE) (HDC hdc, ANIMATE* ani );

#define DEF_DRAW_ANIMATE NULL
#define DEF_ALPHA_DRAW_BITMAP (DRAW_ANIMATE)(1)
#define DEF_MEMDC_DRAW  (DRAW_ANIMATE)(2)
#define DEF_WINDOW_DRAW (DRAW_ANIMATE)(3)


struct _ANIMATE{
    void *img;
    int x;
    int y;
    int w;
    int h;
    int alpha;
    int visible;
    void *param;
    ANIMATE* next, *prev;
};

#define GetAnimateX(a)  (((a)->x)>>16)
#define GetAnimateY(a)  (((a)->y)>>16)
#define GetAnimateW(a)  (((a)->w)>>16)
#define GetAnimateH(a)  (((a)->h)>>16)
//#define GetAnimateA(a)  (((a)->alpha)>>16)
#define GetAnimateA(a)  (((a)->alpha))
#define SetAnimateX(a,v) ((a)->x=(v)<<16)
#define SetAnimateY(a,v) ((a)->y=(v)<<16)
#define SetAnimateW(a,v) ((a)->w=(v)<<16)
#define SetAnimateH(a,v) ((a)->h=(v)<<16)
//#define SetAnimateA(a,v) ((a)->alpha=(v)<<16)
#define SetAnimateA(a,v) ((a)->alpha=(v))


enum timelineMode{
    tlmExec = 0,
    tlmRun,
    tlmWait,
    tlmLoop
};

enum timelineState{
    tlsReady = 0,
    tlsRun,
    tlsPaused,
    tlsEnd
};

#define INVALID_TIME_LINE_ID -1
typedef struct _TIME_LINE {
    intptr_t id;
    PCODE_ENV *env;
    int frame_num:16;
    char mode;
    char state;
    void * steplist;
    struct _TIME_LINE* next;
} TIME_LINE;

void ExecuteTimeLine(TIME_LINE* tl);

TIME_LINE *CreateTimeLine(intptr_t id);
void DeleteTimeLine(TIME_LINE* tl);
BOOL SetTimeLineMethod(TIME_LINE* tl, PCODE_METHOD* pm, int stack_size, int *args);

BOOL LoadAnimateFile(const char* file);

///////////////////////////////////////////////////////
//animate sence

enum animateSenceMode{
    asmBlock = 0,
    asmThread,
    asmTimer
};

enum animateSenceState{
    assReady = 0,
    assRunning,
    assPaused,
    assFinished,
    assStoped
};

typedef struct _ANIMATE_SENCE ANIMATE_SENCE;

struct _ANIMATE_SENCE{

    DRAW_ANIMATE drawAnimate;

    ANIMATE * normal;
    ANIMATE * topmost;

    TIME_LINE* timelines;

    int interval;
    int total_time;

    int mode:16;
    int state:16;

    HDC hdc;
    RECT rtArea;

    void (*draw_bkgnd)(HDC hdc, const RECT* rtbk, void *param);
    void (*on_finished)(ANIMATE_SENCE * as);
    void (*before_draw_one_frame)(ANIMATE_SENCE* as);
    void (*end_draw_one_frame)(ANIMATE_SENCE* as);

    pthread_cond_t cond;
    pthread_mutex_t mutex;
    union{
        pthread_t _pthread; //used for thread mode
        DWORD _old_time; //used for time mode
    }u;
    #define pthread u._pthread
    #define old_time u._old_time
    #define use_thread_obj u._use_thread_obj

    DWORD start_time;

    void * param;
};

#define DrawAnimate(as, a) (as)->drawAnimate(as->hdc, a)



#define AS_LOCK(as) \
    if((as)->state == assRunning && ((as)->mode == asmBlock || (as)->mode == asmThread)) pthread_mutex_lock(&(as)->mutex)

#define AS_TRY_LOCK(as) \
    (((as)->state == assRunning && ((as)->mode == asmBlock || (as)->mode == asmThread))? (pthread_mutex_trylock(&(as)->mutex) ==0 ):TRUE)



#define AS_UNLOCK(as) \
    if((as)->state == assRunning && ((as)->mode == asmBlock || (as)->mode == asmThread)) pthread_mutex_unlock(&(as)->mutex)


/*if all_times==-1, no stop*/
ANIMATE_SENCE* CreateAnimateSence (int interval, int all_times, HDC hdc,
        DRAW_ANIMATE drawAnimate,
        void (*drawbkgnd)(HDC, const RECT* , void*),
        void (*onFinished)(ANIMATE_SENCE *),
        const RECT* bkgnd, void* user_data);

BOOL InitAnimateSence(ANIMATE_SENCE* as, int interval, int all_times, HDC hdc,
    DRAW_ANIMATE drawAnimate,
    void (*drawbkgnd)(HDC, const RECT*, void *),
    void (*onFinished)(ANIMATE_SENCE *),
    const RECT* bkgnd, void * user_data);

void DeleteAnimateSence (ANIMATE_SENCE* as, void (*free_data)(void*));
void DeinitAnimateSence(ANIMATE_SENCE* as, void(*free_data)(void*));

BOOL InsertAnimate (ANIMATE_SENCE* as, ANIMATE* a, BOOL is_topmost);
BOOL RemoveAnimate (ANIMATE_SENCE* as, ANIMATE* a);
BOOL MoveAnimateToTopMost (ANIMATE_SENCE* as, ANIMATE*a);
BOOL RemoveAnimateFromTopMost (ANIMATE_SENCE* as, ANIMATE*a);

int StartAnimateSence (ANIMATE_SENCE* as);
int StartThreadAnimateSence (ANIMATE_SENCE* as);
int StartTimerAnimateSence (ANIMATE_SENCE *as);

BOOL StopAnimateSence (ANIMATE_SENCE *as);
BOOL PauseAnimateSence (ANIMATE_SENCE *as);

BOOL ContinueAnimateSence (ANIMATE_SENCE* as);

PCODE_METHOD* GetTimeLineMethod(const char* strName);

BOOL StartTimeLine(ANIMATE_SENCE* as, TIME_LINE* tl, PCODE_METHOD* method, int stack_size, int* args);
BOOL StopTimeLine(ANIMATE_SENCE* as, TIME_LINE* tl);
BOOL PauseTimeLine(ANIMATE_SENCE* as, TIME_LINE* tl);
BOOL ContinueTimeLine(ANIMATE_SENCE* as, TIME_LINE* tl);

TIME_LINE * GetTimeLineByID(ANIMATE_SENCE* as, int id);

void TLMoveTo(TIME_LINE* tl, ANIMATE* a, int x1, int y1, int frame_num);
void TLScaleTo(TIME_LINE* tl, ANIMATE* a, int w1, int h1, int frame_num);
void TLAlphaTo(TIME_LINE* tl, ANIMATE* a, int a1,int frame_num);
void TLRun(TIME_LINE* tl, int frame_num);
void TLWait(TIME_LINE* tl, int frame_num);
void TLLoop(TIME_LINE* tl);

enum tlStepType{
    tlstMove = 0x01,
    tlstScale = 0x02,
    tlstAlpha = 0x04
};
void TLStopAnimateStep(TIME_LINE* tl, ANIMATE* a, int type,BOOL finish_animate);

void SetNativeCallback(PCODE_NATIVE_METHOD_TABLE *pnt);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif
