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
#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>

#include "p-code.h"
#include "animate.h"

#ifdef KBD_ANIMATE

static HDC buf_dc;

static void draw_animate (HDC hdc, ANIMATE* ani)
{
    HDC memdc = (HDC)ani->img;
    SetMemDCAlpha(memdc, MEMDC_FLAG_SRCALPHA, GetAnimateA(ani));
    BitBlt(memdc, 0, 0, GetAnimateW(ani), GetAnimateH(ani), hdc,
            GetAnimateX(ani), GetAnimateY(ani), 0);
}

static void draw_bkgnd (HDC hdc, const RECT* rc, void* param)
{
    BitBlt ((HDC)param, 0, 0, RECTWP(rc), RECTHP(rc),
            hdc, rc->left, rc->top, 0);
}

static void end_draw_one_frame(ANIMATE_SENCE *as)
{
    if(as != NULL){
        BitBlt(as->hdc, 0, 0, 0, 0, HDC_SCREEN, 0, 0, 0);
    }
}

static void on_finished (ANIMATE_SENCE* as)
{
    if (as->param)
        DeleteCompatibleDC ((HDC)as->param);

    DeleteCompatibleDC ((HDC)as->normal->img);
    DeleteCompatibleDC (buf_dc);
    free (as->normal);
    as->normal = NULL;
}

void StartAlphaAnimate (int interval, int all_times, HDC hdc,
        const BITMAP* bitmap, const RECT* rc, int start_alpha, int end_alpha)
{
    TIME_LINE* tl;
    ANIMATE* a;
    ANIMATE_SENCE* as;
    int w = RECTWP(rc);
    int h = RECTHP(rc);
    HDC bkmemdc = CreateCompatibleDCEx (hdc, w, h);
    HDC fgmemdc = CreateCompatibleDCEx (hdc, w, h);
    buf_dc = CreateCompatibleDCEx (hdc, w, h);

    FillBoxWithBitmap (fgmemdc, 0, 0, w, h, bitmap);

    as = CreateAnimateSence (interval, all_times,
            hdc, draw_animate, draw_bkgnd, on_finished, rc, (void*)bkmemdc);
    as->end_draw_one_frame = end_draw_one_frame;

    a = (ANIMATE*) calloc (1, sizeof(ANIMATE));
    a->img = (void*)fgmemdc;
    SetAnimateW(a, w);
    SetAnimateH(a, h);
    SetAnimateX(a, rc->left);
    SetAnimateY(a, rc->top);
    SetAnimateA(a, start_alpha);
    InsertAnimate(as, a, FALSE);

    tl = CreateTimeLine((intptr_t)a);
    TLAlphaTo(tl, a, end_alpha, all_times/interval - 3);

    TLRun (tl, all_times/interval - 3);
    StartTimeLine(as, tl, NULL, 0, NULL);
    //StartTimerAnimateSence(as);
    StartAnimateSence(as);
}

#else

typedef struct _alphainfo{
    const BITMAP* img;
    HDC   buf_dc;
}ALPHAINFO;

static void draw_animate (HDC hdc, ANIMATE* ani)
{
    HDC bmpdc = (HDC)ani->img;
    SetMemDCAlpha(bmpdc, MEMDC_FLAG_SRCALPHA, GetAnimateA(ani));
    BitBlt(bmpdc, 0, 0, GetAnimateW(ani), GetAnimateH(ani), hdc,
            GetAnimateX(ani), GetAnimateY(ani), 0);
}

static void draw_bkgnd (HDC hdc, const RECT* rc, void* param)
{
    //BitBlt ((HDC)param, 0, 0, RECTWP(rc), RECTHP(rc), hdc, rc->left, rc->top, 0);
    ALPHAINFO* alpha = (ALPHAINFO*)param;
    //SetBrushColor(alpha->buf_dc, RGB2Pixel(HDC_SCREEN, 0, 0, 0));
    SetBrushColor(alpha->buf_dc, 0);
    FillBox(alpha->buf_dc, rc->left, rc->top, RECTWP(rc), RECTHP(rc));
}

static void on_finished (ANIMATE_SENCE* as)
{
    ALPHAINFO* alpha = (ALPHAINFO*)as->param;
    if (as->param)
        DeleteCompatibleDC ((HDC)alpha->buf_dc);

    DeleteCompatibleDC ((HDC)as->normal->img);
    free (as->normal);
    as->normal = NULL;
}


static void end_draw_one_frame(ANIMATE_SENCE *as) {
    if(as != NULL)
    {
        ALPHAINFO* alpha = (ALPHAINFO*)as->param;
        //BitBlt(as->hdc, 0, 0, RECTW(as->rtArea), RECTH(as->rtArea), HDC_SCREEN, 0, 0, 0);
        BitBlt(alpha->buf_dc, 0, 0, 0, 0, HDC_SCREEN, 0, 0, 0);
    }
}

void StartAlphaAnimate (int interval, int all_times, HDC hdc, const BITMAP* bitmap, const RECT* rc)
{
    TIME_LINE* tl;
    ANIMATE* a;
    ANIMATE_SENCE* as;
    int w = RECTWP(rc);
    int h = RECTHP(rc);
    HDC buf_dc  = CreateCompatibleDCEx (hdc, w, h);
    HDC fgmemdc = CreateCompatibleDCEx (hdc, w, h);
    ALPHAINFO _alpha;

    //SetMemDCAlpha(fgmemdc, MEMDC_FLAG_SRCALPHA, 10);
    //SetMemDCAlpha(buf_dc, MEMDC_FLAG_SRCALPHA, 10);
    //BitBlt (hdc, rc->left, rc->top, RECTWP(rc), RECTHP(rc), buf_dc, 0, 0, 0);
    FillBoxWithBitmap (fgmemdc, 0, 0, w, h, bitmap);
    _alpha.buf_dc = buf_dc;
    _alpha.img    = bitmap;

    as = CreateAnimateSence (interval, all_times,
            buf_dc, draw_animate, draw_bkgnd, on_finished, rc, (void*)&_alpha);
    as->end_draw_one_frame = end_draw_one_frame;

    a = (ANIMATE*) calloc (1, sizeof(ANIMATE));
    a->img = (void*)fgmemdc;
    SetAnimateW(a, w);
    SetAnimateH(a, h);
    SetAnimateA(a, 0);
    InsertAnimate(as, a, FALSE);

    tl = CreateTimeLine((intptr_t)a);
    TLAlphaTo(tl, a, 255, all_times/interval - 3);
    TLRun (tl, all_times/interval - 3);
    StartTimeLine(as, tl, NULL, 0, NULL);
    StartTimerAnimateSence(as);
}

#endif // KBD_ANIMATE

