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
 * @file NetworkService.hh
 * @Synopsis
 * @author Vincent Wei
 * @version 1.0
 * @date 21/05/2018
 */

#ifndef _NETWORK_SERVICE_HH_
#define _NETWORK_SERVICE_HH_

#include <string>
#include <vector>

#include "AsyncTask.hh"

#define TASKID_NETWORK_MAGIC        0x0200
#define TASKID_NETWORK_STATE        (TASKID_NETWORK_MAGIC | 0x0000)
#define TASKID_NETWORK_WIFI_LIST    (TASKID_NETWORK_MAGIC | 0x0001)
#define TASKID_NETWORK_CONN_WIFI    (TASKID_NETWORK_MAGIC | 0x0002)
#define TASKID_NETWORK_ENABLE_LTE   (TASKID_NETWORK_MAGIC | 0x0003)
#define TASKID_NETWORK_ENABLE_WIFI  (TASKID_NETWORK_MAGIC | 0x0004)

#define NETWORK_TYPE_MOBILE         0
#define NETWORK_TYPE_WIFI           1
#define NETWORK_TYPE_BLUETOOTH      2
#define NETWORK_TYPE_ETHERNE        3

#define NETWORK_STATE_CONNECTED         0
#define NETWORK_STATE_CONNECTING        1
#define NETWORK_STATE_SUSPENDED         2
#define NETWORK_STATE_DISCONNECTING     3
#define NETWORK_STATE_DISCONNECTED      4
#define NETWORK_STATE_NO_MEDIA          5
#define NETWORK_STATE_DISABLED          6
#define NETWORK_STATE_ABSENT            7

#define NETWORK_STATE_UNKNOWN           255

typedef struct _NETWOR_STATE {
    BYTE global;
    BYTE lte;
    BYTE wifi;
    BYTE eth;
    BYTE ble;
    char* curr_ssid;
} NETWORK_STATE;

#define TS_WIFILIST_OK          0
#define TS_WIFILIST_DISABLED    1
#define TS_WIFILIST_BADDATA     2
#define TS_WIFILIST_SYSERROR    3
#define TS_WIFILIST_UNKNOWN     255

struct WifiHotspot
{
    char* ssid;
    bool is_open;
    int signal;

    char* bssid;
    char* capabilities;
    int freq;
};

typedef std::vector<WifiHotspot*> HotspotCollection;

typedef struct _WIFI_HOTSPOTS_INFO {
    int status;
    int current;
    HotspotCollection* hotspots;
} WIFI_HOTSPOTS_INFO;

#define TS_CONN_WIFI_STATUS_OK      0
#define TS_CONN_WIFI_STATUS_NOMEM   1
#define TS_CONN_WIFI_STATUS_SYSERR  2

#define TS_CONN_WIFI_RES_CONNECTED  0
#define TS_CONN_WIFI_RES_BADPASSWD  40012
#define TS_CONN_WIFI_RES_NOTEXISTS  40023
#define TS_CONN_WIFI_RES_UNKNOWNERR -4

typedef struct _WIFI_CONN_INFO {
    char* ssid;
    char* password;
} WIFI_CONN_INFO;

typedef struct _WIFI_CONN_RESULT {
    int status;
    int result;
} WIFI_CONN_RESULT;

class AsyncTaskGetNetworkState : public AsyncTask
{
public:
    virtual void* execute_task (pthread_t thread_id, int task_id, DWORD add_data);
    virtual void cleanup_task (pthread_t thread_id, void* res);
};

class AsyncTaskGetHotspotList : public AsyncTask
{
public:
    WIFI_HOTSPOTS_INFO* m_info;

    virtual void* execute_task (pthread_t thread_id, int task_id, DWORD add_data);

    AsyncTaskGetHotspotList () : m_info (NULL) {}
    ~AsyncTaskGetHotspotList ();
};

class AsyncTaskConnectHotspot : public AsyncTask
{
public:
    WIFI_CONN_INFO m_conn_info;

    virtual void* execute_task (pthread_t thread_id, int task_id, DWORD add_data);
    virtual void cleanup_task (pthread_t thread_id, void* res);

    AsyncTaskConnectHotspot () {
        m_conn_info.ssid = NULL;
        m_conn_info.password = NULL;
    }

    ~AsyncTaskConnectHotspot () {
        if (m_conn_info.ssid) free (m_conn_info.ssid);
        if (m_conn_info.password) free (m_conn_info.password);
    }
};

#define TS_ENABLE_LTE_STATUS_OK         0
#define TS_ENABLE_LTE_STATUS_SYSERR     1
#define TS_ENABLE_LTE_STATUS_UNKNOWN    2

class AsyncTaskEnableLTE : public AsyncTask
{
public:
    virtual void* execute_task (pthread_t thread_id, int task_id, DWORD add_data);
    virtual void cleanup_task (pthread_t thread_id, void* res) {}

    AsyncTaskEnableLTE () { }
    ~AsyncTaskEnableLTE () { }
};

#define TS_ENABLE_WIFI_STATUS_OK         0
#define TS_ENABLE_WIFI_STATUS_SYSERR     1
#define TS_ENABLE_WIFI_STATUS_UNKNOWN    2

class AsyncTaskEnableWiFi : public AsyncTask
{
public:
    virtual void* execute_task (pthread_t thread_id, int task_id, DWORD add_data);
    virtual void cleanup_task (pthread_t thread_id, void* res) { }

    AsyncTaskEnableWiFi () { }
    ~AsyncTaskEnableWiFi () { }
};

class NetworkService
{
public:
    static NetworkService* singleton ();

    AsyncTaskGetNetworkState* getInstantState (HWND hwnd);
    AsyncTaskGetHotspotList* getWifiList (HWND hwnd);
    AsyncTaskConnectHotspot* connectHotspot (HWND hwnd, const char* ssid, const char* password);
    AsyncTaskEnableLTE* enableLTE (HWND hwnd, BOOL enable);
    AsyncTaskEnableWiFi* enableWiFi (HWND hwnd, BOOL enable);

    const NETWORK_STATE* getState () {
        return &m_ns;
    }

    void onStateChanged (const char* event_msg);

    void onWiFiStateChanged (int state, const char* ssid) {
        m_ns.wifi = state;
        if (m_ns.curr_ssid) {
            free (m_ns.curr_ssid);
            m_ns.curr_ssid = NULL;
        }

        if (ssid)
            m_ns.curr_ssid = strdup (ssid);
    }

    void onStateChanged (const NETWORK_STATE* new_ns)
    {
        m_ns.global = new_ns->global;
        m_ns.lte = new_ns->lte;
        m_ns.wifi = new_ns->wifi;
        m_ns.eth = new_ns->eth;
        m_ns.ble = new_ns->ble;

        if (m_ns.curr_ssid) {
            free (m_ns.curr_ssid);
            m_ns.curr_ssid = NULL;
        }

        if (new_ns->curr_ssid)
            m_ns.curr_ssid = strdup (new_ns->curr_ssid);
    }

private:
    NETWORK_STATE m_ns;

    NetworkService ();
    ~NetworkService ();

    static NetworkService* s_service;

    class Gc
    {
    public:
        ~Gc() {
            if (NetworkService::s_service)
                delete NetworkService::s_service;
        }
    };

    static Gc gc;
};

#define NETWORKSERVICE NetworkService::singleton()

#endif

