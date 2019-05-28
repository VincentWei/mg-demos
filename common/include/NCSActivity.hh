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
 * @file NCSActivity.hh
 * @Synopsis
 * @author DongKai
 * @version 1.0
 *  Company: Beijing FMSoft Technology Co., Ltd.
 */

#ifndef  NCSACTIVITY_INC
#define  NCSACTIVITY_INC

/**
 * @synopsis NCSActivity
 */
class NCSActivity : public Activity
{
public:
    virtual ~NCSActivity();

    bool isAutoEscape () {
        return m_autoEscape;
    }

    void setAutoEscape (bool autoEscape) {
        m_autoEscape = autoEscape;
    }

    void onDestroyOrigin(mWidget* widget, int msg) {
        if (m_onDestroyOrigin) {
            m_onDestroyOrigin(widget, msg);
        }
    }

    virtual int onResume();
    virtual int onPause();
    virtual int onCancel() {
        ActivityStack::singleton()->pop();
        return 0;
    }

protected:
    // tmpl or package just include content of body.
    NCSActivity(NCS_MNWND_TEMPLATE* _template, bool autoEscape = true);

    NCSActivity(HPACKAGE hPack, HICON hIcon, HMENU hMenu, bool autoEscape = true) :
        m_tmpl(NULL), m_hPack(hPack), m_hIcon(hIcon), m_hMenu(hMenu), m_autoEscape(autoEscape)
    {
    }

    NCSActivity(const NCSActivity&);
    virtual HWND createHWND();

    NCS_MNWND_TEMPLATE* m_tmpl;
    HPACKAGE m_hPack;
    HICON m_hIcon;
    HMENU m_hMenu;
    void (*m_onDestroyOrigin)(mWidget*, int);

    bool m_autoEscape;
};

#endif   /* ----- #ifndef NCSACTIVITY_INC  ----- */

