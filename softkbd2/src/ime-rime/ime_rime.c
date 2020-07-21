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
** Copyright (C) 2008 ~ 2019 FMSoft (http://www.fmsoft.cn).
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>

#include "ime_rime.h"
#include "rime_api.h"

// static RimeSessionId session_id;
// static RimeApi *rime = NULL;
static RimeTraits traits;
static RimeCommit commit;
static RimeStatus status;
static RimeContext context;

const int Page_size = 5;

void notification_handler(void *context,
                          RimeSessionId session_id,
                          const char *message_type,
                          const char *message_value)
{
  printf("message: [%lu] [%s] %s\n", session_id, message_type, message_value);
}

BOOL ime_rime_init(void)
{

  fprintf(stderr, "ready.\n");
  RimeApi *rime = rime_get_api();

  // RIME_STRUCT(RimeTraits, traits);
  // RIME_STRUCT(RimeCommit, commit);
  // RIME_STRUCT(RimeStatus, status);
  // RIME_STRUCT(RimeContext, context);

  RIME_STRUCT_INIT(RimeTraits, traits);
  RIME_STRUCT_INIT(RimeCommit, commit);
  RIME_STRUCT_INIT(RimeStatus, status);
  RIME_STRUCT_INIT(RimeContext, context);
  traits.app_name = "IME_Rime";
  traits.distribution_name = "IME_Rime";
  char data_dir[128];
  if ((getcwd(data_dir, sizeof(data_dir))) != NULL)
  {
    strcat(data_dir, "/data");
  }
  traits.user_data_dir = data_dir;
  traits.shared_data_dir = data_dir;

  //  traits.distribution_name = "IME_Rime";
  rime->setup(&traits);

  printf("rime 初始化");
  rime->set_notification_handler(&notification_handler, NULL);

  fprintf(stderr, "IME_Rime Initializing...\n");
  rime->initialize(NULL);
  Bool full_check = True;
  if (rime->start_maintenance(full_check))
    rime->join_maintenance_thread();
  fprintf(stderr, "IME_Rime Ready.\n");
  RimeSessionId session_id;
  session_id = rime->create_session();
  if (!session_id)
  {
    fprintf(stderr, "Error creating rime session.\n");
    return FALSE;
  }
  rime->destroy_session(session_id);
  printf("成功rime\n");
  return TRUE;
}

static const char *digit_to_letter_map[] = {
    "abc",
    "def",
    "ghi",
    "jkl",
    "mno",
    "pqrs",
    "tuv",
    "wxyz"};

int rime_translate_word(void *method, const char *keystokes,
                        char *buff, int buff_len, int index)
{
  RimeApi *rime = rime_get_api();
  RimeSessionId session_id;
  //可能是长度限制，需要修改一下len(w中)
  //TODO 翻页等功能
  //index 表示当前页面号(多页功能)
  //keystokes 表示按键序列
  //buff存储转换后的文本,由于buff大小限制，仅仅表示一部分
  //buff_len表示文本的长度
  // 返回值，一种信号，表示下一页的页号

  if (index < 0)
    return -1;

  int is_last = 0;
  char stokes[32];
  memset(stokes, 0, 32);
  strcat(stokes, keystokes);
  printf("%s\n", stokes);
  printf("%d\n", buff_len);
  // if (strlen(stokes) > 5)
  // {
  //   stokes[5] = '\0';
  // }
  //memset(buff, 0, buff_len);//无效
  buff[0] = 0; //有效避免重复;务必！！！！！
  session_id = rime->create_session();
  // rime->simulate_key_sequence(session_id, "1");
  rime->simulate_key_sequence(session_id, stokes);

  // rime->get_context(session_id, &context);
  //rime 中翻页用 “，”与“.”

  for (int i = 0; i < index; i++)
  {
    rime->simulate_key_sequence(session_id, ".");
  }
  rime->get_context(session_id, &context);
  if (context.menu.is_last_page)
  {
    is_last = 1;
  }
  else
  {
    is_last = 0;
  }
  int lens = 0;
  for (int candidate_cnt = 0; candidate_cnt < context.menu.num_candidates; candidate_cnt++)
  {
    if (strlen(context.menu.candidates[candidate_cnt].text) < 14)
    {
      strcat(buff, context.menu.candidates[candidate_cnt].text);
      // // printf("sim %s\n", context.menu.candidates[candidate_cnt].text);
      strcat(buff, " ");
      lens += 1;
      if (lens >= Page_size)
      {
        break;
      }
      rime->simulate_key_sequence(session_id, ".");
    }
  }
  // printf("%d\n", session_id);

  rime->free_context(&context);
  // strcat(buff, " √ ");
  // strcat(buff, "中国伟大 ");
  // strcat(buff, " ");
  printf("%s\n", buff);
  rime->destroy_session(session_id);
  if (is_last)
    return -1;
  return index + 1;
}

int rime_predict_word(void *method, const char *keystokes,
                      char *buff, int buff_len, int index)
{
  // memset(buff, 0, buff_len);
  buff[0] = 0; //有效避免重复;务必！！！！！
  // printf("%d\n", session_id);
  return -1;
}

int RimeMatchKeystokes(const char *keystokes, char *buff, int buff_len,
                       int index)
{
  RimeApi *rime = rime_get_api();
  RimeSessionId session_id;
  if (index < 0)
    return -1;

  int is_last = 0;
  char stokes[32];
  memset(stokes, 0, 32);
  strcat(stokes, keystokes);
  printf("%s\n", stokes);
  //memset(buff, 0, buff_len);//无效
  buff[0] = 0; //有效避免重复;务必！！！！！
  session_id = rime->create_session();
  rime->simulate_key_sequence(session_id, stokes);

  //rime 中翻页用 “，”与“.”

  for (int i = 0; i < index; i++)
  {
    rime->simulate_key_sequence(session_id, ".");
  }
  rime->get_context(session_id, &context);
  if (context.menu.is_last_page)
  {
    is_last = 1;
  }
  else
  {
    is_last = 0;
  }
  for (int candidate_cnt = 0; candidate_cnt < context.menu.num_candidates; candidate_cnt++)
  {
    strcat(buff, context.menu.candidates[candidate_cnt].text);
    //    printf("sim %s\n", context.menu.candidates[candidate_cnt].text);
    strcat(buff, "  ");
  }

  rime->free_context(&context);
  // strcat(buff, " √ ");
  // strcat(buff, "中国伟大 ");
  // strcat(buff, " ");
  rime->destroy_session(session_id);
  if (is_last)
    return -1;
  return index + 1;
  ;
}
