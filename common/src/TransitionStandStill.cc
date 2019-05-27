#include <assert.h>
#include "TransitionStandStill.hh"
#include <minigui/gdi.h>
#include <minigui/window.h>
#include "global.h"

TransitionStandStill::TransitionStandStill(State *source, State *target, int timeoutMs, const char *name)
    : EventTransition(Event::MouseEvent | Event::MoveEvent | Event::TimerEvent, source, target, name), 
    m_timeout(timeoutMs), m_status(0), m_timeOnEnter(0)
{
    assert(m_timeout > 0);
    setButtonType(MouseEvent::ButtonPressed);
    setButtons(MouseEvent::LeftButton);
}

TransitionStandStill::~TransitionStandStill() {
}

void TransitionStandStill::reset() {
    m_status = 0;
}

bool TransitionStandStill::testMouse(const MouseEvent *event) {
    if (m_status == 0 && EventTransition::testMouse(event)) {
        m_status = 1;
        m_timeOnEnter = GetTickCount();
        event->point(m_point);
    }else{
        m_status = 0;
    }
    return false;
}

bool TransitionStandStill::testMove(const MoveEvent *event) {
    m_status = 0;
    return false;
}

bool TransitionStandStill::testTimer(const TimerEvent *event) {
    if (m_status == 1 && (int)(GetTickCount() - m_timeOnEnter) > m_timeout/10) {
        m_status = 2;
        return true;
    }else{
        return false;
    }
}
