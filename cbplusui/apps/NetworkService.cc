/*!============================================================================
 * @file NetworkService.cc
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
#include "NetworkService.hh"

NetworkService* NetworkService::s_service = NULL;

// get the single instance of NetworkService
NetworkService* NetworkService::singleton()
{
    if (NULL == NetworkService::s_service) {
        NetworkService::s_service = new NetworkService();
    }
    return NetworkService::s_service;
}

// private:
NetworkService::NetworkService ()
{
    memset (&m_ns, NETWORK_STATE_UNKNOWN, sizeof (NETWORK_STATE));
    m_ns.curr_ssid = NULL;
}

NetworkService::~NetworkService ()
{
}

void NetworkService::onStateChanged (const char* message)
{
    /* convert to C structure here */
    cJSON* root = cJSON_Parse (message);

    if (root) {
        cJSON* currentSSID = cJSON_GetObjectItem (root, "currentSSID");
        if (currentSSID == NULL || currentSSID->type != cJSON_String) {
            goto error;
        }
        if (currentSSID->valuestring) {
            m_ns.curr_ssid = strdup (currentSSID->valuestring);
        }
        else
            m_ns.curr_ssid = NULL;

        cJSON* globalState = cJSON_GetObjectItem (root, "globalState");
        if (globalState == NULL || globalState->type != cJSON_Number) {
            goto error;
        }
        m_ns.global = (BYTE)globalState->valueint;

        cJSON* subState = cJSON_GetObjectItem (root, "networkInfo");
        if (subState == NULL) {
            goto error;
        }

        cJSON* item = subState->child;
        while (item) {
            cJSON* type = cJSON_GetObjectItem (item, "type");
            cJSON* state = cJSON_GetObjectItem (item, "state");
            if (type == NULL || state == NULL) {
                goto error;
            }

            switch (type->valueint) {
            case NETWORK_TYPE_MOBILE:
                m_ns.lte = (BYTE)state->valueint;
                break;
            case NETWORK_TYPE_WIFI:
                m_ns.wifi = (BYTE)state->valueint;
                break;
            case NETWORK_TYPE_BLUETOOTH:
                m_ns.ble = (BYTE)state->valueint;
                break;
            case NETWORK_TYPE_ETHERNE:
                m_ns.eth = (BYTE)state->valueint;
                break;
            }

            item = item->next;
        }
    }

error:
    if (root)
        cJSON_Delete (root);
}

AsyncTaskGetNetworkState* NetworkService::getInstantState (HWND hwnd)
{
    AsyncTaskGetNetworkState* task = new AsyncTaskGetNetworkState ();

    task->createTask (hwnd, TASKID_NETWORK_STATE, 0);

    return task;
}

AsyncTaskGetHotspotList* NetworkService::getWifiList (HWND hwnd)
{
    AsyncTaskGetHotspotList* task = new AsyncTaskGetHotspotList ();

    task->createTask (hwnd, TASKID_NETWORK_WIFI_LIST, 0);

    return task;
}

AsyncTaskConnectHotspot* NetworkService::connectHotspot (HWND hwnd, const char* ssid, const char* password)
{
    AsyncTaskConnectHotspot* task = new AsyncTaskConnectHotspot ();
    task->m_conn_info.ssid = strdup (ssid);
    task->m_conn_info.password = strdup (password);
    task->createTask (hwnd, TASKID_NETWORK_CONN_WIFI, 0);
    return task;
}

void* AsyncTaskGetNetworkState::execute_task (pthread_t thread_id, int task_id, DWORD add_data)
{
    NETWORK_STATE* ns = new NETWORK_STATE;
    memset (ns, NETWORK_STATE_UNKNOWN, sizeof (NETWORK_STATE));
    ns->curr_ssid = NULL;

    char* res = jbus_invoke ("cbplus.network", "getState", "{}");
    if (res == NULL) {
        return ns;
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

        cJSON* currentSSID = cJSON_GetObjectItem (data, "currentSSID");
        if (currentSSID == NULL) {
            goto error;
        }
        if (currentSSID->valuestring) {
            _MG_PRINTF ("currentSSID: %s\n", currentSSID->valuestring);
            ns->curr_ssid = strdup (currentSSID->valuestring);
        }
        else
            ns->curr_ssid = NULL;

        cJSON* globalState = cJSON_GetObjectItem (data, "globalState");
        if (globalState == NULL) {
            goto error;
        }
        ns->global = (BYTE)globalState->valueint;

        cJSON* subState = cJSON_GetObjectItem (data, "networkInfo");
        if (subState == NULL) {
            goto error;
        }

        cJSON* item = subState->child;
        while (item) {
            cJSON* type = cJSON_GetObjectItem (item, "type");
            cJSON* state = cJSON_GetObjectItem (item, "state");
            if (type == NULL || state == NULL) {
                goto error;
            }

            switch (type->valueint) {
            case NETWORK_TYPE_MOBILE:
                ns->lte = (BYTE)state->valueint;
                break;
            case NETWORK_TYPE_WIFI:
                ns->wifi = (BYTE)state->valueint;
                break;
            case NETWORK_TYPE_BLUETOOTH:
                ns->ble = (BYTE)state->valueint;
                break;
            case NETWORK_TYPE_ETHERNE:
                ns->eth = (BYTE)state->valueint;
                break;
            }

            item = item->next;
        }
    }

error:
    cJSON_Delete (root);

    return (void*) ns;
}

void AsyncTaskGetNetworkState::cleanup_task (pthread_t thread_id, void* res)
{
    NETWORK_STATE* info = (NETWORK_STATE*) res;
    if (info) {
        if (info->curr_ssid)
            free (info->curr_ssid);
        delete info;
    }

    delete this;
}

#define MIN_RSSI -100
#define MAX_RSSI -55 

static int signal_rssi_to_level (int rssi)
{
    if (rssi <= MIN_RSSI)
        return 0;
    else if (rssi >= MAX_RSSI)
        return 100;

    return (int)((rssi - MIN_RSSI) * 100.0 / (MAX_RSSI - MIN_RSSI));
}

static bool check_is_open (const char* cap)
{
    if (strstr (cap, "WPA") || strstr (cap, "WPS")) {
        return false;
    }

    return true;
}

void* AsyncTaskGetHotspotList::execute_task (pthread_t thread_id, int task_id, DWORD add_data)
{
    m_info = new WIFI_HOTSPOTS_INFO;
    m_info->status = TS_WIFILIST_UNKNOWN;
    m_info->current = -1;
    m_info->hotspots = new HotspotCollection;

    char* res = jbus_invoke ("cbplus.network", "scanWiFi", "{}");
    if (res == NULL) {
        m_info->status = TS_WIFILIST_SYSERROR;
        return m_info;
    }

    /* convert to C structure here */
    cJSON* root = cJSON_Parse (res);
    jbus_free (res);

    if (root == NULL) {
        goto error;
    }
    else {

        cJSON* retCode = cJSON_GetObjectItem (root, "retCode");
        if (retCode == NULL) {
            m_info->status = TS_WIFILIST_BADDATA;
            goto error;
        }

        if (retCode->valueint == 40043) {
            m_info->status = TS_WIFILIST_DISABLED;
            goto error;
        }

        if (retCode->valueint == 40044) {
            m_info->status = TS_WIFILIST_BADDATA;
            goto error;
        }

        if (retCode->valueint != 0) {
            m_info->status = TS_WIFILIST_SYSERROR;
            goto error;
        }

        cJSON* data = cJSON_GetObjectItem (root, "data");
        if (data == NULL) {
            m_info->status = TS_WIFILIST_BADDATA;
            goto error;
        }

        cJSON* currentSSID = cJSON_GetObjectItem (data, "currentSSID");
        if (currentSSID == NULL) {
            m_info->status = TS_WIFILIST_BADDATA;
            goto error;
        }

        cJSON* scanResult = cJSON_GetObjectItem (data, "scanResult");
        if (scanResult == NULL) {
            m_info->status = TS_WIFILIST_BADDATA;
            goto error;
        }

        int i = 0;
        cJSON* item = scanResult->child;
        while (item) {
            cJSON* ssid = cJSON_GetObjectItem (item, "ssid");
            cJSON* bssid = cJSON_GetObjectItem (item, "bssid");
            cJSON* cap = cJSON_GetObjectItem (item, "capabilities");
            cJSON* freq = cJSON_GetObjectItem (item, "frequency");
            cJSON* level = cJSON_GetObjectItem (item, "level");

            if (ssid == NULL || bssid == NULL || cap == NULL || freq == NULL || level == NULL) {
                /* skip the bad data */
                item = item->next;
                i++;
                continue;
            }

            if (currentSSID->valuestring && 
                    (strcasecmp (ssid->valuestring, currentSSID->valuestring) == 0)) {
                m_info->current = i;
            }

            WifiHotspot* hotspot = new WifiHotspot;

            hotspot->ssid = strdup (ssid->valuestring);
            hotspot->bssid = strdup (bssid->valuestring);
            hotspot->capabilities = strdup (cap->valuestring);
            hotspot->freq = atoi (freq->valuestring);
            hotspot->signal = signal_rssi_to_level (atoi (level->valuestring));
            hotspot->is_open = check_is_open (cap->valuestring);

            m_info->hotspots->push_back (hotspot);

            item = item->next;
            i++;
        }
    }

    m_info->status = TS_WIFILIST_OK;

error:
    cJSON_Delete (root);
    return (void*) m_info;
}

AsyncTaskGetHotspotList::~AsyncTaskGetHotspotList ()
{
    if (m_info) {
        int size = m_info->hotspots->size ();
        for (int i = 0; i < size; i++) {
            WifiHotspot* hotspot = m_info->hotspots->at (i);

            if (hotspot->ssid) free (hotspot->ssid);
            if (hotspot->bssid) free (hotspot->bssid);
            if (hotspot->capabilities) free (hotspot->capabilities);
            delete hotspot;
        }

        m_info->hotspots->clear ();

        delete m_info->hotspots;
        delete m_info;
    }
}

void* AsyncTaskConnectHotspot::execute_task (pthread_t thread_id, int task_id, DWORD add_data)
{
    WIFI_CONN_RESULT* info = new WIFI_CONN_RESULT;
    memset (info, 0, sizeof (WIFI_CONN_RESULT));

    cJSON* root = cJSON_CreateObject ();
    cJSON_AddStringToObject (root, "ssid", m_conn_info.ssid);
    cJSON_AddStringToObject (root, "password", m_conn_info.password);

    char* args = cJSON_PrintUnformatted (root);
    _MG_PRINTF ("AsyncTaskConnectHotspot::execute_task > args: %s\n", args);
    char* res = jbus_invoke ("cbplus.network", "connectWiFi", args);
    free (args);

    if (res == NULL) {
        info->status = TS_CONN_WIFI_STATUS_SYSERR;
        cJSON_Delete (root);
        return info;
    }
    cJSON_Delete (root);

    /* convert to C structure here */
    root = cJSON_Parse (res);
    jbus_free (res);

    info->status = TS_CONN_WIFI_STATUS_SYSERR;
    if (root) {
        cJSON* retCode = cJSON_GetObjectItem (root, "retCode");
        if (retCode) {
            info->status = TS_CONN_WIFI_STATUS_OK;
            info->result = retCode->valueint;
        }

        cJSON_Delete (root);
    }

    return info;
}

void AsyncTaskConnectHotspot::cleanup_task (pthread_t thread_id, void* res)
{
    WIFI_CONN_RESULT* info = (WIFI_CONN_RESULT*) res;
    if (info) {
        delete info;
    }

    delete this;
}

void* AsyncTaskEnableLTE::execute_task (pthread_t thread_id, int task_id, DWORD add_data)
{
    cJSON* root = cJSON_CreateObject ();
    const char* cmd;

    if (add_data)
        cmd = "enableLte";
    else
        cmd = "disableLte";

    char* res = jbus_invoke ("cbplus.network", cmd, "{}");
    if (res == NULL) {
        cJSON_Delete (root);
        return (void*)MAKELONG (add_data, TS_ENABLE_LTE_STATUS_SYSERR);
    }
    cJSON_Delete (root);

    /* convert to C structure here */
    root = cJSON_Parse (res);
    jbus_free (res);

    DWORD status = TS_ENABLE_LTE_STATUS_UNKNOWN;
    if (root) {
        cJSON* retCode = cJSON_GetObjectItem (root, "retCode");
        if (retCode && retCode->valueint == 0) {
            status = TS_ENABLE_LTE_STATUS_OK;
        }

        cJSON_Delete (root);
    }

    return (void*)MAKELONG (add_data, status);
}

AsyncTaskEnableLTE* NetworkService::enableLTE (HWND hwnd, BOOL enable)
{
    AsyncTaskEnableLTE* task = new AsyncTaskEnableLTE ();
    task->createTask (hwnd, TASKID_NETWORK_ENABLE_LTE, (DWORD)enable);
    return task;
}

void* AsyncTaskEnableWiFi::execute_task (pthread_t thread_id, int task_id, DWORD add_data)
{
    cJSON* root = cJSON_CreateObject ();
    const char* cmd;

    if (add_data)
        cmd = "enableWiFi";
    else
        cmd = "disableWiFi";

    char* res = jbus_invoke ("cbplus.network", cmd, "{}");
    if (res == NULL) {
        cJSON_Delete (root);
        return (void*)MAKELONG (add_data, TS_ENABLE_WIFI_STATUS_SYSERR);
    }
    cJSON_Delete (root);

    /* convert to C structure here */
    root = cJSON_Parse (res);
    jbus_free (res);

    DWORD status = TS_ENABLE_WIFI_STATUS_UNKNOWN;
    if (root) {
        cJSON* retCode = cJSON_GetObjectItem (root, "retCode");
        if (retCode && retCode->valueint == 0) {
            status = TS_ENABLE_WIFI_STATUS_OK;
        }

        cJSON_Delete (root);
    }

    return (void*)MAKELONG (add_data, status);
}

AsyncTaskEnableWiFi* NetworkService::enableWiFi (HWND hwnd, BOOL enable)
{
    AsyncTaskEnableWiFi* task = new AsyncTaskEnableWiFi ();
    task->createTask (hwnd, TASKID_NETWORK_ENABLE_WIFI, (DWORD)enable);
    return task;
}

