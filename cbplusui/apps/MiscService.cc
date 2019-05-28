/*
** This file is a part of mg-demos package.
**
** Copyright (C) 2019 FMSoft (http://www.fmsoft.cn).
** Copyright (C) 2018 Beijing Joobot Technogolies Inc.
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
 * @file MiscService.cc
 * @Synopsis
 * @author Vincent Wei
 * @version 1.0
 * @date 13/06/2018
 */

#include <unistd.h>
#include <jbus.h>

#include "global.h"
#include "resource.h"

#include "cJSON.h"
#include "MiscService.hh"

MiscService* MiscService::s_service = NULL;

// get the single instance of MiscService
MiscService* MiscService::singleton()
{
    if (NULL == MiscService::s_service) {
        MiscService::s_service = new MiscService();
    }
    return MiscService::s_service;
}

AsyncTaskFuncCtl* MiscService::startFunc (HWND hwnd, const char* func_name)
{
    AsyncTaskFuncCtl* task = new AsyncTaskFuncCtl ();

    task->createTask (hwnd, TASKID_MISC_START_FUNC, (DWORD)func_name);

    return task;
}

AsyncTaskFuncCtl* MiscService::stopFunc (HWND hwnd, const char* func_name)
{
    AsyncTaskFuncCtl* task = new AsyncTaskFuncCtl ();

    task->createTask (hwnd, TASKID_MISC_STOP_FUNC, (DWORD)func_name);

    return task;
}

void* AsyncTaskFuncCtl::execute_task (pthread_t thread_id, int task_id, DWORD add_data)
{
    const char* cmd;
    char cmd_arg [64];
    INT_PTR status = -1;

    if (add_data) {
        snprintf (cmd_arg, sizeof (cmd_arg) - 1, "{\"name\":\"%s\"}", (const char*)add_data);
    }
    else {
        strncpy (cmd_arg, "{\"name\":NULL}", sizeof (cmd_arg) - 1);
    }

    if (task_id == TASKID_MISC_START_FUNC) {
        cmd = "start";
    }
    else if (task_id == TASKID_MISC_STOP_FUNC) {
        cmd = "stop";
    }
    else {
        cmd = "update";
    }

    char* res = jbus_invoke ("cbplus.am", cmd, cmd_arg);
    if (res == NULL) {
        return (void*)status;
    }

    /* convert to C structure here */
    cJSON* root = cJSON_Parse (res);
    jbus_free (res);

    if (root) {
        cJSON* retCode = cJSON_GetObjectItem (root, "retCode");
        if (retCode && retCode->valueint == 0) {
            status = 0;
        }

        cJSON_Delete (root);
    }

    return (void*)status;
}

void* AsyncTaskUpdateStatus::execute_task (pthread_t thread_id, int task_id, DWORD add_data)
{
    INT_PTR status = -1;
    char* res = jbus_invoke ("cbplus.am", "updateStatus", "{}");

    if (res == NULL) {
        return (void*)status;
    }

    /* convert to C structure here */
    cJSON* root = cJSON_Parse (res);
    jbus_free (res);

    if (root) {
        cJSON* retCode = cJSON_GetObjectItem (root, "retCode");
        if (retCode && retCode->valueint == 0) {
            status = 0;
        }

        cJSON_Delete (root);
    }

    return (void*)status;
}

AsyncTaskUpdateStatus* MiscService::updateStatus (HWND hwnd)
{
    AsyncTaskUpdateStatus* task = new AsyncTaskUpdateStatus ();
    task->createTask (hwnd, TASKID_MISC_UPDATE_STATUS, 0L);
    return task;
}

void* AsyncTaskMUCCtl::execute_task (pthread_t thread_id, int task_id, DWORD add_data)
{
    INT_PTR status = -1;
    const char* cmd;
    char cmd_arg [256];

    if (task_id == TASKID_MISC_START_MUC) {
        time_t curr_time = time (NULL);
        time_t start_time;
        time_t end_time;

        if (m_duration == -1) {
            start_time = -1;
            end_time = -1;
        }
        else {
            start_time = curr_time - m_duration;
            end_time = curr_time;
        }

        cmd = "startTransfer";
        snprintf (cmd_arg, sizeof (cmd_arg) - 1, "{\"startTime\":\"%ld\", \"endTime\":\"%ld\", \"filter\":%d}",
            start_time, end_time,
#ifdef OEM_TPP
            -1
#else
            m_filter
#endif
            );
    }
    else {
        cmd = "stopTransfer";
        strcpy (cmd_arg, "{}");
    }

    char* res = jbus_invoke ("cbplus.app.muc", cmd, cmd_arg);
    if (res == NULL) {
        return (void*)status;
    }

    /* convert to C structure here */
    cJSON* root = cJSON_Parse (res);
    jbus_free (res);

    if (root) {
        cJSON* retCode = cJSON_GetObjectItem (root, "retCode");
        if (retCode && retCode->valueint == 0) {
            status = 0;
        }

        cJSON_Delete (root);
    }

    return (void*)status;
}

AsyncTaskMUCCtl* MiscService::startMUC (HWND hwnd, int duration, int filter)
{
    AsyncTaskMUCCtl* task = new AsyncTaskMUCCtl (duration, filter);
    task->createTask (hwnd, TASKID_MISC_START_MUC, 0L);
    return task;
}

AsyncTaskMUCCtl* MiscService::stopMUC (HWND hwnd)
{
    AsyncTaskMUCCtl* task = new AsyncTaskMUCCtl ();
    task->createTask (hwnd, TASKID_MISC_STOP_MUC, 0L);
    return task;
}

void* AsyncTaskHotspotCtl::execute_task (pthread_t thread_id, int task_id, DWORD add_data)
{
    const char* cmd;
    char cmd_arg [256];

    if (task_id == TASKID_MISC_START_HOTSPOT) {
        cmd = "enableAp";
        snprintf (cmd_arg, sizeof (cmd_arg) - 1, "{\"refreshPassword\":%d}", m_refresh_passwd?1:0);
    }
    else {
        cmd = "disableAp";
        strcpy (cmd_arg, "{}");
    }

    char* res = jbus_invoke ("cbplus.network", cmd, cmd_arg);
    if (res == NULL) {
        return NULL;
    }

    /* convert to C structure here */
    cJSON* root = cJSON_Parse (res);
    jbus_free (res);

    if (root == NULL) {
        return NULL;
    }

    cJSON* retCode = cJSON_GetObjectItem (root, "retCode");
    if (retCode == NULL || retCode->valueint != 0) {
        goto error;
    }

    if (task_id == TASKID_MISC_START_HOTSPOT) {
        cJSON* data = cJSON_GetObjectItem (root, "data");
        if (data == NULL) {
            goto error;
        }

        cJSON* ssid = cJSON_GetObjectItem (data, "ssid");
        if (ssid == NULL || ssid->valuestring == NULL) {
            goto error;
        }
        strncpy (m_hotspot_info.ssid, ssid->valuestring, MAX_LEN_HOTSPOT_SSID);

        cJSON* password = cJSON_GetObjectItem (data, "password");
        if (password == NULL || password->valuestring == NULL) {
            goto error;
        }
        strncpy (m_hotspot_info.passwd, password->valuestring, MAX_LEN_HOTSPOT_PASSWD);

        m_hotspot_info.enabled = true;
    }
    else {
        memset (&m_hotspot_info, 0, sizeof (m_hotspot_info));
    }

    cJSON_Delete (root);
    return &m_hotspot_info;

error:
    cJSON_Delete (root);
    return NULL;
}

AsyncTaskHotspotCtl* MiscService::startHotspot (HWND hwnd, bool refreshPasswd)
{
    AsyncTaskHotspotCtl* task = new AsyncTaskHotspotCtl (refreshPasswd);
    task->createTask (hwnd, TASKID_MISC_START_HOTSPOT, 0L);
    return task;
}

AsyncTaskHotspotCtl* MiscService::stopHotspot (HWND hwnd)
{
    AsyncTaskHotspotCtl* task = new AsyncTaskHotspotCtl ();
    task->createTask (hwnd, TASKID_MISC_STOP_HOTSPOT, 0L);
    return task;
}

bool MiscService::getHotspotStatus(HOTSPOT_INFO* hotspot_info)
{
    char* res = jbus_invoke ("cbplus.network", "getApState", "{}");
    if (res == NULL) {
        return false;
    }

    /* convert to C structure here */
    cJSON* root = cJSON_Parse (res);
    jbus_free (res);

    if (root == NULL) {
        return false;
    }

    cJSON *retCode, *data, *state;
    memset (hotspot_info, 0, sizeof (HOTSPOT_INFO));

    retCode = cJSON_GetObjectItem (root, "retCode");
    if (retCode == NULL || retCode->valueint != 0) {
        goto error;
    }

    data = cJSON_GetObjectItem (root, "data");
    if (data == NULL) {
        goto error;
    }

    state = cJSON_GetObjectItem (data, "state");
    if (state == NULL) {
        goto error;
    }
    if (state->valueint) {
        hotspot_info->enabled = true;
        cJSON* ssid = cJSON_GetObjectItem (data, "ssid");
        if (ssid == NULL || ssid->valuestring == NULL) {
            goto error;
        }
        strncpy (hotspot_info->ssid, ssid->valuestring,
            MAX_LEN_HOTSPOT_SSID);

        cJSON* password = cJSON_GetObjectItem (data, "password");
        if (password == NULL || password->valuestring == NULL) {
            goto error;
        }
        strncpy (hotspot_info->passwd, password->valuestring,
            MAX_LEN_HOTSPOT_PASSWD);
    }
    else {
        hotspot_info->enabled = false;
    }

    cJSON_Delete (root);
    return true;

error:
    cJSON_Delete (root);
    return false;
}

