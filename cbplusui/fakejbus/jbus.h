/*
** This file is a part of mg-demos package.
**
** Copyright (C) 2019 FMSoft (http://www.fmsoft.cn).
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

#ifndef __JBUS_H__
#define __JBUS_H__

#ifdef  __cplusplus
extern "C" {
#endif

#define JBUS_SUCCESS 0
#define JBUS_CONNECT_UBUS_FAILED -40001
#define JBUS_REGIST_EVENT_FAILED -40002
#define JBUS_PARAM_JSON_PARSE_ERROR -40003
#define JBUS_PARAM_ERROR -40004

int jbus_send_event(const char *event, const char *message);
char *jbus_invoke(const char *path, const char *method, const char *message);
void jbus_free(void *buf);
char *jbus_result(char *message);


typedef void (*jbus_event_handler_t)(const char *event, const char *message);
int jbus_add_event_handler(const char *event, jbus_event_handler_t handler);


enum jbus_message_type {
    JBUSMSG_TYPE_UNSPEC,
    JBUSMSG_TYPE_ARRAY,
    JBUSMSG_TYPE_TABLE,
    JBUSMSG_TYPE_STRING,
    JBUSMSG_TYPE_INT64,
    JBUSMSG_TYPE_INT32,
    JBUSMSG_TYPE_INT16,
    JBUSMSG_TYPE_INT8,
    __JBUSMSG_TYPE_LAST,
    JBUSMSG_TYPE_LAST = __JBUSMSG_TYPE_LAST - 1,
    JBUSMSG_TYPE_BOOL = JBUSMSG_TYPE_INT8,
};

struct jbus_message_policy {
    const char *name;
    enum jbus_message_type type;
};

typedef char * (*jbus_method_handler_t)(const char *method, const char *message);

struct jbus_method {
    const char *name;
    jbus_method_handler_t handler;
    struct jbus_message_policy *policies;
    int n_policies;
};

int jbus_add_method_handler(const char *path, struct jbus_method *methods, int n_methods);

int jbus_connect(void);
void jbus_run(void);
void jbus_clear(void);

#ifdef  __cplusplus
}
#endif

#endif
