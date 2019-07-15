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
 * @file TimeService.hh
 * @Synopsis
 * @author DengMin
 * @version 1.0
 * @date 23/11/2010 16:09:07
 *  Company: Beijing FMSoft Technology Co., Ltd.
 */

#ifndef _TIMESERVICE_H
#define _TIMESERVICE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>

#include <mgncs/mgncs.h>

class TimeService;

typedef BOOL (*TIMESERVICE_ONEVENT_CB)(void* target, TimeService *ts, int eventId, DWORD param);

class TimeSnapshot
{
public:
    TimeSnapshot();
    TimeSnapshot(time_t t);

    const int get(int type) const;

    bool set(int type, int val);

    size_t format(char* s, size_t max, const char* fmt) const;

    static bool inSameDay(const TimeSnapshot* first, const TimeSnapshot* second);

    friend class TimeService;

protected:
    time_t m_timeInSeconds;

    void inc();
    void reset();
    struct tm* localtime(struct tm* tm) const;
};

class TimeService
{
public:
    enum {
        SECOND = 0,
        MINUTE,
        HOUR,
        MDAY,
        MONTH,
        YEAR,
        SEC_FROM_EPOCH,

        WDAY,
    };

    static TimeService* singleton();

    const int get(int type) const;

    const TimeSnapshot snapshot() const { return m_time; }

    bool set(int type, int val);

    void reset();

    size_t format(char* s, size_t max, const char* fmt) const;

    // TODO: return an listener id
    BOOL addEventListener(void* target, int eventId, TIMESERVICE_ONEVENT_CB onEvent);
    // TODO: Remove event listener by target is not safe, consider to remove by id returned by addEventListener.
    BOOL removeEventListener(void* target);

    // there are two method for another moudle access the info bar window, for example update it.
    // TODO: This implementation of a very ugly and should be replaced by a more rational way.
    void setHost(HWND hwnd) {m_hwnd = hwnd;}
    HWND getHost() { return m_hwnd; }

private:
    static BOOL timerProc(HWND hwnd, LINT id, DWORD count);

    TimeService();
    virtual ~TimeService() { }

    TIMESERVICE_ONEVENT_CB findEventHandler(DWORD target, int eventId);

private:
    TimeSnapshot m_time;
    time_t m_diff;
    unsigned int m_ticket;
    list_t m_eventHandler;
    HWND m_hwnd;

    static TimeService* s_service;

    class Gc
    {
    public:
        ~Gc()
            {
                if(TimeService::s_service)
                    delete TimeService::s_service;
            }
    };
    static Gc gc;
};


#define TIMESERVICE TimeService::singleton()

#endif
