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
 * @file SysConfService.cc
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
#include "SysConfService.hh"

AsyncTaskSysConf* SysConfService::getAll (HWND hwnd)
{
    AsyncTaskSysConf* task = new AsyncTaskSysConf ();

    task->createTask (hwnd, TASKID_SYSCONF_GETALL, (DWORD)this);

    return task;
}

void* AsyncTaskSysConf::execute_task (pthread_t thread_id, int task_id, DWORD add_data)
{
    SysConfService* service = (SysConfService*)add_data;

    char* res = jbus_invoke ("cbplus.config", "getAllConfig", "{}");
    if (res == NULL) {
        return NULL;
    }

    /* convert to C structure here */
    cJSON* root = cJSON_Parse (res);
    jbus_free (res);

    if (service->parseConfig (root)) {
        cJSON_Delete (root);
        return (void*)-1;
    }

    cJSON_Delete (root);
    return NULL;
}

void AsyncTaskSysConf::cleanup_task (pthread_t thread_id, void* res)
{
    delete this;
}

SysConfService* SysConfService::s_service = NULL;

// get the single instance of SysConfService
SysConfService* SysConfService::singleton()
{
    if (NULL == SysConfService::s_service) {
        SysConfService::s_service = new SysConfService();
    }
    return SysConfService::s_service;
}

bool SysConfService::parseConfig (cJSON* root)
{
    if (root == NULL) {
        goto error;
    }
    else {
        cJSON* retCode = cJSON_GetObjectItem (root, "retCode");
        if (retCode == NULL || retCode->valueint != 0) {
            goto error;
        }

        cJSON* data = cJSON_GetObjectItem (root, "data");
        if (data == NULL) {
            goto error;
        }

        cJSON* config = cJSON_GetObjectItem (data, "config");
        if (config == NULL) {
            goto error;
        }

        cJSON* item = config->child;
        while (item) {
            cJSON* module = cJSON_GetObjectItem (item, "module");
            cJSON* key = cJSON_GetObjectItem (item, "key");
            cJSON* value = cJSON_GetObjectItem (item, "value");
            if (module == NULL || module->type != cJSON_String
                    || key == NULL || key->type != cJSON_String
                    || value == NULL) {
                goto error;
            }

            if (strcasecmp (module->valuestring, "device") == 0) {
                if (strcasecmp (key->valuestring, "name") == 0) {
                    if (value->type != cJSON_String) {
                        goto error;
                    }

                    if (m_device_name)
                        free(m_device_name);
                    m_device_name = strdup (value->valuestring);
                }
                else if (strcasecmp (key->valuestring, "hardwareId") == 0) {
                    if (value->type != cJSON_String) {
                        goto error;
                    }

                    if (m_hardware_id)
                        free (m_hardware_id);
                    m_hardware_id = strdup (value->valuestring);
                }
                else if (strcasecmp (key->valuestring, "sn") == 0) {
                    if (value->type != cJSON_String) {
                        goto error;
                    }

                    if (m_serial_number)
                        free (m_serial_number);
                    m_serial_number = strdup (value->valuestring);
                }
                else if (strcasecmp (key->valuestring, "model") == 0) {
                    if (value->type != cJSON_String) {
                        goto error;
                    }

                    if (m_device_model)
                        free (m_device_model);
                    m_device_model = strdup (value->valuestring);
                }
            }
            else if (strcasecmp (module->valuestring, "os") == 0) {
                if (strcasecmp (key->valuestring, "verCode") == 0) {
                    if (value->type != cJSON_String) {
                        goto error;
                    }

                    m_version_code = atoi (value->valuestring);
                }
                else if (strcasecmp (key->valuestring, "verName") == 0) {
                    if (value->type != cJSON_String) {
                        goto error;
                    }

                    if (m_version_number)
                        free (m_version_number);
                    m_version_number = strdup (value->valuestring);
                }
                else if (strcasecmp (key->valuestring, "timeZone") == 0) {
                    if (value->type != cJSON_String) {
                        goto error;
                    }

                    _DBG_PRINTF ("System time zone: %s\n", value->valuestring);
                }
            }
            else if (strcasecmp (module->valuestring, "user") == 0) {
                if (strcasecmp (key->valuestring, "userId") == 0) {
                    if (value->type != cJSON_String) {
                        goto error;
                    }

                    if (m_bound_user_id)
                        free (m_bound_user_id);
                    m_bound_user_id = strdup (value->valuestring);
                }
                else if (strcasecmp (key->valuestring, "joobotId") == 0) {
                    if (value->type != cJSON_String) {
                        goto error;
                    }

                    if (m_bound_joobot_id)
                        free (m_bound_joobot_id);
                    m_bound_joobot_id = strdup (value->valuestring);
                }
                else if (strcasecmp (key->valuestring, "nickname") == 0) {
                    if (value->type != cJSON_String) {
                        goto error;
                    }

                    if (m_bound_user_nickname)
                        free (m_bound_user_nickname);
                    m_bound_user_nickname = strdup (value->valuestring);
                }
                else if (strcasecmp (key->valuestring, "transferMode") == 0) {
                    if (value->type != cJSON_String) {
                        goto error;
                    }

                    m_transfer_protocol = atoi (value->valuestring);
                }
            }
            else if (strcasecmp (module->valuestring, "autoUploadCloud") == 0) {
                if (strcasecmp (key->valuestring, "filter") == 0) {
                    if (value->type != cJSON_String) {
                        goto error;
                    }

                    m_auc_opts.filter = atoi (value->valuestring);
                }
                else if (strcasecmp (key->valuestring, "autoDel") == 0) {
                    if (value->type != cJSON_String) {
                        goto error;
                    }

                    m_auc_opts.autoDel = atoi (value->valuestring);
                }
                else if (strcasecmp (key->valuestring, "writeLoc") == 0) {
                    if (value->type != cJSON_String) {
                        goto error;
                    }

                    m_auc_opts.writeLoc = atoi (value->valuestring);
                }
            }
            else if (strcasecmp (module->valuestring, "manualUploadCloud") == 0) {
                if (strcasecmp (key->valuestring, "filter") == 0) {
                    if (value->type != cJSON_String) {
                        goto error;
                    }

                    m_muc_opts.filter = atoi (value->valuestring);
                }
                else if (strcasecmp (key->valuestring, "autoDel") == 0) {
                    if (value->type != cJSON_String) {
                        goto error;
                    }

                    m_muc_opts.autoDel = atoi (value->valuestring);
                }
                else if (strcasecmp (key->valuestring, "writeLoc") == 0) {
                    if (value->type != cJSON_String) {
                        goto error;
                    }

                    m_muc_opts.writeLoc = atoi (value->valuestring);
                }
            }
            else if (strcasecmp (module->valuestring, "ftp") == 0) {
                if (strcasecmp (key->valuestring, "host") == 0) {
                    if (value->type != cJSON_String) {
                        goto error;
                    }

                    if (m_ftp_info.host)
                        free (m_ftp_info.host);
                    m_ftp_info.host = strdup (value->valuestring);
                }
                else if (strcasecmp (key->valuestring, "port") == 0) {
                    if (value->type != cJSON_String) {
                        goto error;
                    }

                    if (m_ftp_info.port)
                        free (m_ftp_info.port);
                    m_ftp_info.port = strdup (value->valuestring);
                }
                else if (strcasecmp (key->valuestring, "user") == 0) {
                    if (value->type != cJSON_String) {
                        goto error;
                    }

                    if (m_ftp_info.user)
                        free (m_ftp_info.user);
                    m_ftp_info.user = strdup (value->valuestring);
                }
                else if (strcasecmp (key->valuestring, "password") == 0) {
                    if (value->type != cJSON_String) {
                        goto error;
                    }

                    if (m_ftp_info.password)
                        free (m_ftp_info.password);
                    m_ftp_info.password = strdup (value->valuestring);
                }
            }


            item = item->next;
        }
    }

    return true;

error:
    return false;
}

const bool SysConfService::refreshUserInfo ()
{
    char* res = jbus_invoke ("cbplus.config", "getModuleConfig", "{\"module\":\"user\"}");
    if (res == NULL) {
        return false;
    }

    /* convert to C structure here */
    cJSON* root = cJSON_Parse (res);
    jbus_free (res);

    if (parseConfig (root)) {
        cJSON_Delete (root);
        return true;
    }

    cJSON_Delete (root);
    return false;
}

const UPLOAD_OPTIONS* SysConfService::getAUCOptions (bool cached)
{
    if (cached) {
        return &m_auc_opts;
    }

    char* res = jbus_invoke ("cbplus.config", "getModuleConfig", "{\"module\":\"autoUploadCloud\"}");
    if (res == NULL) {
        return NULL;
    }

    /* convert to C structure here */
    cJSON* root = cJSON_Parse (res);
    jbus_free (res);

    if (parseConfig (root)) {
        cJSON_Delete (root);
        return &m_auc_opts;
    }

    cJSON_Delete (root);
    return NULL;
}

const UPLOAD_OPTIONS* SysConfService::getMUCOptions (bool cached)
{
    if (cached) {
        return &m_muc_opts;
    }

    char* res = jbus_invoke ("cbplus.config", "getModuleConfig", "{\"module\":\"autoUploadCloud\"}");
    if (res == NULL) {
        return NULL;
    }

    /* convert to C structure here */
    cJSON* root = cJSON_Parse (res);
    jbus_free (res);

    if (parseConfig (root)) {
        cJSON_Delete (root);
        return &m_muc_opts;
    }

    cJSON_Delete (root);
    return NULL;
}

static bool set_config (const char* opt)
{
    char* res = jbus_invoke ("cbplus.config", "setConfig", opt);
    if (res == NULL) {
        return false;
    }

    /* convert to C structure here */
    cJSON* root = cJSON_Parse (res);
    jbus_free (res);

    if (root == NULL) {
        goto error;
    }
    else {
        cJSON* retCode = cJSON_GetObjectItem (root, "retCode");
        if (retCode == NULL || retCode->valueint != 0)
            goto error;

        cJSON_Delete (root);
        return true;
    }

error:
    cJSON_Delete (root);
    return false;
}

bool SysConfService::setAUCOptionFilter (int filter)
{
    char opt [128];
    snprintf (opt, 127, "{\"module\":\"autoUploadCloud\", \"key\":\"filter\", \"value\":\"%d\"}", filter);

    if (set_config (opt)) {
        m_auc_opts.filter = filter;
        return true;
    }

    return false;
}

bool SysConfService::setAUCOptionAutoDel (bool autoDel)
{
    char opt [128];
    snprintf (opt, 127, "{\"module\":\"autoUploadCloud\", \"key\":\"autoDel\", \"value\":\"%d\"}", autoDel?1:0);

    if (set_config (opt)) {
        m_auc_opts.autoDel = autoDel;
        return true;
    }

    return false;
}

bool SysConfService::setAUCOptionWriteLoc (bool writeLoc)
{
    char opt [128];
    snprintf (opt, 127, "{\"module\":\"autoUploadCloud\", \"key\":\"writeLoc\", \"value\":\"%d\"}", writeLoc?1:0);

    if (set_config (opt)) {
        m_auc_opts.writeLoc = writeLoc;
        return true;
    }

    return false;
}

bool SysConfService::setMUCOptionFilter (int filter)
{
    char opt [128];
    snprintf (opt, 127, "{\"module\":\"manualUploadCloud\", \"key\":\"filter\", \"value\":\"%d\"}", filter);

    if (set_config (opt)) {
        m_muc_opts.filter = filter;
        return true;
    }

    return false;
}

bool SysConfService::setMUCOptionAutoDel (bool autoDel)
{
    char opt [128];
    snprintf (opt, 127, "{\"module\":\"manualUploadCloud\", \"key\":\"autoDel\", \"value\":\"%d\"}", autoDel?1:0);

    if (set_config (opt)) {
        m_muc_opts.autoDel = autoDel;
        return true;
    }

    return false;
}

bool SysConfService::setSysTimeZone (const char* tz)
{
    char* buf = strdup (tz);
    /* trim the postfix */
    char* ch = buf;
    while (*ch) {
        if (*ch == '#') {
            *ch = '\0';
            break;
        }
        ch++;
    }

    char opt [256];
    snprintf (opt, 255, "{\"module\":\"os\", \"key\":\"timeZone\", \"value\":\"%s\"}", buf);
    free (buf);

    if (set_config (opt)) {
        return true;
    }

    return false;
}

bool SysConfService::setTransferMode (int transfer_mode)
{
    char opt [128];
    snprintf (opt, 127, "{\"module\":\"user\", \"key\":\"transferMode\", \"value\":\"%d\"}", transfer_mode);

    if (set_config (opt)) {
        m_transfer_protocol = transfer_mode;
        return true;
    }

    return false;
}

const FTP_SERVER_INFO* SysConfService::getFtpAccount (bool cached)
{
    if (cached) {
        return &m_ftp_info;
    }

    char* res = jbus_invoke ("cbplus.config", "getModuleConfig", "{\"module\":\"ftp\"}");
    if (res == NULL) {
        return NULL;
    }

    /* convert to C structure here */
    cJSON* root = cJSON_Parse (res);
    jbus_free (res);

    if (parseConfig (root)) {
        cJSON_Delete (root);
        return &m_ftp_info;
    }

    cJSON_Delete (root);
    return NULL;
}

void SysConfService::resetDevice (HWND hwnd)
{
    AsyncTaskSysReset* task = new AsyncTaskSysReset ();

    task->createTask (hwnd, TASKID_SYSCONF_RESET, 0);
}

void* AsyncTaskSysReset::execute_task (pthread_t thread_id, int task_id, DWORD add_data)
{
    m_status = SYS_RESET_ERR_GENERAL;

    char* res = jbus_invoke ("cbplus.config", "clear", "{}");
    if (res == NULL) {
        return (void*) &m_status;
    }

    /* convert to C structure here */
    cJSON* root = cJSON_Parse (res);
    jbus_free (res);

    if (root == NULL) {
        goto error;
    }
    else {
        cJSON* retCode = cJSON_GetObjectItem (root, "retCode");
        if (retCode == NULL || retCode->valueint != 0) {
            goto error;
        }
    }

    m_status = SYS_RESET_OK;

error:
    cJSON_Delete (root);
    return (void*) &m_status;
}

void AsyncTaskSysReset::cleanup_task (pthread_t thread_id, void* res)
{
    delete this;
}

void SysConfService::setFtpAccount (HWND hwnd,
            const char* host, const char* port,
            const char* user, const char* password)
{
    AsyncTaskSysSetFtp* task = new AsyncTaskSysSetFtp ();

    task->m_ftp_info.host = strdup(host);
    task->m_ftp_info.port = strdup(port);
    task->m_ftp_info.user = strdup(user);
    task->m_ftp_info.password = strdup(password);

    task->createTask (hwnd, TASKID_SYSCONF_SETFTP, 0);
}

void* AsyncTaskSysSetFtp::execute_task (pthread_t thread_id, int task_id, DWORD add_data)
{
    cJSON* root = cJSON_CreateObject ();
    cJSON_AddStringToObject (root, "host", m_ftp_info.host);
    cJSON_AddStringToObject (root, "port", m_ftp_info.port);
    cJSON_AddStringToObject (root, "user", m_ftp_info.user);
    cJSON_AddStringToObject (root, "password", m_ftp_info.password);

    char* args = cJSON_PrintUnformatted (root);
    _MG_PRINTF ("AsyncTaskSysSetFtp::execute_task > args: %s\n", args);

    m_status = SYS_FTP_ERR_UNKNOWN;
    char* res = jbus_invoke ("cbplus.config", "setFtpConfig", args);
    free (args);

    if (res == NULL) {
        return (void*) &m_status;
    }
    cJSON_Delete (root);

    /* convert to C structure here */
    root = cJSON_Parse (res);
    jbus_free (res);

    if (root == NULL) {
        goto error;
    }
    else {
        cJSON* retCode = cJSON_GetObjectItem (root, "retCode");
        if (retCode == NULL) {
            goto error;
        }

        _DBG_PRINTF("%s: retcode: %d\n", __FUNCTION__, retCode->valueint);

        switch (retCode->valueint) {
        case 0:
            m_status = SYS_RESET_OK;
            break;
        case 40046: //    连接服务器失败
            m_status = SYS_FTP_ERR_CONNECT;
            break;
        case 40047: //    创建目录失败
            m_status = SYS_FTP_ERR_MKDIR;
            break;
        case 40048: //    进入目录失败
            m_status = SYS_FTP_ERR_CD;
            break;
        case 40049: //    上传文件失败
            m_status = SYS_FTP_ERR_PUT;
            break;
        case 40050: //    删除文件失败
            m_status = SYS_FTP_ERR_DELETE;
            break;
        case 40051: //    删除目录失败
            m_status = SYS_FTP_ERR_RMDIR;
            break;
        default:
            m_status = SYS_FTP_ERR_UNKNOWN;
            break;
        }
    }

error:
    cJSON_Delete (root);
    return (void*) &m_status;
}

void AsyncTaskSysSetFtp::cleanup_task (pthread_t thread_id, void* res)
{
    if (m_ftp_info.host)
        free (m_ftp_info.host);
    if (m_ftp_info.port)
        free (m_ftp_info.port);
    if (m_ftp_info.user)
        free (m_ftp_info.user);
    if (m_ftp_info.password)
        free (m_ftp_info.password);

    delete this;
}

