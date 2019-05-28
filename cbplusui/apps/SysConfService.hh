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
 * @file SysConfService.hh
 * @Synopsis
 * @author Vincent Wei
 * @version 1.0
 * @date 25/05/2018
 */

#ifndef _SYSCONF_SERVICE_HH_
#define _SYSCONF_SERVICE_HH_

#include <string>
#include <vector>

#include "AsyncTask.hh"

#define TASKID_SYSCONF_MAGIC        0x0400
#define TASKID_SYSCONF_GETALL       (TASKID_SYSCONF_MAGIC | 0x0000)
#define TASKID_SYSCONF_RESET        (TASKID_SYSCONF_MAGIC | 0x0001)
#define TASKID_SYSCONF_SETFTP       (TASKID_SYSCONF_MAGIC | 0x0002)

#define TASKSTATE_SYSTEM_ERR    0
#define TASKSTATE_OK            1

class AsyncTaskSysConf : public AsyncTask
{
public:
    virtual void* execute_task (pthread_t thread_id, int task_id, DWORD add_data);
    virtual void cleanup_task (pthread_t thread_id, void* res);
};

#define SYS_RESET_OK            0
#define SYS_RESET_ERR_LOWPOWER  1
#define SYS_RESET_ERR_GENERAL   2

class AsyncTaskSysReset : public AsyncTask
{
public:
    virtual void* execute_task (pthread_t thread_id, int task_id, DWORD add_data);
    virtual void cleanup_task (pthread_t thread_id, void* res);

private:
    int m_status;
};

#define SYS_FTP_OK              0
#define SYS_FTP_ERR_CONNECT     1
#define SYS_FTP_ERR_MKDIR       2
#define SYS_FTP_ERR_CD          3
#define SYS_FTP_ERR_PUT         4
#define SYS_FTP_ERR_DELETE      5
#define SYS_FTP_ERR_RMDIR       6
#define SYS_FTP_ERR_UNKNOWN     7

typedef struct _FTP_SERVER_INFO {
    char* host;
    char* port;
    char* user;
    char* password;
} FTP_SERVER_INFO;

class AsyncTaskSysSetFtp : public AsyncTask
{
public:
    FTP_SERVER_INFO m_ftp_info;

    virtual void* execute_task (pthread_t thread_id, int task_id, DWORD add_data);
    virtual void cleanup_task (pthread_t thread_id, void* res);

private:
    int m_status;
};

#define UPLOAD_FILTER_JPEG          1
#define UPLOAD_FILTER_RAW           2
#define UPLOAD_FILTER_JPEG_RAW      3
#define UPLOAD_FILTER_JPEG_IN_RAW   4
#define UPLOAD_FILTER_SMALL_JPEG    5

typedef struct _UPLOAD_OPTIONS {
    int filter;
    bool autoDel;
    bool writeLoc;
} UPLOAD_OPTIONS;

struct cJSON;

#define SYS_TRANSFER_PROTOCOL_JOOPIC    0
#define SYS_TRANSFER_PROTOCOL_FTP       1
#define SYS_TRANSFER_PROTOCOL_MAX       1

class SysConfService
{
public:
    static SysConfService* singleton ();

    bool parseConfig (cJSON* root);

    /* async calls */
    AsyncTaskSysConf* getAll (HWND hwnd);
    void resetDevice (HWND hwnd);
    void setFtpAccount (HWND hwnd, const char* host, const char* port,
            const char* user, const char* password);

    /* sync calls */
    const bool refreshUserInfo ();
    const UPLOAD_OPTIONS* getAUCOptions (bool cached);
    const UPLOAD_OPTIONS* getMUCOptions (bool cached);
    bool setAUCOptionFilter (int filter);
    bool setAUCOptionAutoDel (bool autoDel);
    bool setAUCOptionWriteLoc (bool writeLoc);
    bool setMUCOptionFilter (int filter);
    bool setMUCOptionAutoDel (bool autoDel);

    bool setTransferMode (int transfer_mode);
    const FTP_SERVER_INFO* getFtpAccount (bool cached);

    bool setSysTimeZone (const char* tz);

    const char* getDeviceName () {
        return m_device_name;
    };

    const char* getDeviceModel () {
        return m_device_model;
    };

    const char* getHardwareId () {
        return m_hardware_id;
    };

    const char* getSerialNumber () {
        return m_serial_number;
    };

    const char* getVersionNumber () {
        return m_version_number;
    };

    const int getVersionCode () {
        return m_version_code;
    };

    const char* getBoundUserId () {
        return m_bound_user_id;
    };

    const char* getBoundJoobotId () {
        return m_bound_joobot_id;
    };

    const char* getBoundUserNickname () {
        return m_bound_user_nickname;
    };

    const char* getFtpServerHostname () {
        return m_ftp_info.host;
    };

    const FTP_SERVER_INFO* getFtpServerInfo () {
        return &m_ftp_info;
    };

    int getTransferProtocol () {
        return m_transfer_protocol;
    };

    void setBoundUserId (const char* userId) {
        if (m_bound_user_id)
            free (m_bound_user_id);

        if (userId) {
            m_bound_user_id = strdup (userId);
        }
        else {
            m_bound_user_id = NULL;
        }
    }

    void setBoundUserNickname (const char* userNickname) {
        if (m_bound_user_nickname)
            free (m_bound_user_nickname);

        if (userNickname) {
            m_bound_user_nickname = strdup (userNickname);
        }
        else {
            m_bound_user_nickname = NULL;
        }
    }

    void clearFtpServerInfo() {
        if (m_ftp_info.host) free (m_ftp_info.host);
        if (m_ftp_info.port) free (m_ftp_info.port);
        if (m_ftp_info.user) free (m_ftp_info.user);
        if (m_ftp_info.password) free (m_ftp_info.password);

        memset (&m_ftp_info, 0, sizeof(FTP_SERVER_INFO));
    }

protected:

    char* m_device_name;
    char* m_device_model;
    char* m_hardware_id;
    char* m_serial_number;
    int   m_version_code;
    char* m_version_number;
    char* m_bound_user_id;
    char* m_bound_joobot_id;
    char* m_bound_user_nickname;

    UPLOAD_OPTIONS m_auc_opts;
    UPLOAD_OPTIONS m_muc_opts;

    int   m_transfer_protocol;
    FTP_SERVER_INFO m_ftp_info;

    friend class AsyncTaskSysConf;

private:
    SysConfService () {
        m_device_name = NULL;
        m_device_model = NULL;
        m_hardware_id = NULL;
        m_serial_number = NULL;
        m_version_code = 0;
        m_version_number = NULL;
        m_bound_user_id = NULL;
        m_bound_joobot_id = NULL;
        m_bound_user_nickname = NULL;

        m_auc_opts.filter = 1;
        m_muc_opts.filter = 1;

        m_transfer_protocol = SYS_TRANSFER_PROTOCOL_JOOPIC;
        memset (&m_ftp_info, 0, sizeof(FTP_SERVER_INFO));
    }

    ~SysConfService () {
        if (m_device_name) free (m_device_name);
        if (m_device_model) free (m_device_model);
        if (m_hardware_id) free (m_hardware_id);
        if (m_serial_number) free (m_serial_number);
        if (m_version_number) free (m_version_number);
        if (m_bound_user_id) free (m_bound_user_id);
        if (m_bound_joobot_id) free (m_bound_joobot_id);
        if (m_bound_user_nickname) free (m_bound_user_nickname);

        if (m_ftp_info.host) free (m_ftp_info.host);
        if (m_ftp_info.port) free (m_ftp_info.port);
        if (m_ftp_info.user) free (m_ftp_info.user);
        if (m_ftp_info.password) free (m_ftp_info.password);
    }

    static SysConfService* s_service;

    class Gc
    {
    public:
        ~Gc() {
            if (SysConfService::s_service)
                delete SysConfService::s_service;
        }
    };

    static Gc gc;
};

#define SYSCFGSERVICE SysConfService::singleton()

#endif /* !_SYSCONF_SERVICE_HH_ */

