#include <assert.h>

#include "global.h"
#include "TransitionHover.hh"

TransitionHover::TransitionHover(State *source, State *target, RECT rc, int timeoutMs, const char *name)
    : EventTransition(Event::MoveEvent | Event::TimerEvent, source, target, name), 
    m_rc(rc), 
    m_timeout(timeoutMs), 
    m_status(0), 
    m_timeOnEnter(0)
{
    assert(m_timeout > 0);
}

TransitionHover::~TransitionHover() {
}

void TransitionHover::reset() {
    m_status = 0;
}

bool TransitionHover::testMove(const MoveEvent *event) {
    POINT pt = event->pos();
    if (m_status == 0 && PtInRect(&m_rc, pt.x, pt.y)) {
        m_timeOnEnter = GetTickCount();
        m_status = 1;
    }
    return false;
}

bool TransitionHover::testTimer(const TimerEvent *event) {
    if (m_status == 1 && (int)(GetTickCount() - m_timeOnEnter) > m_timeout/10) {
        m_status = 2;
        return true;
    }else{
        return false;
    }
}
