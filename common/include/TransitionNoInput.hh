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
/*!============================================================================
 * @file TransitionNoInput.hh
 * @Synopsis
 * @author DongKai
 * @version 1.0
 *  Company: Beijing FMSoft Technology Co., Ltd.
 */

#ifndef  TRANSITIONNOINPUT_INC
#define  TRANSITIONNOINPUT_INC

#include <ctime>
#include "StateMachine.hh"
using namespace mStateMachine;

/**
 * @synopsis  TransitionNoInput
 */
class TransitionNoInput : public EventTransition
{
public:
    TransitionNoInput(State *source, State *target,
            unsigned int timeoutSec, const char *name="");
    ~TransitionNoInput();

private:
    virtual void reset();
    bool testMouse(const MouseEvent *);
    bool testMove(const MoveEvent *);
    bool testTimer(const TimerEvent *);

private:
    time_t  m_timeOut;
    time_t  m_timeOnEnter;
};


#endif   /* ----- #ifndef TRANSITIONNOINPUT_INC  ----- */
