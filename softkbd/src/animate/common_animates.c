
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include "common_animates.h"

#if KBD_ANIMATE

static ANIMATE_SENCE sence;
static int interval = 100;

static void def_nothing_drawbkgnd(HDC hdc, RECT* rt, void *param)
{
}

void SetInterval(int interval_ms)
{
	if(interval_ms < 50)
		interval = 50;
	else
		interval = interval_ms;
}

//define the default ANIMATE_OPS

static void def_draw_animate(HDC hdc, ANIMATE*ani)
{
//	printf("%s: %d,%d,%d,%d, img=%p\n",__FUNCTION__, GetAnimateX(ani), GetAnimateY(ani), GetAnimateW(ani), GetAnimateH(ani), ani->img);
	if(GetAnimateW(ani) != 0 && GetAnimateH(ani)!=0)
	{
		FillBoxWithBitmap(hdc, 
			GetAnimateX(ani), GetAnimateY(ani),
			GetAnimateW(ani), GetAnimateH(ani),
			(PBITMAP)ani->img);
	}
}

static void def_draw_bkgnd(HDC hdc, const RECT* rtbk, void *param)
{
	//draw white bkgnd
	gal_pixel oldPixel = SetBrushColor(hdc, RGB2Pixel(hdc, 0xFF,0xFF,0xFF));

	FillBox(hdc, rtbk->left, rtbk->top, RECTWP(rtbk), RECTHP(rtbk));

	SetBrushColor(hdc, oldPixel);
}

void RunPushPullAnimate(HDC hdc, const RECT *rt, 
	PUSH_PULL_OBJ* objs, int obj_cnt, 
	ANIMATE_OPS *ops, 
	int frame_num,
	void* user_param)
{
	TIME_LINE* tl;
	int i;
	if(objs == NULL || obj_cnt <= 0){
		fprintf(stderr, "ERROR: common animates: Invalidate Param: PUSH_PULL_OBJ is null in %s\n", __FUNCTION__);
		return ;
	}

	memset(&sence, 0, sizeof(sence));

	if(! InitAnimateSence(&sence, interval, -1, hdc, 
			(ops && ops->draw_animate)?ops->draw_animate:def_draw_animate,
			(ops && ops->draw_bkgnd)?ops->draw_bkgnd:def_draw_bkgnd,
			ops?ops->on_finished:NULL,
			rt,
			user_param))
	{
		fprintf(stderr, "ERROR: common animates: InitAnimateSence Failed in RunPushPullAnimate\n");
		return ;
	}

	if(ops){
		sence.before_draw_one_frame = ops->before_draw_one_frame;
		//sence.end_draw_one_frame = sence.end_draw_one_frame;
		sence.end_draw_one_frame = ops->end_draw_one_frame;
	}

	//Create a Time line
	tl = CreateTimeLine(-1);

	for(i=0; i<obj_cnt; i++)
	{
		//Create Animate
		ANIMATE* a = (ANIMATE*)calloc(1,sizeof(ANIMATE));
		a->img = objs[i].img;
		SetAnimateX(a, objs[i].x_begin);
		SetAnimateY(a, objs[i].y_begin);
		SetAnimateW(a, RECTWP(rt));
		SetAnimateH(a, RECTHP(rt));
		SetAnimateA(a, 255);
		a->visible = 1;
		InsertAnimate(&sence, a, FALSE);

		TLMoveTo(tl, a, objs[i].x_end, objs[i].y_end, frame_num);
	}
	TLRun(tl, frame_num);

	StartTimeLine(&sence, tl, NULL, 0, NULL);

	StartAnimateSence(&sence);

	memset(&sence, 0, sizeof(sence));	
}

void RunPushPullBitmapAnimate(HDC hdc, const RECT *rt, PBITMAP bmpPush, PBITMAP bmpPull, int frame_num, BOOL left_to_right)
{
	int w,h;
	w = RECTWP(rt);
	h = RECTHP(rt);
	PUSH_PULL_OBJ objs[2] ={
		{bmpPush,left_to_right?-w:w, 0,0,0},
		{bmpPull,0, 0, left_to_right?w:-w, 0}
	};

	RunPushPullAnimate(hdc, rt, objs, 2, NULL, frame_num, NULL);
}

static void def_move_window(HDC hdc, ANIMATE* a)
{
	MoveWindow((HWND)a->img, 
		GetAnimateX(a),
		GetAnimateY(a),
		GetAnimateW(a),
		GetAnimateH(a),
		TRUE);
}
void RunPushPullWindowAnimate(const RECT *rt, HWND hwndPush, HWND hwndPull, int frame_num, BOOL left_to_right)
{
	int w, h;
	w = RECTWP(rt);
	h = RECTHP(rt);

	PUSH_PULL_OBJ objs[2] ={
		{(void*)hwndPush,left_to_right?-w:w, 0, 0,0},
		{(void*)hwndPull,0, 0, left_to_right?w:-w, 0}
	};
	
	ANIMATE_OPS ops = {
		def_move_window,
		NULL,
		NULL,
		NULL,
		NULL
	};

	RunPushPullAnimate(HDC_SCREEN, rt, objs, 2, &ops, frame_num, NULL);

}


static void def_pin_draw_bkgnd(HDC hdc, const RECT* rtbk, void *param)
{
	FillBoxWithBitmap(hdc, rtbk->left, rtbk->top, RECTWP(rtbk), RECTHP(rtbk), (PBITMAP)param);
}

void RunPinAnimate(HDC hdc, PBITMAP bmpbk, PBITMAP bmpPin, int x, int y, int x_begin, int y_begin, int x_end, int y_end)
{
	TIME_LINE* tl;
	ANIMATE* a;

	if(bmpbk == NULL || bmpPin == NULL){
		fprintf(stderr, "ERROR: common animates: Invalidate Param, bmpbk is NULL or bmpPin is NULL in %s\n", __FUNCTION__);
		return ;
	}

	RECT rtbk = { x, y, x+bmpbk->bmWidth, y+bmpbk->bmHeight};

	memset(&sence, 0, sizeof(sence));

	if(! InitAnimateSence(&sence, 
		interval, -1, 
		hdc, 
		def_draw_animate,
		def_pin_draw_bkgnd,
		NULL,
		&rtbk,
		(void*)bmpbk)){
		fprintf(stderr, "ERROR: common animates: InitAnimateSence Failed in %s\n", __FUNCTION__);
		return ;
	}
	
	tl = CreateTimeLine(-1);
	
	a = (ANIMATE*)calloc(1, sizeof(ANIMATE));

	a->img = bmpPin;
	SetAnimateX(a, x_begin);
	SetAnimateY(a, y_begin);
	SetAnimateW(a, bmpPin->bmWidth);
	SetAnimateH(a, bmpPin->bmHeight);
	SetAnimateA(a, 255);
	a->visible = 1;
	InsertAnimate(&sence, a, FALSE);

	TLMoveTo(tl, a, x_end, y_end, 3);
	TLRun(tl, 3);
	StartTimeLine(&sence, tl, NULL, 0, NULL);
	StartAnimateSence(&sence);
	memset(&sence, 0, sizeof(sence));
}

static void on_run_jump_finished(ANIMATE_SENCE* as)
{
    memset(as, 0, sizeof(ANIMATE_SENCE));
}
void RunJumpWindow(HWND hwnd, int x_begin, int y_begin, int x_end, int y_end, int w, int h, void (*on_finished)(ANIMATE_SENCE* as), void *param)
{
	RECT rt;
	TIME_LINE* tl;
	ANIMATE* a;
	if(!IsWindow(hwnd))
	{
		fprintf(stderr, "ERROR: common animates: invalidate param: hwnd is not window in %s\n", __FUNCTION__);
		return ;
	}

	rt.left = x_begin;
	rt.right = x_end;
	rt.top = y_begin;
	rt.bottom = y_end;

	NormalizeRect(&rt);

	memset(&sence, 0, sizeof(sence));

	if(! InitAnimateSence(&sence, 
		interval, -1, 
		HDC_SCREEN, 
		def_move_window,
		def_nothing_drawbkgnd,
		on_finished,
		&rt,
		param)){
		fprintf(stderr, "ERROR: common animates: InitAnimateSence Failed in %s\n", __FUNCTION__);
		return ;
	}
	a = (ANIMATE*)calloc(1, sizeof(ANIMATE));

	SetAnimateX(a, x_begin);
	SetAnimateY(a, y_begin);
	SetAnimateW(a, w);
	SetAnimateH(a, h);
	a->img = (void*)hwnd;

	InsertAnimate(&sence, a, FALSE);
	
	tl = CreateTimeLine(-1);
	TLMoveTo(tl, a, x_end, y_end, 5);
	TLRun(tl, 5);	

	StartTimeLine(&sence, tl, NULL, 0, NULL);

	StartTimerAnimateSence(&sence);
}

#endif

