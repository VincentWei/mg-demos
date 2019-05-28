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
 * @file FirmwareService.hh
 * @Synopsis
 * @author Vincent Wei
 * @version 1.0
 * @date 24/05/2018
 */

#ifndef _FIRMWARE_SERVICE_HH_
#define _FIRMWARE_SERVICE_HH_

#include <string>
#include <vector>

#include "AsyncTask.hh"

#define TASKID_FIRMWARE_MAGIC       0x0100
#define TASKID_FIRMWARE_CHECK       (TASKID_FIRMWARE_MAGIC | 0x0000)
#define TASKID_FIRMWARE_DOWNLOAD    (TASKID_FIRMWARE_MAGIC | 0x0001)
#define TASKID_FIRMWARE_UPGRADE     (TASKID_FIRMWARE_MAGIC | 0x0002)

#define LEN_VER_NAME                16
#define LEN_URL_PATH                1024

#define FWS_OK                      0
#define FWS_ERR_NETWORK             1
#define FWS_ERR_DISK_FULL           2
#define FWS_ERR_LOW_BATTERY         3
#define FWS_ERR_NO_EXT_POWER        4
#define FWS_ERR_UPTODATE            5
#define FWS_ERR_SYSTEM              6

typedef struct _FIRMWARE_INFO {
    int status;

    unsigned size;
    int verCode;
    char* verName;
    char* verDesc;
    char* url;
    char* md5;
} FIRMWARE_INFO;

class AsyncTaskCheckFirmware : public AsyncTask
{
public:
    virtual void* execute_task (pthread_t thread_id, int task_id, DWORD add_data);
    virtual void cleanup_task (pthread_t thread_id, void* res);

private:
    FIRMWARE_INFO m_info;
};

class AsyncTaskDownloadFirmware : public AsyncTask
{
public:
    virtual void* execute_task (pthread_t thread_id, int task_id, DWORD add_data);
    virtual void cleanup_task (pthread_t thread_id, void* res);

private:
    FIRMWARE_INFO m_info;
};

class AsyncTaskUpgradeFirmware : public AsyncTask
{
public:
    virtual void* execute_task (pthread_t thread_id, int task_id, DWORD add_data);
    virtual void cleanup_task (pthread_t thread_id, void* res);

private:
    FIRMWARE_INFO m_info;
};

class FirmwareService
{
public:
    static FirmwareService* singleton ();

    void checkFirmware (HWND hwnd);
    void downloadFirmware (HWND hwnd, const char* url);
    void upgradeFirmware (HWND hwnd, const char* path);

private:

    FirmwareService ();
    ~FirmwareService ();

    static FirmwareService* s_service;

    class Gc
    {
    public:
        ~Gc() {
            if (FirmwareService::s_service)
                delete FirmwareService::s_service;
        }
    };

    static Gc gc;
};

#define FIRMWARESERVICE FirmwareService::singleton()

#endif

