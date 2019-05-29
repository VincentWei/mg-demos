#include <stdio.h>
#include <pthread.h>
#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>

#include "animate.h"

#if KBD_ANIMATE

static inline DWORD getcurtime()
{
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return tv.tv_sec*1000 + (tv.tv_usec+999)/1000;
}

static void as_draw_one_frame(ANIMATE_SENCE *as, int skip_frame)
{
	ANIMATE *a, *h;

	if(as->before_draw_one_frame)
		(*as->before_draw_one_frame)(as);

	SelectClipRect(as->hdc, &as->rtArea);

	//clear the bkgnd
	(*as->draw_bkgnd)(as->hdc, &as->rtArea, as->param);

	//caclue timeline
	TIME_LINE* tl = as->timelines;
	TIME_LINE* tlprev = tl;
	while(tl)
	{
		ExecuteTimeLine(tl);

		if(tl->state == tlsEnd){
			TIME_LINE* tldel = tl;
			if(tlprev == tl){
				tl = tl->next;
				as->timelines = tl;
				tlprev = tl;
                tl = as->timelines;
                tlprev = tl;
			}
			else {
				tl = tl->next;
				tlprev->next = tl;
			}
			DeleteTimeLine(tldel);
			//printf("delete timeline %p, timelines=%p\n",tldel, as->timelines);
		}
		else {
			tlprev = tl;
			tl = tl->next;
		}
	}

	//draw animates
	
	a = as->normal;
DRAW_ANIMATES:
	h = a;
	while(a){

		DrawAnimate(as, a);

		a = a->next;
		if(a == h)
			break;
	}
	
	if ( h == as->normal){
		a = as->topmost;
		goto DRAW_ANIMATES;
	}

	SelectClipRect(as->hdc, NULL);

	if(as->end_draw_one_frame)
		(*as->end_draw_one_frame)(as);
	//getchar();
}

static void as_draw_loop(ANIMATE_SENCE *as)
{
	// we don't support skip now
	//int skip_frame = 0;
	int interval_sec;
	int interval_nsec;
	if(as->total_time > 0)
		as->start_time = getcurtime();

	interval_sec = as->interval/1000;
	interval_nsec = (as->interval%1000)*1000000;

DRAWALL:
	while((as->normal || as->topmost) && as->state == assRunning && as->timelines)
	{
		struct timeval tv;
		struct timespec timeout;
		DWORD before_time/*, end_time*/;
//		before_time = getcurtime();
		gettimeofday(&tv, NULL);
		timeout.tv_sec = tv.tv_sec + interval_sec;
		timeout.tv_nsec = tv.tv_usec*1000 + interval_nsec;
		if(timeout.tv_nsec > 1000000000){
			timeout.tv_sec ++;
			timeout.tv_nsec -= 1000000000;
		}

		if( as->total_time > 0 ){
			before_time = tv.tv_sec*1000 + (tv.tv_usec+999)/1000;
            
			if((before_time - as->start_time) >= as->total_time)
				break;
		}
	

		pthread_mutex_lock(&as->mutex);
		as_draw_one_frame(as, /*skip_frame*/0);

		/*gettimeofday(&tv, NULL);
		end_time = tv.tv_sec*1000 + tv.tv_usec/1000;

		int diff = end_time - before_time;
		//skip_frame = diff / as->interval;

		diff = as->interval - diff%as->interval;
		timeout.tv_sec = tv.tv_sec + diff/1000;
		timeout.tv_nsec = tv.tv_usec*1000 + (diff%1000)*1000000;
		if(timeout.tv_nsec >= 10e9)
		{
			timeout.tv_sec ++;
			timeout.tv_nsec -= 10e9;
		}
	*/	
		pthread_cond_timedwait(&as->cond, &as->mutex,&timeout);

		pthread_mutex_unlock(&as->mutex);

	}

	if(as->state == assRunning){
		as->state = assFinished;
	}
	else if(as->state == assPaused){
		pthread_mutex_lock(&as->mutex);
		pthread_cond_wait(&as->cond, &as->mutex);
		pthread_mutex_unlock(&as->mutex);
		goto DRAWALL;
	}
	else{
		as->state = assStoped;
	}

	if(as->on_finished)
		(*as->on_finished)(as);
}


static void defDrawAnimate(HDC hdc, ANIMATE* a)
{
	FillBoxWithBitmap(hdc, GetAnimateX(a), GetAnimateY(a), GetAnimateW(a),GetAnimateH(a),(BITMAP*)a->img);
}

static void defAlphaDrawBitmap(HDC hdc, ANIMATE* a)
{
	if(a->alpha > 240)
	{
		FillBoxWithBitmap(hdc, GetAnimateX(a), GetAnimateY(a), GetAnimateW(a), GetAnimateH(a), (BITMAP*)a->img);
	}
	else
	{
		//create from memdc	
	}
}

static void defMemDCDraw(HDC hdc, ANIMATE* a)
{
	SetMemDCAlpha((HDC)a->img, MEMDC_FLAG_SRCALPHA, GetAnimateA(a));
	SetMemDCColorKey((HDC)a->img,MEMDC_FLAG_SRCCOLORKEY, 0);
	BitBlt((HDC)a->img, 0, 0, GetAnimateW(a), GetAnimateH(a), hdc, GetAnimateX(a), GetAnimateY(a),0);
}

static void defWindowDraw(HDC hdc, ANIMATE* a)
{
	MoveWindow((HWND)a->img, GetAnimateX(a), GetAnimateY(a), GetAnimateW(a), GetAnimateH(a), TRUE);
}


/////////////////////////////////////////////
//APIs
/*if all_times==-1, no stop*/
ANIMATE_SENCE* CreateAnimateSence (int interval, int all_times, HDC hdc, 
		DRAW_ANIMATE drawAnimate,
        void (*drawbkgnd)(HDC, const RECT* , void*), 
		void (*onFinished)(ANIMATE_SENCE *),
        const RECT* bkgnd, void* user_data)
{

	ANIMATE_SENCE * as = (ANIMATE_SENCE*)calloc(1,sizeof(ANIMATE_SENCE));

	if(InitAnimateSence(as, interval, all_times, hdc,drawAnimate, drawbkgnd, onFinished, bkgnd, user_data))
		return as;

	free(as);
	return NULL;
}

//default drawbkgnd function
static void defASDrawbkgnd(HDC hdc, const RECT* prt, void * usr_param)
{
	gal_pixel old_bk = SetBrushColor(hdc, 0xFFFFFF); //white color
	FillBox(hdc, prt->left, prt->top,RECTWP(prt), RECTHP(prt));
	SetBrushColor(hdc, old_bk);
}

BOOL InitAnimateSence(ANIMATE_SENCE* as, int interval, int all_times, HDC hdc,
	DRAW_ANIMATE drawAnimate,
	void (*drawbkgnd)(HDC, const RECT*, void *),
	void (*onFinished)(ANIMATE_SENCE *),
	const RECT* bkgnd, void * user_data)
{
	if(as == NULL)
		return FALSE;

	as->interval = (interval<=10)?10:interval;

	as->total_time = all_times;

	as->mode = 0;

	as->state = assReady;

	as->hdc = (hdc==HDC_INVALID)?HDC_SCREEN:hdc;

	if(bkgnd == NULL)
		as->rtArea = g_rcScr;
	else
		as->rtArea = *bkgnd;

	if(drawbkgnd == NULL)
		as->draw_bkgnd = defASDrawbkgnd;
	else
		as->draw_bkgnd = drawbkgnd;

	as->drawAnimate = drawAnimate;

	as->on_finished = onFinished;

	as->param = user_data;

	return TRUE;

}

void DeinitAnimateSence(ANIMATE_SENCE* as, void(*free_data)(void*))
{
	if(as)
	{
		StopAnimateSence(as);
		//stop timeline and delete it
	}		
}

void DeleteAnimateSence (ANIMATE_SENCE* as, void(*free_data_pool)(void*))
{
	if(as)
	{
		DeinitAnimateSence(as, free_data_pool);
		free(as);
	}
}

BOOL InsertAnimate (ANIMATE_SENCE* as, ANIMATE* a,BOOL is_topmost)
{
	ANIMATE *head;

	if(as == NULL || a == NULL)
		return FALSE;

	if(is_topmost){
		if(as->topmost == NULL){
			as->topmost = a;
			a->prev = a->next = a;
			return TRUE;
		}
		head = as->topmost;
	}
	else
	{
		if(as->normal == NULL){
			as->normal = a;
			a->prev = a->next = a;
			return TRUE;
		}
		head = as->normal;
	}

	//insert the last place
	a->next = head;
	a->prev = head->prev;
	head->prev = a;
	a->prev->next = a;

	return TRUE;
}

BOOL RemoveAnimate (ANIMATE_SENCE* as, ANIMATE* a)
{
	if(as == NULL || a == NULL || a->prev == NULL || a->next == NULL)
		return FALSE;
	
	if(a->prev == a && a->next == a )
	{
		if(as->normal == a){ 
			as->normal = NULL;
			return TRUE;
		}
		else if(as->topmost == a){
			as->topmost = NULL;
			return TRUE;
		}
	}

	a->prev->next = a->next;
	a->next->prev = a->prev;

	if(as->normal == a)
		as->normal = a->next;
	else if(as->topmost == a)
		as->topmost = a->next;
	
	return TRUE;
}

BOOL MoveAnimateToTopMost (ANIMATE_SENCE* as, ANIMATE*a)
{
	RemoveAnimate(as, a);
	return InsertAnimate(as, a, TRUE);
}

BOOL RemoveAnimateFromTopMost (ANIMATE_SENCE* as, ANIMATE*a)
{
	RemoveAnimate(as, a);
	return InsertAnimate(as, a, FALSE);
}

static void as_init_thread_obj(ANIMATE_SENCE* as)
{
	if(as == NULL)
		return;

	pthread_cond_init(&as->cond,NULL);
	pthread_mutex_init(&as->mutex, NULL);
}

static void as_deinit_thread_obj(ANIMATE_SENCE* as)
{
	if(as == NULL)
		return ;
	
	pthread_cond_destroy(&as->cond);
	pthread_mutex_destroy(&as->mutex);
}

static void as_delete_timelines(ANIMATE_SENCE* as)
{
	TIME_LINE* tl = as->timelines;
	TIME_LINE* tldel;
	while(tl){
		tldel = tl;
		tl = tl->next;
		DeleteTimeLine(tldel);
	}
	as->timelines = NULL;
}

static void as_delete_animates(ANIMATE_SENCE *as)
{
	ANIMATE *a = as->normal;
	ANIMATE *aend = a;
	while(a){
		ANIMATE *atmp = a;
		a = a->next;
		free(atmp);

		if(aend == a)
		{
			if( a == as->normal)
			{
				a = as->topmost;
				aend = a;
			}
			else
				break;
		}
	}
	as->normal = NULL;
	as->topmost = NULL;
}

int StartAnimateSence (ANIMATE_SENCE* as)
{
	if(as == NULL)
		return -1;

	if(as->state != assReady)
		return -1;
	
	as->mode = asmBlock;

	as_init_thread_obj(as);

	as->state = assRunning;

	as_draw_loop(as);

	as_delete_timelines(as);
	as_delete_animates(as);
	as_deinit_thread_obj(as);

	return as->state;
	
}

static void * asLoopProc(void * param)
{
	ANIMATE_SENCE *as = (ANIMATE_SENCE*) param;

	as_draw_loop(as);

	as_delete_timelines(as);
	as_delete_animates(as);
	as_deinit_thread_obj(as);

	as->pthread = 0;
	
	return NULL;
}

int StartThreadAnimateSence (ANIMATE_SENCE* as)
{
	//create thread
	if(as == NULL || as->state != assReady)
		return -1;

	as->mode = asmThread;
	as->state = assRunning;

	as_init_thread_obj(as);

	pthread_create(&as->pthread, NULL, asLoopProc, (void*)as);

	return 0;
}

static BOOL as_timer_draw(ANIMATE_SENCE* as, int speed, DWORD dw)
{
	DWORD cur_time = getcurtime();

	if(as->timelines == NULL || (as->normal == NULL && as->topmost == NULL) ||
        (as->total_time > 0 
         && (cur_time - as->start_time) > as->total_time))
	{
		as->state = assFinished;
		if(as->on_finished)
			(*as->on_finished)(as);
		return FALSE; //miniugi will delete timer
	}

	if(as->state == assPaused){
		as->old_time = cur_time;
		return TRUE;
	}

	if( (as->normal || as->topmost) && as->state == assRunning)
	{
		int skip_frame = (cur_time - as->old_time)/as->interval;
		as_draw_one_frame(as, skip_frame);

		as->old_time = cur_time;
		return TRUE;
	}
	
	as->state = as->state == assRunning?assFinished:assStoped;


	if(as->on_finished)
		(*as->on_finished)(as);

	as_delete_timelines(as);
	as_delete_animates(as);
	as_deinit_thread_obj(as);

	return FALSE; //delete time by minigui
}

int StartTimerAnimateSence (ANIMATE_SENCE *as)
{
	if(as == NULL || as->state != assReady )
		return -1;

	as->mode = asmTimer;

	as->state = assRunning;

    as->start_time = getcurtime();
	as->old_time = as->start_time;

	return SetTimerEx((HWND)as, 0, as->interval/10, as_timer_draw);
}

static BOOL as_set_state(ANIMATE_SENCE *as, int state)
{
	if(as == NULL || as->state == state)
		return FALSE;

	switch(as->mode){
	case asmBlock:
	case asmThread:
		pthread_mutex_lock(&as->mutex);
		as->state = state;
		pthread_cond_signal(&as->cond);
		pthread_mutex_unlock(&as->mutex);
		break;
	case asmTimer:
		as->state = state;
	default:
		return FALSE;
	}

	return TRUE;
}

BOOL StopAnimateSence (ANIMATE_SENCE *as)
{
	if(!(as->state == assRunning || as->state == assPaused))
		return FALSE;
	
	return as_set_state(as, assStoped);
}


BOOL PauseAnimateSence (ANIMATE_SENCE *as)
{
	if(as == NULL || as->state != assRunning)
		return FALSE;

	return as_set_state(as, assPaused);
}

BOOL ContinueAnimateSence (ANIMATE_SENCE* as)
{
	if(as == NULL || as->state != assPaused)
		return FALSE;
	
	return as_set_state(as, assRunning);
}

#endif

