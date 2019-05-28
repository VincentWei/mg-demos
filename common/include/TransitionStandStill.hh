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
/**
 * @file TransitionStandStill.hh
 * @synopsis
 * @author ???
 * @version 0.1
 * @date 2010-08-02
 * Copyright © 2002-2009 Beijing FMSoft Technology Co., Ltd.
 */

#ifndef TRANSITIONSTANDSTILL_HH
#define TRANSITIONSTANDSTILL_HH

#include "StateMachine.hh"
using namespace mStateMachine;

/**
 * @synopsis  TransitionStandStill
 */
class TransitionStandStill : public EventTransition {
public:
    TransitionStandStill(State *source, State *target, int timeoutMs, const char *name);
    virtual ~TransitionStandStill();
private:
    virtual void reset();
    bool testMouse(const MouseEvent *);
    bool testMove(const MoveEvent *);
    bool testTimer(const TimerEvent *);
protected:
    POINT m_point;
private:
    int m_timeout;
    int m_status;
    int m_timeOnEnter;
};

#endif /* TRANSITIONSTANDSTILL_HH */
