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
** Copyright (C) 2019 FMSoft (http://www.fmsoft.cn).
** Copyright (C) 2018 Beijing Joobot Technologies Co., Ltd.
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
 * @file MiscService.hh
 * @Synopsis
 * @author Vincent Wei
 * @version 1.0
 * @date 25/05/2018
 */

#ifndef _MISC_SERVICE_HH_
#define _MISC_SERVICE_HH_

#include <string>
#include <vector>

#include "AsyncTask.hh"

#define TASKID_MISC_MAGIC           0x0500
#define TASKID_MISC_START_FUNC      (TASKID_MISC_MAGIC | 0x0000)
#define TASKID_MISC_STOP_FUNC       (TASKID_MISC_MAGIC | 0x0001)
#define TASKID_MISC_UPDATE_STATUS   (TASKID_MISC_MAGIC | 0x0002)
#define TASKID_MISC_START_MUC       (TASKID_MISC_MAGIC | 0x0003)
#define TASKID_MISC_STOP_MUC        (TASKID_MISC_MAGIC | 0x0004)
#define TASKID_MISC_START_HOTSPOT   (TASKID_MISC_MAGIC | 0x0005)
#define TASKID_MISC_STOP_HOTSPOT    (TASKID_MISC_MAGIC | 0x0006)

#define TASKSTATE_SYSTEM_ERR    0
#define TASKSTATE_OK            1

#define APP_AUTO_UPLOAD_CLOUD       "cbplus.app.auto_upload_cloud"
#define APP_MANUAL_UPLOAD_CLOUD     "cbplus.app.manual_upload_cloud"
#define APP_REMOTE_CONTROL_WLAN     "cbplus.app.remote_control_wlan"
#define APP_REMOTE_CONTROL_HOTSPOT  "cbplus.app.soft_ap"
#define APP_REMOTE_CONTROL_CLOUD    "cbplus.app.remote_control_cloud"
#define APP_LIVE_PUSH_CLOUD         "cbplus.app.live_push_cloud"

class AsyncTaskFuncCtl : public AsyncTask
{
public:
    virtual void* execute_task (pthread_t thread_id, int task_id, DWORD add_data);
    virtual void cleanup_task (pthread_t thread_id, void* res) {};
};

class AsyncTaskUpdateStatus : public AsyncTask
{
public:
    virtual void* execute_task (pthread_t thread_id, int task_id, DWORD add_data);
    virtual void cleanup_task (pthread_t thread_id, void* res) { }

    AsyncTaskUpdateStatus () { }
    ~AsyncTaskUpdateStatus () { }
};

#define PHOTO_FILTER_JPEG       0x01
#define PHOTO_FILTER_RAW        0x02
#define PHOTO_FILTER_JPEGRAW    (PHOTO_FILTER_JPEG | PHOTO_FILTER_RAW)
#define PHOTO_FILTER_JPEGINRAW  0x04
#define PHOTO_FILTER_JPEGDRAFT  0x08

class AsyncTaskMUCCtl : public AsyncTask
{
protected:
    int m_duration;
    int m_filter;

public:
    virtual void* execute_task (pthread_t thread_id, int task_id, DWORD add_data);
    virtual void cleanup_task (pthread_t thread_id, void* res) { }

    AsyncTaskMUCCtl (int duration = 3600, int filter = PHOTO_FILTER_JPEG) {
        m_duration = duration;
        m_filter = filter;
    }
    ~AsyncTaskMUCCtl () { }
};

#define MAX_LEN_HOTSPOT_SSID    31
#define MAX_LEN_HOTSPOT_PASSWD  15
typedef struct _HOTSPOT_INFO
{
    bool enabled;
    char ssid [MAX_LEN_HOTSPOT_SSID + 1];
    char passwd [MAX_LEN_HOTSPOT_PASSWD + 1];
} HOTSPOT_INFO;

class AsyncTaskHotspotCtl : public AsyncTask
{
protected:
    bool m_refresh_passwd;
    HOTSPOT_INFO m_hotspot_info;

public:
    virtual void* execute_task (pthread_t thread_id, int task_id, DWORD add_data);
    virtual void cleanup_task (pthread_t thread_id, void* res) { }

    AsyncTaskHotspotCtl (bool refreshPasswd = false) {
        m_refresh_passwd = refreshPasswd;
    }
    ~AsyncTaskHotspotCtl () { }
};

class MiscService
{
public:
    static MiscService* singleton ();

    AsyncTaskFuncCtl* startFunc (HWND hwnd, const char* func_name);
    AsyncTaskFuncCtl* stopFunc (HWND hwnd, const char* func_name = NULL);
    AsyncTaskUpdateStatus* updateStatus (HWND hwnd);

    AsyncTaskMUCCtl* startMUC (HWND hwnd, int duration, int filter);
    AsyncTaskMUCCtl* stopMUC (HWND hwnd);

    AsyncTaskHotspotCtl* startHotspot (HWND hwnd, bool refreshPasswd);
    AsyncTaskHotspotCtl* stopHotspot (HWND hwnd);

    bool getHotspotStatus(HOTSPOT_INFO* hotspot_info);

private:
    MiscService () { }

    ~MiscService () { }

    static MiscService* s_service;

    class Gc
    {
    public:
        ~Gc() {
            if (MiscService::s_service)
                delete MiscService::s_service;
        }
    };

    static Gc gc;
};

#define MISCSERVICE MiscService::singleton()

#endif /* !_MISC_SERVICE_HH_ */

