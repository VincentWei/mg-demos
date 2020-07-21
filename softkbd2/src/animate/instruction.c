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
#include <assert.h>
#include <minigui/common.h>

#ifdef KBD_ANIMATE

#include "instruction.h"
#include "my_debug.h"

int inline anim_get_instrct_size (char instrct)
{
    assert (instrct < INSTRCT_KIND_NUM);
    static int instrct_lens [INSTRCT_KIND_NUM] = {
        FRAME_INSTRCT_SIZE,
        MOVE_INSTRCT_SIZE,
        SCALE_INSTRCT_SIZE,
        ALPHA_INSTRCT_SIZE,
        GOTO_INSTRCT_SIZE,
        LOOP_INSTRCT_SIZE,
        BLANK_INSTRCT_SIZE
    };
    return instrct_lens [REAL_INSTRCT(instrct)];
}

/*return instrct size,
 * if return -1, error*/
int anim_parse_instrct (char* ins_str, instrct_t* instrct)
{
    DWORD tmp;
    STORE_INSTRCT(ins_str, instrct->id, instrct->is_index);
    switch (instrct->id)
    {
        case FRAME_INSTRCT:
            STORE_DWORD (ins_str, tmp);
            instrct->frame_num = 1;
            instrct->u.frame_param.img = (void*)tmp;
            return FRAME_INSTRCT_SIZE;

        case MOVE_INSTRCT:
            STORE_WORD (ins_str, instrct->u.move_param.x0);
            STORE_WORD (ins_str, instrct->u.move_param.y0);
            STORE_BYTE (ins_str, instrct->frame_num);
            STORE_WORD (ins_str, instrct->u.move_param.x1);
            STORE_WORD (ins_str, instrct->u.move_param.y1);
            return MOVE_INSTRCT_SIZE;

        case SCALE_INSTRCT:
            STORE_WORD (ins_str, instrct->u.scale_param.w0);
            STORE_WORD (ins_str, instrct->u.scale_param.h0);
            STORE_BYTE (ins_str, instrct->frame_num);
            STORE_WORD (ins_str, instrct->u.scale_param.w1);
            STORE_WORD (ins_str, instrct->u.scale_param.h1);
            return SCALE_INSTRCT_SIZE;

        case ALPHA_INSTRCT:
            STORE_WORD (ins_str, instrct->u.alpha_param.alpha0);
            STORE_BYTE (ins_str, instrct->frame_num);
            STORE_WORD (ins_str, instrct->u.alpha_param.alpha1);
//            printf("alpha: %d-%d-%d\n", instrct->u.alpha_param.alpha0, instrct->frame_num, instrct->u.alpha_param.alpha1);
            return ALPHA_INSTRCT_SIZE;

        case GOTO_INSTRCT:
            STORE_WORD (ins_str, instrct->u.goto_param.ins_pos);
            instrct->frame_num = 1;
            return GOTO_INSTRCT_SIZE;

        case LOOP_INSTRCT:
            instrct->frame_num = 1;
            return LOOP_INSTRCT_SIZE;

        case BLANK_INSTRCT:
            STORE_BYTE (ins_str, instrct->frame_num);
            return BLANK_INSTRCT_SIZE;

        default:
            return -1;
    }
}

#endif /* KBD_ANIMATE */
