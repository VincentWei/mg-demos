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
 * @file TransitionAnimationFinished.hh
 * @synopsis
 * @author WanZhen
 * @version 0.1
 * @date 2010-08-02
 * Copyright © 2002-2009 Beijing FMSoft Technology Co., Ltd.
 */

#ifndef TRANSITIONANIMATIONFINISHED_HH
#define TRANSITIONANIMATIONFINISHED_HH

#include "StateMachine.hh"
#include <mgeff/mgeff.h>
using namespace mStateMachine;

/**
 * @synopsis  TransitionAnimationFinished
 */
class TransitionAnimationFinished : public SignalTransition {
public:
    TransitionAnimationFinished(MGEFF_ANIMATION animation, State *source, State *target, const char *name=NULL);
    ~TransitionAnimationFinished();
    void setAnimation(MGEFF_ANIMATION animation);
private:
    static void onAnimationFinished(MGEFF_ANIMATION animation);
};

#endif /* TRANSITIONANIMATIONFINISHED_HH */
