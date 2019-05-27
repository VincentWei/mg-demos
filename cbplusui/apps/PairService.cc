/*!============================================================================
 * @file PairService.cc
 * @Synopsis  
 * @author Vincent Wei
 * @version 1.0
 * @date 17/05/2018
 *
 * Copyright (C) 2018 Beijing Joobot Technologies Co., Ltd.
 */

#include <unistd.h>
#include <jbus.h>

#include "global.h"
#include "resource.h"

#include "cJSON.h"
#include "PairService.hh"

PairService* PairService::s_service = NULL;

// get the single instance of PairService
PairService* PairService::singleton()
{
    if (NULL == PairService::s_service) {
        PairService::s_service = new PairService();
    }
    return PairService::s_service;
}

// private:
PairService::PairService ()
{
}

PairService::~PairService ()
{
}

AsyncTaskPairCheck* PairService::checkPair (HWND hwnd)
{
    AsyncTaskPairCheck* task = new AsyncTaskPairCheck ();

    task->createTask (hwnd, TASKID_PAIR_CHECK, 0);

    return task;
}

AsyncTaskPairGetCode* PairService::getCode (HWND hwnd)
{
    AsyncTaskPairGetCode* task = new AsyncTaskPairGetCode ();

    task->createTask (hwnd, TASKID_PAIR_GETCODE, 0);

    return task;
}

AsyncTaskPairExecute* PairService::executePair (HWND hwnd, const char* code)
{
    AsyncTaskPairExecute* task = new AsyncTaskPairExecute ();

    task->createTask (hwnd, TASKID_PAIR_EXECUTE, (DWORD)code);

    return task;
}

void* AsyncTaskPairCheck::execute_task (pthread_t thread_id, int task_id, DWORD add_data)
{
    PAIR_INFO* info = new PAIR_INFO;
    memset (info, 0, sizeof (PAIR_INFO));

    char* res = jbus_invoke ("cbplus.bind", "getBindState", "{}");
    if (res == NULL) {
        return info;
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

	    cJSON* data = cJSON_GetObjectItem (root, "data");
        if (data == NULL) {
            goto error;
        }

	    cJSON* bindState = cJSON_GetObjectItem (data, "bind");
        if (bindState == NULL) {
            goto error;
        }

        if (bindState->valueint == 0) {
            info->status = PS_CHECK_UNPAIRED;
            goto error;
        }

	    cJSON* userId = cJSON_GetObjectItem (data, "userId");
        if (userId == NULL) {
            goto error;
        }

        info->user_id = strdup (userId->valuestring);
    }

error:
    cJSON_Delete (root);

    return (void*) info;
}

void AsyncTaskPairCheck::cleanup_task (pthread_t thread_id, void* res)
{
    PAIR_INFO* info = (PAIR_INFO*)res;

    if (info->user_id) {
        free (info->user_id);
    }

    delete info;
    delete this;
}

void* AsyncTaskPairGetCode::execute_task (pthread_t thread_id, int task_id, DWORD add_data)
{
    PAIR_INFO* info = new PAIR_INFO;
    memset (info, 0, sizeof (PAIR_INFO));

    char* res = jbus_invoke ("cbplus.bind", "getBindCode", "{}");
    if (res == NULL) {
        return info;
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

	    cJSON* data = cJSON_GetObjectItem (root, "data");
        if (data == NULL) {
            goto error;
        }

	    cJSON* bindCode = cJSON_GetObjectItem (data, "code");
        if (bindCode == NULL) {
            goto error;
        }
        info->code = strdup (bindCode->valuestring);

	    cJSON* filePath = cJSON_GetObjectItem (data, "filepath");
        if (filePath == NULL) {
            goto error;
        }

        info->status = PS_GETCODE_OK;
        info->qrcode_path = strdup (filePath->valuestring);
    }

error:
    cJSON_Delete (root);

    return (void*) info;
}

void AsyncTaskPairGetCode::cleanup_task (pthread_t thread_id, void* res)
{
    PAIR_INFO* info = (PAIR_INFO*)res;

    if (info->code) {
        free (info->code);
    }

    if (info->qrcode_path) {
        free (info->qrcode_path);
    }

    delete info;
    delete this;
}

bool AsyncTaskPairExecute::init_task (pthread_t thread_id, int task_id, DWORD add_data)
{
    snprintf (m_args, sizeof (m_args) - 1, "{\"code\":\"%s\"}", (const char*)add_data);
    return true;
}

void* AsyncTaskPairExecute::execute_task (pthread_t thread_id, int task_id, DWORD add_data)
{
    PAIR_INFO* info = new PAIR_INFO;
    memset (info, 0, sizeof (PAIR_INFO));

    int count_retries = 0;
    while (true) {

        if (count_retries == 20) {
            info->status = PS_EXECUTE_TIMEOUT;
            break;
        }

        /* sleep 700ms */
        usleep (700*1000);

        char* res = jbus_invoke ("cbplus.bind", "getBindCodeState", m_args);
        if (res == NULL) {
            return info;
        }

        /* convert to C structure here */
        cJSON* root = cJSON_Parse (res);
        jbus_free (res);

        if (root) {
            cJSON* retCode = cJSON_GetObjectItem (root, "retCode");
            if (retCode == NULL) {
                cJSON_Delete (root);
                break;
            }

            if (retCode->valueint == 0) {
                info->status = PS_EXECUTE_OK;

                cJSON* data = cJSON_GetObjectItem (root, "data");
                if (data != NULL) {
                    cJSON* userId = cJSON_GetObjectItem (data, "userId");
                    if (userId != NULL && userId->type == cJSON_String) {
                        info->user_id = strdup (userId->valuestring);
                    }

                    cJSON* nickname = cJSON_GetObjectItem (data, "nickname");
                    if (nickname != NULL && nickname->type == cJSON_String) {
                        info->nickname = strdup (nickname->valuestring);
                    }
                }

                cJSON_Delete (root);
                break;
            }

            cJSON_Delete (root);
        }

        count_retries ++;
    }

    return (void*) info;
}

void AsyncTaskPairExecute::cleanup_task (pthread_t thread_id, void* res)
{
    PAIR_INFO* info = (PAIR_INFO*)res;

    if (info->user_id) {
        free (info->user_id);
    }

    if (info->nickname) {
        free (info->nickname);
    }

    delete info;
    delete this;
}

