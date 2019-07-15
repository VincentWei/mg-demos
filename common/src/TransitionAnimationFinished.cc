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
#include "TransitionAnimationFinished.hh"
#include "global.h"

static const char SIGNAL_FINISHED[] = "finished";

TransitionAnimationFinished::TransitionAnimationFinished(MGEFF_ANIMATION animation, State *source, State *target, const char *name)
    : SignalTransition(animation, SIGNAL_FINISHED, source, target, name)
{
    setAnimation(animation);
}

TransitionAnimationFinished::~TransitionAnimationFinished() {
}

void TransitionAnimationFinished::setAnimation(MGEFF_ANIMATION animation) {
    setSignal(animation, SIGNAL_FINISHED);
    if (animation) {
        mGEffAnimationSetContext(animation, this); /* XXX: occupy the context of the animation */
        mGEffAnimationSetFinishedCb(animation, onAnimationFinished);
    }
    else
        printf ("animation not set\n");
}

void TransitionAnimationFinished::onAnimationFinished(MGEFF_ANIMATION animation) {
    SignalEvent *event = new SignalEvent(animation, SIGNAL_FINISHED);
    Transition *transition = (Transition *) mGEffAnimationGetContext(animation);

    printf("%s:%d.\n", __FUNCTION__, __LINE__);
    transition->source()->stateMachine()->postEvent(event);
}
