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
/*! ============================================================================
 * @file NCSActivity.cc
 * @Synopsis
 * @author DongKai
 * @version 1.0
 *  Company: Beijing FMSoft Technologies Co., Ltd.
 */

#include <cassert>

#include "global.h"
#include "Activity.hh"
#include "ActivityStack.hh"
#include "NCSActivity.hh"

static BOOL onMyKeyDown (mMainWnd* self, int msg, int scancode, DWORD shiftkeys)
{
    NCSActivity* act = (NCSActivity*) Activity::getActivityFromHWND (self->hwnd);
    if (msg == MSG_KEYDOWN && scancode == SCANCODE_ESCAPE) {
        if (act->onCancel () == 0)
            return FALSE;
    }

    return TRUE;
}

#define HOOK_ONDESTROY 1
void global_onDestroy(mWidget* self, int message)
{
    Activity* activity = Activity::getActivityFromHWND (self->hwnd);
    if (NULL != activity) {
        NCSActivity* ncsActivity = dynamic_cast<NCSActivity*>(activity);
        assert(NULL != ncsActivity);
        if (NULL != ncsActivity) {
            ncsActivity->onDestroyOrigin(self, message);
        }
        delete ncsActivity;
    }
}

NCSActivity::NCSActivity(NCS_MNWND_TEMPLATE* _template, bool autoEscape) :
    m_tmpl(_template), m_hPack(NULL), m_hIcon(NULL), m_hMenu(NULL), m_autoEscape(autoEscape)
{
#if HOOK_ONDESTROY
    if (NULL != _template) {
        int i = 0;
        const NCS_EVENT_HANDLER* store = _template->handlers;
        if (NULL != store) {
            NCS_MNWND_TEMPLATE* tmpl
                = (NCS_MNWND_TEMPLATE*)malloc(sizeof(*tmpl));
            memcpy(tmpl, _template, sizeof(*tmpl));
            while (true) {
                if (MSG_DESTROY == _template->handlers[i].message) {
                    int found = i;
                    // calculate how many message handlers in message map table.
                    while (++i) {
                        if (0 == _template->handlers[i].message &&
                                NULL == _template->handlers[i].handler) {
                            break;
                        }
                    }
                    m_onDestroyOrigin = reinterpret_cast<void (*)(mWidget*, int)>(_template->handlers[found].handler);
                    assert(m_onDestroyOrigin != global_onDestroy);

                    tmpl->handlers = (NCS_EVENT_HANDLER*)calloc(1, sizeof(NCS_EVENT_HANDLER) * (i + 1));
                    memcpy(tmpl->handlers, store, sizeof(NCS_EVENT_HANDLER) * (i + 1));
                    /* Overwrite the termination item of original NCS_EVENT_HANDLER table. */
                    tmpl->handlers[found].message = MSG_DESTROY;
                    tmpl->handlers[found].handler = reinterpret_cast<void*>(global_onDestroy);
                    m_tmpl = tmpl;

                    break;
                } else if (0 == _template->handlers[i].message &&
                        NULL == _template->handlers[i].handler) {
                    m_onDestroyOrigin = NULL;
                    tmpl->handlers = (NCS_EVENT_HANDLER*)calloc(1, sizeof(NCS_EVENT_HANDLER) * (i + 2));
                    memcpy(tmpl->handlers, store, sizeof(NCS_EVENT_HANDLER) * i + 1);
                    /* Overwrite the termination item of original NCS_EVENT_HANDLER table. */
                    tmpl->handlers[i].message = MSG_DESTROY;
                    tmpl->handlers[i].handler = reinterpret_cast<void*>(global_onDestroy);
                    m_tmpl = tmpl;
                    break;
                } else {
                }
                ++i;
            }
        }
    }
    else {
        // TODO: throw a exception.
    }
#endif
}

NCSActivity::~NCSActivity()
{
#if HOOK_ONDESTROY
    if (NULL != m_tmpl->handlers) { // If realloc the NCS_MNWND_TEMPLATE struct.
        NCS_MNWND_TEMPLATE* tmpl = const_cast<NCS_MNWND_TEMPLATE*>(m_tmpl);
        free(tmpl->handlers);
        free(tmpl);
    }
#endif
}


int NCSActivity::onResume()
{
    _DBG_PRINTF ("%s:%s:%d, set secondaryDC\n", __FILE__, __FUNCTION__, __LINE__);
    setHWNDSecondaryDC(this->hwnd());
    return 0;
}

int NCSActivity::onPause()
{
    _DBG_PRINTF ("%s:%s:%d, delete secondaryDC\n", __FILE__, __FUNCTION__, __LINE__);
    deleteHWNDSecondaryDC();
    return 0;
}

HWND NCSActivity::createHWND()
{
    mWidget* window = NULL;
    if (m_tmpl) {
        // Pre-set the additional data,
        // so we can call getActivityFromHWND() to get our Activity's pointer
        // in MSG_CREATE message handler.
        m_tmpl->user_data = (DWORD)this;

        window = (mWidget*)ncsCreateMainWindowIndirect (m_tmpl, GET_GLOBAL_HOSTING);
        if (isAutoEscape ())
            ncsSetComponentHandler((mComponent*)window, MSG_KEYDOWN,
                reinterpret_cast<void*>(onMyKeyDown));
    }
    else {
#if 0
        /* TODO: Create main window frome ID is not implemented. */
        window = (mWidget*)ncsCreateMainWindowIndirectFromID (m_hPack, Uint32 wndId,
                    HWND owner, m_hIcon, m_hMenu,
                    NCS_EVENT_HANDLER_INFO* handlers,
                    NCS_EVENT_CONNECT_INFO *connects,
                    DWORD user_data);
#endif
    }

    return NULL != window ? window->hwnd : HWND_NULL;
}

