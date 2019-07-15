///////////////////////////////////////////////////////////////////////////////
//
//                          IMPORTANT NOTICE
//
// The following open source license statement does not apply to any
// entity in the Exception List published by FMSoft.
//
// For more information, please visit:
//
// https://www.fmsoft.cn/exception-list
//
//////////////////////////////////////////////////////////////////////////////
/*
** This file is a part of mg-demos package.
**
** Copyright (C) 2010 ~ 2019 FMSoft (http://www.fmsoft.cn).
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
