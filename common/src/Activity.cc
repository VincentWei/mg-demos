/*! ============================================================================
 * @file activity.cc 
 * @Synopsis  
 * @author DongKai
 * @version 1.0
 *  Company: Beijing Feynman Software Technology Co., Ltd.
 */

#include <cassert>
#include <sstream>

#include "global.h"
#include "utilities.h"

#include "Activity.hh"

ActivityFactory *ActivityFactory::s_single = NULL;

// public:

// constructor & destructor
Activity::Activity() :
    m_pushIndex(0), m_hwnd(HWND_INVALID), m_style(Activity::STYLE_ALPHA), m_needSwitchEffect(true)
{
}

Activity::~Activity()
{
    deleteHWNDSecondaryDC();
}

void Activity::setHWNDSecondaryDC(HWND hwnd)
{
    HDC hdc = GetSecondaryDC(hwnd);
    RECT rcWin;

    _MG_PRINTF ("Activity::setHWNDSecondaryDC called\n");

    /* has secondary dc.*/
    if (hdc != HDC_SCREEN) {
        return;
    }

    GetWindowRect (hwnd, &rcWin);
    hdc = CreateCompatibleDCEx (HDC_SCREEN, RECTW(rcWin), RECTH(rcWin));
    if (hdc != HDC_INVALID) {
        SetSecondaryDC (hwnd, hdc, ON_UPDSECDC_DEFAULT);
    }
}

void Activity::deleteHWNDSecondaryDC()
{
    DeleteSecondaryDC(this->hwnd());
}

// create this activity, including creating window, setting additional data, etc.
int Activity::create() 
{
    m_hwnd = createHWND();

    if (m_hwnd == HWND_INVALID || m_hwnd == HWND_NULL) {
        fprintf(stderr, "Create Window failed!\n");
        return -1;
    }

    /* force to use SecondaryDC */
    setHWNDSecondaryDC (m_hwnd);

    // store 'this' pointer for static function.
    SetWindowAdditionalData(m_hwnd, (DWORD)this);
    return 0;
}

// show the window of this activity
void Activity::show()
{
#if 0
    ShowWindow(m_hwnd, SW_SHOWNORMAL);
#else
    HDC sec_dc;
    HDC dst_dc;
    HWND hmainwnd;

    /* if is main window(not control), do not need to release secondary dc */
    hmainwnd = GetMainWindowHandle(m_hwnd);
    sec_dc = GetSecondaryDC(hmainwnd);
    if (HDC_INVALID != sec_dc && HDC_SCREEN != sec_dc) {
        SetSecondaryDC(hmainwnd, sec_dc, ON_UPDSECDC_DONOTHING);

        ShowWindow(hmainwnd, SW_SHOWNORMAL);
        UpdateWindow(hmainwnd, FALSE);
        SetSecondaryDC(hmainwnd, sec_dc, ON_UPDSECDC_DEFAULT);

        dst_dc = GetClientDC(hmainwnd); 
        BitBlt(sec_dc, 0, 0, 0, 0, dst_dc, 0, 0, 0);
        ReleaseDC(dst_dc);
    } else {
        ShowWindow(hmainwnd, SW_SHOWNORMAL);
    }
#endif
}

// hide the window of this activity
void Activity::hide()
{
    ShowWindow(m_hwnd, SW_HIDE);
}

// get snapshot of this activity
HDC Activity::snapshot() const
{
    return forceGetShadowDC (m_hwnd, FALSE);
}

