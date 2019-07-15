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
