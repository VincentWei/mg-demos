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

#ifndef __P_CODE_H__
#define __P_CODE_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MIN_STACK_SIZE  8
#define DEF_STACK_SIZE  1024

//define the p-code method
typedef struct _PCODE_METHOD {
    int argc;
    unsigned char* codes;
    int code_len;
}PCODE_METHOD;

//the p-code execute env
typedef struct _PCODE_ENV{
    PCODE_METHOD *main; //main entry
    intptr_t* stack;
    int stack_top;
    int ip; //instruction pointer;
    intptr_t result; //save the result
    int calldepath;
    void *param;
}PCODE_ENV;

//define the user instruction
// it return the instruction length
typedef int (* DO_USER_INSTR)(unsigned char* ins, intptr_t** stack, void* param);

typedef struct _PCODE_USER_INSTR_TABLE{
    DO_USER_INSTR *entries;
    int count;
}PCODE_USER_INSTR_TABLE;

enum pcodeInstructionErr{
    pcieOk = 0,
    pcieInvalidateIns,
    pcieStackOverflow,
    pcieInvalidateLocalVar,
    pcieInvalidateJmpAddr,
    pcieDiv0,
    pcieMethodTableIsNull,
    pcieUnknownMethod,
    pcieNativeMethodTableIsNull,
    pcieUnknownNativeMethod,
    pciePtrIsNull,
    pcieUnknownError
};

typedef struct _PCODE_METHOD_TABLE{
    PCODE_METHOD ** methods;
    int method_count;
}PCODE_METHOD_TABLE;

typedef intptr_t (*NATIVE_CALL_BACK)(int *param, int param_count);
typedef struct _PCODE_NATIVE_METHOD{
    NATIVE_CALL_BACK native_callback;
    int argc;
}PCODE_NATIVE_METHOD;

typedef struct _PCODE_NATIVE_METHOD_TABLE{
    PCODE_NATIVE_METHOD * native_methods;
    int count;
}PCODE_NATIVE_METHOD_TABLE;

#define PCOE_CONTINUE 0
#define PCOE_RET      1
typedef struct _PCODE {
    PCODE_USER_INSTR_TABLE *table;
    PCODE_METHOD_TABLE * method_table;
    PCODE_NATIVE_METHOD_TABLE* native_table;
    int (*on_error)(struct _PCODE*, int ip, int err_code, const char* desc);
}PCODE;

#define PCE_RET 0 //ExecutePCode return by ret instruction
#define PCE_INTERRUPT 1 //ExecutePCode return by interrupt instruction
#define PCE_ERROR 2
int ExecutePCode(PCODE *pcode, PCODE_ENV *env);

PCODE_ENV* CreatePCodeEnv(PCODE_METHOD* method, int stack_size, int* args);

void DeletePCodeEnv(PCODE_ENV* env);

///////////////////////////////////////////////
// define the pcode instruction
enum pcodeInstructions{
    nop = 0, //do nothing
    iconst, //push integer_value
    iload, //push integer_local_value
    iload_ptr,
    isave,
    isave_ptr,
    nullpush, //just increase the stack top
    pop,
    ret, //ret
    retv,
    jmp, //jmp to
    iadd,
    isub,
    imul,
    idiv,
    imod,
    interrupt, //interrupt current flow
    call, //call sub function
    ncall, //call native function, stdcall
    cmp, //cmp two value: stack before: ..., t1, t2, stack after: ..., 0/-1/1;
        // if t1 == t2, it push 0 into stack
        // if t1 < t2, it push -1 into stack
        // if t1 > t2, it push 1 into stack
    jmp_zero, // if current stack value == 0, pop stack, and jmp
    jmp_less, // if current stack value < 0 , pop stack, and jmp
    jmp_great, // if current stack value > 0, pop stack, and jmp
    max_pre_def_ins
};

/////////////////////////////////////////////////////
//pcode compile
//
/*  i: interger
 *  u: unsigned short
 *  c: char
 */
void AppendValue(void* pmethod, const char* fmt, ...);
#define AppendIns(pm,ins) AppendValue(pm,"c", ins)
#define AppendNop(pm)  AppendIns(pm, nop)
#define AppendIconst(pm,i) AppendValue(pm,"ci", iconst, i)
#define AppendIsave(pm,i) AppendValue(pm,"ci", isave,i)
#define AppendIload(pm,idx) AppendValue(pm,"cu", iload,idx)
#define AppendPop(pm) AppendIns(pm, pop)
#define AppendJmp(pm) AppendIns(pm, jmp)
#define AppendInterrupt(pm) AppendIns(pm,interrupt)
#define AppendRet(pm) AppendIns(pm, ret)
#define AppendRetv(pm) AppendIns(pm, retv)
#define AppendIadd(pm) AppendIns(pm, iadd)
#define AppendImul(pm) AppendIns(pm, imul)
#define AppendIsub(pm) AppendIns(pm, isub)
#define AppendIdiv(pm) AppendIns(pm, idiv)
#define AppendImod(pm) AppendIns(pm, imod)

void * CreateMethod(int arg_counts);

PCODE_METHOD * EndMethod(void *pmethod);

unsigned short SetLabel(void* pmethod);

void DeletePCodeMethod(PCODE_METHOD *pm);


//stack operation
#define PUSH(stack, v)      (*((stack)++) = (intptr_t)(v))
#define POP(stack)          (*(--(stack)))
#define POP_NOUSE(stack)    (--(stack))

PCODE_METHOD_TABLE * LoadPCodeMethodsFromFile(const char* file, void (*on_load)(PCODE_METHOD*,const char*, void*), void* param);

void DeletePCodeMethods(PCODE_METHOD_TABLE* pmt);

void ResetPCodeExecute(PCODE_ENV* env);

#ifdef __cplusplus
}
#endif

#endif
