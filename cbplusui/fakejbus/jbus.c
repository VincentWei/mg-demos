#include <unistd.h>
#include <stdio.h>

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

int jbus_add_event_handler(const char *event, jbus_event_handler_t handler)
{
    return JBUS_SUCCESS;
}

int jbus_connect(void)
{
    return JBUS_SUCCESS;
}

void jbus_run(void)
{
    while (1) {
        usleep (10000);
    }
}

void jbus_clear(void)
{
}

