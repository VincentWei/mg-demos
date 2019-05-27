#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "jbus.h"

int jbus_send_event(const char *event, const char *message)
{
    return JBUS_SUCCESS;
}

char *jbus_invoke(const char *path, const char *method, const char *message)
{
    return NULL;
}

void jbus_free(void *buf)
{
}

char *jbus_result(char *message)
{
    return NULL;
}

static const char* events [] = {
    "cbplus.event.NETWORK_STATE_CHANGED",
    "cbplus.event.BATTERY_CHANGED",
    "cbplus.event.WIFI_STATE_CHANGED",
    "cbplus.event.WIFI_RSSI_CHANGED",
    "cbplus.event.CELLULAR_DATA_STATE_CHANGED",
    "cbplus.event.CELLULAR_SIGNAL_CHANGED",
    "cbplus.event.CAMERA_CONNECTED",
    "cbplus.event.CAMERA_DISCONNECTED",
    "cbplus.event.CLOUD_STATE",
    "cbplus.event.UPLOAD_STATE",
    "cbplus.event.ORDER_STATE",
    "cbplus.event.FIRMWARE_DOWNLOAD",
    "cbplus.event.FIRMWARE_UPGRADE",
};

#define TABLESIZE(t)    (int)(sizeof(t)/sizeof(t[0]))

static int get_event_id(const char* event)
{
    int i;
    for (i = 0; i < TABLESIZE(events); i++) {
        if (strcmp (event, events[i]) == 0)
            return i;
    }

    return -1;
}

typedef char* (*event_generator_t)(void);

typedef struct _event_handler {
    event_generator_t generator;
    jbus_event_handler_t handler;
} event_handler;

static event_handler handlers [TABLESIZE(events)];

int jbus_add_event_handler(const char *event, jbus_event_handler_t handler)
{
    int eid = get_event_id (event);
    if (eid < 0)
        return JBUS_PARAM_ERROR;

    handlers[eid].handler = handler;
    return JBUS_SUCCESS;
}

static char* event_generator_network_state_changed(void)
{
    return NULL;
}

static char* event_generator_battery_changed(void)
{
    static const char* format = "{\"status\":%d, \"scale\": %d, \"level\": %d}";
    char* message;

    message = calloc (sizeof(char), 256);
    if (message) {
        sprintf(message, format, time(NULL)%5 + 1, 100, time(NULL)%100);
    }

    return message;
}

static char* event_generator_wifi_state_changed(void)
{
    return NULL;
}

static char* event_generator_wifi_rssi_changed(void)
{
    return NULL;
}

static char* event_generator_cellular_data_state_changed(void)
{
    return NULL;
}

static char* event_generator_cellular_signal_changed(void)
{
    return NULL;
}

static char* event_generator_camera_connected(void)
{
    return NULL;
}

static char* event_generator_camera_disconnected(void)
{
    return NULL;
}

static char* event_generator_cloud_state_changed(void)
{
    return NULL;
}

static char* event_generator_upload_state_changed(void)
{
    return NULL;
}

static char* event_generator_order_state_changed(void)
{
    return NULL;
}

static char* event_generator_firmware_download(void)
{
    return NULL;
}

static char* event_generator_firmware_upgrade(void)
{
    return NULL;
}

int jbus_connect(void)
{
    handlers[0].generator = event_generator_network_state_changed;
    handlers[1].generator = event_generator_battery_changed;
    handlers[2].generator = event_generator_wifi_state_changed;
    handlers[3].generator = event_generator_wifi_rssi_changed;
    handlers[4].generator = event_generator_cellular_data_state_changed;
    handlers[5].generator = event_generator_cellular_signal_changed;
    handlers[6].generator = event_generator_camera_connected;
    handlers[7].generator = event_generator_camera_disconnected;
    handlers[8].generator = event_generator_cloud_state_changed;
    handlers[9].generator = event_generator_upload_state_changed;
    handlers[10].generator = event_generator_order_state_changed;
    handlers[11].generator = event_generator_firmware_download;
    handlers[12].generator = event_generator_firmware_upgrade;

    return JBUS_SUCCESS;
}

void jbus_run(void)
{
    while (1) {
        int eid;

        usleep (500000);

        eid = time(NULL) % TABLESIZE(events);
        if (handlers[eid].handler) {
            char* message = handlers[eid].generator();
            if (message) {
                handlers[eid].handler(events[eid], message);
                free (message);
            }
        }
    }
}

void jbus_clear(void)
{
}

