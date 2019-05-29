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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef KBD_ANIMATE

#include "p-code.h"
#include "animate.h"

//define the steplist info
typedef struct _tl_step_t{
    struct _tl_step_t *next;
    int type:16;
    int frame_left:16;
    ANIMATE* obj;
}tl_step_t;

#define STEP_HEAD  struct _tl_step_t *next; \
    int type:16; \
    int frame_left:16; \
    ANIMATE* obj;

typedef struct _tl_move_step_t{
    STEP_HEAD
    int stepx;
    int stepy;
}tl_move_step_t;

typedef struct _tl_scale_step_t{
    STEP_HEAD
    int stepw;
    int steph;
}tl_scale_step_t;

typedef struct _tl_alpha_step_t{
    STEP_HEAD
    int stepa;
}tl_alpha_step_t;


//define the timeline expend instructons
enum tlInstruction{
    tliSetLocation = max_pre_def_ins,
    /*
     * desc: set location of animate object
     * stack before: ..., ani, x_value, y_value
     * stack after: ...,
     *
     */
    tliSetSize,
    /*
     * desc: set size of animate object
     * stack before: ..., ani, w_value, h_value
     * stack after: ...,
     *
     */
    tliSetAlpha,
    /*
     * desc: set alpha of animate object
     * stack before: ..., ani, alpha_value
     * stack afeter: ...
     */
    tliSetImg,
    /*
     * desc: set image of animate object
     * stack before: ..., ani, img
     * stack after: ...
     */
    tliSetVisible,
    /*
     * desc: set visible of naimate object
     * stack before: ..., ani, visible
     * stack after: ...
     */
    tliMoveTo,
    /*
     * desc: calculate the stepx and stepy, and append tl_move_step_t object into timeline
     * statck before: ..., ani, x1_value, y1_value, frame_num,
     * stack after: ...
     */
    tliScaleTo,
    /*
     * desc: caculate the stepw and steph, and append tl_scale_step_t object into timeline
     *  stack before: ..., ani, w1_value, h1_value, frame_num,
     *  stack after: ...
     */
    tliAlphaTo,
    /*
     *  see also tliMoveTo and tliScaleTo
     */
    tliRun,
    /*
     * desc: run the step setting
     * stack before: ..., frame_num
     * stack after: ...
     */
    tliWait,
    /*
     * desc: wait serveral frames
     * stack before: ..., frame_num
     * stack after: ...
     */
    tliLoop
};

#define AppendIns(pm,ins) AppendValue(pm,"c", ins)

//define the timeline struction implemention
static int tlSetLocation(unsigned char* ins, intptr_t** stack,  void *param)
{
    int x_value;
    int y_value;
    ANIMATE* obj;
    y_value = (int)POP(*stack);
    x_value = (int)POP(*stack);
    obj = (ANIMATE*)POP(*stack);

    obj->x = x_value<<16;
    obj->y = y_value<<16;

    return 1;
}

static int tlSetSize(unsigned char* ins, intptr_t** stack, void *param)
{
    int h_value;
    int w_value;
    ANIMATE* obj;
    w_value = (int)POP(*stack);
    h_value = (int)POP(*stack);
    obj = (ANIMATE*)POP(*stack);

    obj->w = w_value<<16;
    obj->h = h_value<<16;

    return 1;
}

static int tlSetVisible(unsigned char* ins, intptr_t ** stack, void* param)
{
    int v_value;
    ANIMATE* obj;

    v_value = (int)POP(*stack);
    obj = (ANIMATE*)POP(*stack);

    obj->visible = v_value;
    return 1;
}

static int tlSetImage(unsigned char* ins, intptr_t ** stack, void* param)
{
    void* img;
    ANIMATE* obj;

    img = (void*)POP(stack);
    obj = (ANIMATE*)POP(stack);

    obj->img = img;
    return 1;
}

static int tlSetAlpha(unsigned char* ins, intptr_t ** stack, void* param)
{
    ANIMATE* obj;
    int alpha;

    alpha = (int)POP(*stack);
    obj = (ANIMATE*)POP(*stack);

    obj->alpha = alpha<<16;
    return 1;
}

static tl_step_t * get_or_new_step(TIME_LINE* tl, ANIMATE* a, int type, int size)
{
    tl_step_t *ts = (tl_step_t*)tl->steplist;

    while(ts){
        if(ts->obj == a && ts->type == type)
            return ts;
        ts = ts->next;
    }

    ts = (tl_step_t*)calloc(1,size);
    ts->obj = a;
    ts->type = type;
    ts->next = (tl_step_t*)tl->steplist;
    tl->steplist = ts;
    return ts;
}

#define GET_NEW_STEP(tl_step_type, tl, a, type) \
    (tl_step_type*) get_or_new_step(tl, a, type, sizeof(tl_step_type))

static int tlMoveTo(unsigned char* ins, intptr_t** stack, void *param)
{
    TIME_LINE* tl = (TIME_LINE*)param;
    int x1, y1, frame_num;
    ANIMATE* obj;
    tl_move_step_t *tms;

    frame_num = POP(*stack);
    y1 = POP(*stack);
    x1 = POP(*stack);
    obj = (ANIMATE*)POP(*stack);

    tms = GET_NEW_STEP(tl_move_step_t, tl, obj, tlstMove);

    if(frame_num <= 0)
        frame_num = 1;
    //set tms
    tms->frame_left = frame_num;
    tms->stepx = ((x1<<16) - obj->x) / frame_num;
    tms->stepy = ((y1<<16) - obj->y) / frame_num;

    return 1;
}

static int tlScaleTo(unsigned char* ins, intptr_t** stack,  void *param)
{
    TIME_LINE* tl = (TIME_LINE*)param;
    int w1, h1, frame_num;
    ANIMATE* obj;
    tl_scale_step_t *tms;

    frame_num = POP(*stack);
    h1 = POP(*stack);
    w1 = POP(*stack);
    obj = (ANIMATE*)POP(*stack);

    tms = GET_NEW_STEP(tl_scale_step_t, tl, obj, tlstScale);

    if(frame_num <= 0)
        frame_num = 1;
    //set tms
    tms->frame_left = frame_num;
    tms->stepw = ((w1<<16)-obj->w) / frame_num;
    tms->steph = ((h1<<16)-obj->h) / frame_num;

    return 1;
}

static int tlAlphaTo(unsigned char* ins, intptr_t** stack, void *param)
{
    TIME_LINE* tl = (TIME_LINE*)param;
    int a1, frame_num;
    ANIMATE* obj;
    tl_alpha_step_t *tms;

    frame_num = POP(*stack);
    a1 = POP(*stack);
    obj = (ANIMATE*)POP(*stack);

    tms = GET_NEW_STEP(tl_alpha_step_t, tl, obj, tlstAlpha);

    if(frame_num <= 0)
        frame_num = 1;
    //set tms
    tms->frame_left = frame_num;
    //tms->stepa = ((a1<<16)-obj->alpha) / frame_num;
    tms->stepa = (((a1)-obj->alpha) / frame_num);

    return 1;
}

static int tlRun(unsigned char* ip, intptr_t** stack, void* param)
{
    TIME_LINE* tl = (TIME_LINE*)param;

    tl->frame_num = (int)POP(*stack);
    tl->mode = tlmRun;
    return 1;
}

static int tlWait(unsigned char* ip, intptr_t** stack,void* param)
{
    TIME_LINE* tl = (TIME_LINE*)param;

    tl->frame_num = (int)POP(*stack);
    tl->mode = tlmWait;
    return 1;
}

static int tlLoop(unsigned char* ip, intptr_t** stack, void* param)
{
    TIME_LINE* tl = (TIME_LINE*)param;

    tl->mode = tlmLoop;
    return 1;
}

static DO_USER_INSTR tlInstrctionEntries[] = {
    /*tliSetLocaiton, */tlSetLocation,
    /*tliSetSize, */tlSetSize,
    /*tliSetAlpha, */tlSetAlpha,
    /*tliSetImg, */tlSetImage,
    /*tliSetVisible, */tlSetVisible,
    /*tliMoveTo, */tlMoveTo,
    /*tliScaleTo, */tlScaleTo,
    /*tliAlphaTo, */tlAlphaTo,
    /*tliRun,*/ tlRun,
    /*tliWait, */tlWait,
    /*tliLoop, */tlLoop
};

static PCODE_USER_INSTR_TABLE tlInstrctionTable = {
    tlInstrctionEntries,
    sizeof(tlInstrctionEntries)/sizeof(DO_USER_INSTR)
};

static PCODE tlPcode = {
    &tlInstrctionTable,
    NULL,
    NULL,
    NULL
};

void SetNativeCallback(PCODE_NATIVE_METHOD_TABLE *pnt){
    tlPcode.native_table = pnt;
}

TIME_LINE *CreateTimeLine(intptr_t id)
{
    TIME_LINE *tl = (TIME_LINE*)calloc(1,sizeof(TIME_LINE));

    tl->id = id;
    tl->mode = tlmExec;

    return tl;
}

void DeleteTimeLine(TIME_LINE* tl)
{
    TLStopAnimateStep(tl, NULL, tlstMove|tlstScale|tlstAlpha, TRUE);
    if(tl->env)
        DeletePCodeEnv(tl->env);
    free(tl);
}

static void tl_calculate_steplist(TIME_LINE* tl)
{
    tl_step_t *ts = (tl_step_t*)tl->steplist;
    tl_step_t *tsprev = ts;
    while(ts)
    {
        switch(ts->type)
        {
        case tlstMove:
        {
            tl_move_step_t* tms = (tl_move_step_t*)ts;
            // here do the step
            tms->obj->x += tms->stepx;
            tms->obj->y += tms->stepy;
            break;
        }
        case tlstScale:
        {
            tl_scale_step_t* tms = (tl_scale_step_t*)ts;
            tms->obj->w += tms->stepw;
            tms->obj->h += tms->steph;
            break;
        }
        case tlstAlpha:
        {
            tl_alpha_step_t* tms = (tl_alpha_step_t*)ts;
            // here do the step
            tms->obj->alpha += tms->stepa;
            //printf("--:%d: tl=%p, Alpha=%d, step=%d\n", tl, ts->obj, ts->obj->alpha, tms->stepa);
            break;
        }
        }
        ts->frame_left --;
        if(ts->frame_left <= 0)
        {
            tl_step_t *del = ts;
            //remove and free ts
            if(ts == tsprev){
                ts = ts->next;
                tsprev = ts;
                tl->steplist = ts;
            }
            else {
                ts = ts->next;
                tsprev->next = ts;
            }
            free(del);
        }
        else{
            tsprev = ts;
            ts = ts->next;
        }
    }

}

void ExecuteTimeLine(TIME_LINE* tl)
{
    int ret;
    if(tl->state != tlsRun)
        return ;

    if(tl->mode == tlmLoop)
        return ;

    if(tl->frame_num > 0)
    {
        if(tl->mode == tlmRun)
            tl_calculate_steplist(tl);

        tl->frame_num --;
        if(tl->frame_num == 0){
            tl->mode = tlmExec;
            ret = ExecutePCode(&tlPcode, tl->env);
            if(ret == PCE_RET || ret == PCE_ERROR)
                tl->state = tlsEnd;
        }
    }
    else if(tlmExec == tl->mode)
    {
        int ret =  ExecutePCode(&tlPcode,tl->env);
        if(ret == PCE_RET || ret == PCE_ERROR)
        {
            //remove this tineline
            tl->state = tlsEnd;
        }
        else if(ret == PCE_INTERRUPT)
        {
            if(tl->mode == tlmRun)
                tl_calculate_steplist(tl);
            tl->frame_num --;
            if(tl->frame_num == 0)
                tl->mode = tlmExec;
        }
    }
}


BOOL SetTimeLineMethod(TIME_LINE* tl, PCODE_METHOD* pm, int stack_size, int *args)
{
    if(tl == NULL || pm == NULL)
        return FALSE;

    if(tl->env != NULL)
        DeletePCodeEnv(tl->env);

    tl->env = CreatePCodeEnv(pm, stack_size, args);
    tl->env->param = (void*)tl;
    tl->frame_num = 0;
    tl->mode = tlmExec;

    return TRUE;
}

typedef struct _pcode_method_list_t{
    struct _pcode_method_list_t *next;
    char* name;
    PCODE_METHOD* method;
}pcode_method_list_t;

static pcode_method_list_t* method_list = NULL;

static void insert_method(PCODE_METHOD* mth, const char* name, void* param)
{
    if(mth && name){
        pcode_method_list_t* ml = (pcode_method_list_t*)malloc(sizeof(pcode_method_list_t));
        ml->name = strdup(name);
        ml->method = mth;
        ml->next = method_list;
        method_list = ml;
    }
}

BOOL LoadAnimateFile(const char* file)
{
    PCODE_METHOD_TABLE * pmt = LoadPCodeMethodsFromFile(file, insert_method, NULL);
    if(pmt == NULL)
        return FALSE;

    tlPcode.method_table = pmt;
    return TRUE;
}

BOOL StartTimeLine(ANIMATE_SENCE* as, TIME_LINE* tl, PCODE_METHOD* method, int stack_size, int* args)
{
    if(as == NULL || tl == NULL)
        return FALSE;

    if(tl->state != tlsReady)
        return FALSE;

    if(method == NULL) {
        tl->env = NULL;
        tl->mode = tlmRun;
    }
    else {
        SetTimeLineMethod(tl, method, stack_size, args);
    }

    tl->state = tlsRun;

    AS_LOCK(as);

    if(tl->id == INVALID_TIME_LINE_ID || GetTimeLineByID(as, tl->id)==NULL){
        tl->next = as->timelines;
        as->timelines = tl;
    }

    AS_UNLOCK(as);

    return TRUE;
}

BOOL StopTimeLine(ANIMATE_SENCE* as, TIME_LINE* tl)
{
    TIME_LINE* tlprev;
    if(as == NULL || tl == NULL)
        return FALSE;

    AS_LOCK(as);
    if(tl == as->timelines){
        as->timelines = tl->next;
    }
    else {
        tlprev = as->timelines;
        while(tlprev && tlprev->next != tl){
        }
        if(tlprev){
            tlprev->next = tl->next;
        }
    }
    AS_UNLOCK(as);

    DeleteTimeLine(tl);
    return TRUE;
}

BOOL PauseTimeLine(ANIMATE_SENCE* as, TIME_LINE* tl)
{
    if(as == NULL || tl == NULL)
        return FALSE;

    if(tl->state != tlsRun)
        return TRUE;

    AS_LOCK(as);
    tl->state = tlsPaused;
    AS_UNLOCK(as);
    return TRUE;
}

BOOL ContinueTimeLine(ANIMATE_SENCE* as, TIME_LINE* tl)
{
    if(as == NULL || tl == NULL)
        return FALSE;

    if(tl->state != tlsPaused)
        return FALSE;

    AS_LOCK(as);
    tl->state = tlsRun;
    AS_UNLOCK(as);
    return TRUE;
}

TIME_LINE * GetTimeLineByID(ANIMATE_SENCE* as, int id)
{
    TIME_LINE* tl;
    if(as == NULL)
        return NULL;

    if(id == INVALID_TIME_LINE_ID)
        return NULL;

    tl = as->timelines;

    while(tl){
        if(tl->id == id)
            return tl;
        tl = tl->next;
    }

    return NULL;
}

PCODE_METHOD* GetTimeLineMethod(const char* strName)
{
    pcode_method_list_t* ml = method_list;
    while(ml){
        if(strcmp(ml->name, strName) == 0)
            return ml->method;
        ml = ml->next;
    }
    return NULL;
}

/////////////////////////////////////////////////////
//directly call
void TLMoveTo(TIME_LINE* tl, ANIMATE* a, int x1, int y1, int frame_num)
{
    intptr_t _stack [] ={
        (intptr_t)a,
        (intptr_t)x1,
        (intptr_t)y1,
        (intptr_t)frame_num
    };
    intptr_t *stack = _stack + sizeof(_stack)/sizeof(intptr_t);

    tlMoveTo(NULL, &stack, tl);
}

void TLScaleTo(TIME_LINE* tl, ANIMATE* a, int w1, int h1, int frame_num)
{
    intptr_t _stack [] ={
        (intptr_t)a,
        (intptr_t)w1,
        (intptr_t)h1,
        (intptr_t)frame_num
    };
    intptr_t *stack = _stack + sizeof(_stack)/sizeof(intptr_t);

    tlScaleTo(NULL, &stack, tl);

}
void TLAlphaTo(TIME_LINE* tl, ANIMATE* a, int a1,int frame_num)
{
    intptr_t _stack [] ={
        (intptr_t)a,
        (intptr_t)a1,
        (intptr_t)frame_num
    };
    intptr_t *stack = _stack + sizeof(_stack)/sizeof(intptr_t);

    tlAlphaTo(NULL, &stack, tl);

}

void TLRun(TIME_LINE* tl, int frame_num)
{
    intptr_t _stack[] = {
        (intptr_t)frame_num
    };
    intptr_t *stack = _stack + sizeof(_stack)/sizeof(intptr_t);
    tlRun(NULL,&stack,tl);
}

void TLWait(TIME_LINE* tl, int frame_num)
{
    intptr_t _stack[] = {
        (intptr_t)frame_num
    };
    intptr_t *stack = _stack + sizeof(_stack)/sizeof(intptr_t);
    tlWait(NULL,&stack,tl);
}

void TLLoop(TIME_LINE* tl)
{
    tlLoop(NULL,NULL,tl);
}

static void finished_step_animates(tl_step_t* ts)
{
    if(ts->frame_left == 0)
        return;

    switch(ts->type)
    {
    case tlstMove:
    {
        tl_move_step_t* tms = (tl_move_step_t*)ts;
        tms->obj->x += (tms->stepx * tms->frame_left);
        tms->obj->y += (tms->stepy * tms->frame_left);
        break;
    }
    case tlstScale:
    {
        tl_scale_step_t* tss = (tl_scale_step_t*)ts;
        tss->obj->w += (tss->stepw * tss->frame_left);
        tss->obj->h += (tss->steph * tss->frame_left);
        break;
    }
    case tlstAlpha:
    {
        tl_alpha_step_t* tas = (tl_alpha_step_t*)ts;
        tas->obj->alpha += (tas->stepa * tas->frame_left);
        break;
    }
    }
    ts->frame_left = 0;
}

void TLStopAnimateStep(TIME_LINE* tl, ANIMATE* a, int type,BOOL finish_animate)
{
    tl_step_t *ts, *tsprev;
    if(tl == NULL)
        return ;

    ts = (tl_step_t*)tl->steplist;
    tsprev = ts;
    while(ts){
        if((ts->type & type) && (a == NULL || (a == ts->obj)))
        {
            if(finish_animate)
                finished_step_animates(ts);
            tl_step_t * tsdel = ts;
            if(ts == tsprev){
                ts = ts->next;
                tsprev = ts;
                tl->steplist = ts;
            }
            else
            {
                ts = ts->next;
                tsprev->next = ts;
            }
            free(tsdel);
        }
        else
        {
            tsprev = ts;
            ts = ts->next;
        }
    }

}

#endif

