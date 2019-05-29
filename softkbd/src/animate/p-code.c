
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <stdarg.h>

#include "p-code.h"

#if KBD_ANIMATE

PCODE_ENV* CreatePCodeEnv(PCODE_METHOD* method, int stack_size, int *argv)
{
	PCODE_ENV *env;

	if(stack_size < MIN_STACK_SIZE)
		stack_size = MIN_STACK_SIZE;
	if(method == NULL){
		fprintf(stderr, "method is null\n");
		return NULL;
	}

	if( method->argc > 0 && argv == NULL )
	{
		fprintf(stderr,"args is NULL, %d args are needed\n",method->argc);
		return NULL;
	}

	
	env = (PCODE_ENV*)calloc(1,sizeof(PCODE_ENV));
	
	if(env == NULL)
	{
		fprintf(stderr, "malloc env failed!\n");
		return NULL;
	}

	env->main = method;
	env->stack = (unsigned int*)malloc(sizeof(int)*(stack_size + method->argc));
	env->ip = 0;

	//copy args into stack
	if(method->argc > 0){
		memcpy(env->stack, argv, sizeof(int)*method->argc);
	}

	env->stack_top = method->argc;

	return env;
}

void DeletePCodeEnv(PCODE_ENV* env)
{
	if(env)
	{
		free(env->stack);
		free(env);
	}
}

int call_native_method(PCODE_NATIVE_METHOD *nmth, unsigned int *stack)
{
	int *argv = NULL;
	if(nmth->argc > 0)
	{
		argv = (int*)alloca(sizeof(int)*nmth->argc);
		memcpy(argv, stack, sizeof(int)*nmth->argc);
	}

	return (*nmth->native_callback)(argv, nmth->argc);
}

static char* error_str[] = {
	"OK",
	"Invalidate Instruction",
	"Stack overflow",
	"Invalidate local variable",
	"Invalidate jump address",
	"Div 0",
	"Method table is null",
	"Unknown method ",
	"Native method table is null",
	"Unknown native method",
	"Ponter is null",
	"unknown error"
};

static char* ins_names[]={
	"nop",
	"iconst",
	"iload",
	"iload_ptr",
	"isave",
	"isave_ptr",
	"nullpush",
	"pop",
	"ret",
	"retv",
	"jmp",
	"iadd",
	"isub",
	"imul",
	"idiv",
	"imod",
	"intr",
	"call",
	"ncall",
	"cmp",
	"jz",
	"jl",
	"jg"
};

static void reportInsError(int errcode, unsigned char* ins, unsigned int* stack, unsigned int *stack_base)
{
	char szInsInfo[30];

	switch(*ins)
	{
	case nop:
	case nullpush:
	case pop:
	case ret:
	case iadd:
	case isub:
	case imul:
	case idiv:
	case imod:
	case interrupt:
	case cmp:
	case jmp_zero:
	case jmp_less:
	case jmp_great:
		sprintf(szInsInfo, "%s", ins_names[*ins]);
		break;
	case iconst:
	case iload:
	case iload_ptr:
	case isave:
	case isave_ptr:
	case call:
	case ncall:
		sprintf(szInsInfo, "%s %d", ins_names[*ins], *(unsigned short*)(ins+1));
		break;
	default:
		sprintf(szInsInfo,"user extends ins:%d %d", *ins,*(unsigned short*)(ins+1));
		break;
	}

	fprintf(stderr, "error: %s, At %s\n", error_str[errcode], szInsInfo);
	//printf the stack info;
	fprintf(stderr, "-----stack info-----\n");
	int i;
	stack --;
	for(i=1; i<=16 && stack>= stack_base; stack --, i++)
	{
		fprintf(stderr,"\t%d", *stack);
		if(i%4==0)
			fprintf(stderr,"\n");
	}
	fprintf(stderr,"\n");
	if(stack > stack_base)
		fprintf(stderr, " ... ... ... ... ... \n");
	fprintf(stderr,"-----------------------------------\n");
}

int ExecutePCode(PCODE *pcode, PCODE_ENV* env)
{
	register unsigned char* ip, *ipend,* ipbegin;
	register unsigned int* stack;
	register unsigned int* stack_base;
	register DO_USER_INSTR * entries = NULL;
	register int user_instr_count = 0;
	register void* param;
	register int argc;
	register int idx;
	int error = pcieOk;
	unsigned char* err_ip;

	if(pcode == NULL)
	{
		fprintf(stderr,"pcode is null\n");
		return PCE_ERROR;
	}

	if(env == NULL)
	{
		fprintf(stderr,"env is null\n");
		return PCE_ERROR;
	}

	ipbegin = env->main->codes;
	ip = ipbegin + env->ip;
	argc = env->main->argc;
	stack = env->stack + argc + 3;
	ipend = ipbegin + env->main->code_len;
	stack_base = env->stack;
	
	if(pcode->table){
		entries = pcode->table->entries;
		user_instr_count = pcode->table->count;
		param = env->param;
	}

AGIN:
	while(ip < ipend)
	{
		switch(*ip)
		{
		case nop:
			ip ++;
			break;
		case nullpush:
			stack ++;
			ip ++;
			break;
		case iconst:
			ip ++;
			PUSH(stack, *(unsigned int*)ip);
			ip += sizeof(unsigned int);
			break;
		case iload:
			{
				ip ++;
				idx = *(unsigned short*)ip;
				ip += sizeof(unsigned short);
				if(idx < 0 || idx > (stack - stack_base))
				{
					err_ip = ip - 1 - sizeof(unsigned short);
					error = pcieInvalidateLocalVar;
					goto ERROR;
				}
				PUSH(stack, stack_base[idx]);
				break;
			}
		case iload_ptr:
			{
				ip ++;
				idx = *(unsigned short*)ip;
				ip += sizeof(unsigned short);
				if( idx<0 || idx > (stack - stack_base))
				{
					err_ip = ip - 1 - sizeof(unsigned short);
					error = pcieInvalidateLocalVar;
					goto ERROR;
				}
				if(stack_base[idx] == 0){
					err_ip = ip - 1 - sizeof(unsigned short);
					error = pciePtrIsNull;
					goto ERROR;
				}
				PUSH(stack, *((unsigned int*)stack_base[idx]));
				break;
			}
		case isave:
			{
				ip ++;
				idx = *(unsigned short*)ip;
				ip += sizeof(unsigned short);
				if(idx < 0 || idx > (stack - stack_base))
				{
					err_ip = ip - 1 - sizeof(unsigned short);
					error = pcieInvalidateLocalVar;
					goto ERROR;
				}
				stack_base[idx] = POP(stack);
				break;
			}
		case isave_ptr:
			{
				ip ++;
				idx = *(unsigned short*)ip;
				ip += sizeof(unsigned short);
				if(idx < 0 || idx > (stack - stack_base)){
					err_ip = ip - 1 - sizeof(unsigned short);
					error = pcieInvalidateLocalVar;
					goto ERROR;
				}
				if(stack_base[idx] == 0){
					err_ip = ip - 1 - sizeof(unsigned short);
					error = pciePtrIsNull;
					goto ERROR;
				}
				*((unsigned int*)stack_base[idx]) = POP(stack);
				break;
			}
		case call:
			{
				ip ++;
				idx = *(unsigned short*)ip;
				ip += sizeof(unsigned short);
				if(pcode->method_table == NULL)
				{
					err_ip = ip - 1 - sizeof(unsigned short);
					error = pcieMethodTableIsNull;
					goto ERROR;
				}
				if(idx < 0 || idx >= pcode->method_table->method_count)
				{
					err_ip = ip - 1 - sizeof(unsigned short);
					error = pcieUnknownMethod;
					goto ERROR;
				}

				//push ip and stack and old main
				PUSH(stack, stack_base);
				PUSH(stack, ip);
				PUSH(stack, env->main);
				env->main = pcode->method_table->methods[idx];
				argc = env->main->argc;
				stack_base = stack - 3 - argc;
				ip = env->main->codes;
				ipbegin = ip;
				ipend = ip + env->main->code_len;
				env->calldepath ++;
				break;
			}
		case ncall: //native call
			{
				PCODE_NATIVE_METHOD *nmth;
				int result;
				ip ++;
				idx = *(unsigned short*)ip;
				ip += sizeof(unsigned short);
				if(pcode->native_table == NULL){
					err_ip = ip - 1 - sizeof(unsigned short);
					error = pcieNativeMethodTableIsNull;
					goto ERROR;
				}
				if(idx < 0 || idx >= pcode->native_table->count){
					err_ip = ip - 1 - sizeof(unsigned short);
					error = pcieUnknownNativeMethod;
					goto ERROR;
				}
				nmth = &pcode->native_table->native_methods[idx];
				stack -= nmth->argc;
				result = call_native_method(nmth,stack);
				PUSH(stack, result);
				break;
			}
		case pop:
			POP(stack);
			ip ++;
			break;
		case ret:	
		case retv:
			{
				env->result = POP(stack);
				if((--env->calldepath) < 0)
				{
					return PCE_RET;
				}
				stack = stack_base;
				//restore caller's stack info
				stack_base = (unsigned int*)stack[argc];
				ip = (unsigned char*)stack[argc + 1];
				env->main = (PCODE_METHOD*)stack[argc + 2];
				argc = env->main->argc;
				ipbegin = env->main->codes;
				ipend = ipbegin + env->main->code_len;
				PUSH(stack, env->result);
				break;
			}
		case jmp_zero:
		{
			unsigned char* new_ip;
			ip ++;
			if((int)POP(stack) == 0)
			{
				new_ip = ipbegin + *(unsigned short*)ip;
				if( new_ip < ipbegin || new_ip >= ipend){
					err_ip = ip - 1;
					ip += sizeof(unsigned short);
					error = pcieInvalidateJmpAddr;
					goto ERROR;
				}
				ip = new_ip;
			}
			else{
				ip += sizeof(unsigned short);
			}
			break;
		}
		case jmp_less:
		{
			unsigned char* new_ip;
			ip ++;
			if((int)POP(stack) < 0)
			{
				new_ip = ipbegin + *(unsigned short*)ip;
				if( new_ip < ipbegin || new_ip >= ipend){
					err_ip = ip - 1;
					ip += sizeof(unsigned short);
					error = pcieInvalidateJmpAddr;
					goto ERROR;
				}
				ip = new_ip;
			}
			else{
				ip += sizeof(unsigned short);
			}
			break;
		}
		case jmp_great:
		{
			unsigned char* new_ip;
			ip ++;
			if((int)POP(stack) > 0)
			{
				new_ip = ipbegin + *(unsigned short*)ip;
				if( new_ip < ipbegin || new_ip >= ipend){
					err_ip = ip - 1;
					ip += sizeof(unsigned short);
					error = pcieInvalidateJmpAddr;
					goto ERROR;
				}
				ip = new_ip;
			}
			else{
				ip += sizeof(unsigned short);
			}
			break;
		}
		case jmp:
		{
			unsigned char* new_ip;
			ip ++;
			new_ip = ipbegin + *(unsigned short*)ip;
			if( new_ip < ipbegin || new_ip >= ipend){
				err_ip = ip - 1;
				ip += sizeof(unsigned short);
				error = pcieInvalidateJmpAddr;
				goto ERROR;
			}
			ip = new_ip;
			break;
		}	
		case interrupt:
		{
			env->ip = ip - ipbegin + 1;
			env->stack = stack_base;
			env->stack_top = stack_base - stack;
			return PCE_INTERRUPT;
		}
		case iadd:
			{
				int result = (int)(POP(stack)) + (int)(POP(stack));
				PUSH(stack, result);
				ip ++;
			}
			break;
		case isub:
		case cmp:
			{
				int a1, a2;
				a2 = (int)POP(stack);
				a1 = (int)POP(stack);
				PUSH(stack, (a1 - a2));
				ip ++;
				break;
			}
		case imul:
			{
				int result = (int)(POP(stack)) * (int)(POP(stack ));
				PUSH(stack, result);
				ip ++;
				break;
			}
		case idiv:
			{
				int a1, a2;
				ip ++;
				a2 = (int)POP(stack);
				if( a2 == 0){
					err_ip = ip - 1;
					error = pcieDiv0;
					goto ERROR;
				}
				a1 = (int)POP(stack);
				PUSH(stack, (a1/a2));
				break;
			}
		case imod:
			{
				int a1, a2;
				ip ++;
				a2 = (int)POP(stack);
				if ( a2 == 0 ){
					err_ip = ip - 1;
					error = pcieDiv0;
					goto ERROR;
				}
				a1 = (int)POP(stack);
				PUSH(stack, (a1%a2));
				break;
			}
		default:
			if(entries && (*ip-max_pre_def_ins)< user_instr_count)
			{
				unsigned int * st = stack;
				ip += (*entries[*ip-max_pre_def_ins])(ip, &st, param);
				stack = st;
				break;
			}
			else
			{
				err_ip = ip;
				ip ++;
				error = pcieInvalidateIns;
				goto ERROR;
			}
		}
	}

	return PCE_RET;

ERROR:
	
	if(pcode->on_error)
	{
		if((*pcode->on_error)(pcode, ip - ipbegin, error, error_str[error]) == PCOE_CONTINUE)
			goto AGIN;
	}
	else
	{
		reportInsError(error,err_ip,stack, stack_base);
		return PCE_ERROR;
	}
}


////////////////////////////////////////////
//
typedef struct _pcode_method_ex_t{
	PCODE_METHOD *method;
	int buf;
}pcode_method_ex_t;

#define STEP_SIZE 256

void * CreateMethod(int arg_counts)
{
	pcode_method_ex_t *pex = (pcode_method_ex_t*)calloc(1,sizeof(pcode_method_ex_t));
	pex->method = (PCODE_METHOD*)calloc(1,sizeof(PCODE_METHOD));
	pex->buf = STEP_SIZE;
	pex->method->codes = (unsigned char*)malloc(pex->buf);
	pex->method->argc = arg_counts;
	return pex;
}

inline unsigned char* get_ptr(pcode_method_ex_t* pex, int size) 
{
	unsigned char* p = pex->method->codes + pex->method->code_len;
	if( (pex->method->code_len+size) > pex->buf){
		pex->buf += STEP_SIZE;
		pex->method->codes = (unsigned char*)realloc(pex->method->codes, pex->buf);
	}
	pex->method->code_len += size;
	return p;
}

void AppendValue(void* pmethod, const char* fmt, ...)
{
	va_list va;
	pcode_method_ex_t *pex;
	unsigned char* p;

	if(pmethod == NULL || fmt == NULL)
		return ;

	pex = (pcode_method_ex_t*)pmethod;
	va_start(va, fmt);
	
	while(*fmt)
	{
		switch(*fmt){
		case 'i':
			p = get_ptr(pex, sizeof(int));
			*(unsigned int*)p = va_arg(va, unsigned int);
			break;
		case 'u':
			p = get_ptr(pex, sizeof(unsigned short));
			*(unsigned short*)p = (unsigned short)va_arg(va, int);
			break;
		case 'c':
			p = get_ptr(pex, sizeof(unsigned char));
			*p = (unsigned char)va_arg(va, int);
			break;
		}
		fmt ++;
	}

}

PCODE_METHOD* EndMethod(void* pmethod)
{
	pcode_method_ex_t *pex;
	PCODE_METHOD *pm;
	if(pmethod == NULL)
		return NULL;

	pex = (pcode_method_ex_t*)pmethod;
	
	pm = pex->method;

	if(pex->buf != pm->code_len)
	{
		pm->codes = (unsigned char*)realloc(pm->codes, pm->code_len);
	}
	free(pex);
	return pm;
}

unsigned short SetLabel(void* pmethod)
{
	if(pmethod == NULL)
		return 0;

	return (unsigned short)(((pcode_method_ex_t*)pmethod)->method->code_len);
}

void DeletePCodeMethod(PCODE_METHOD *pm)
{
	if(pm == NULL)
		return;

	free(pm->codes);
	free(pm);
}

PCODE_METHOD_TABLE * LoadPCodeMethodsFromFile(const char* file, void (*on_load)(PCODE_METHOD*,const char*, void*), void* param)
{
	FILE* f;
	PCODE_METHOD_TABLE* pmt = NULL;
	PCODE_METHOD *method;
	if(file == NULL)
		return NULL;
	
	f = fopen(file, "rb");
	if(f == NULL)
		return NULL;
	
	pmt = (PCODE_METHOD_TABLE*)calloc(1,sizeof(PCODE_METHOD_TABLE));
	
	int idx;
	fread(&pmt->method_count, 1, sizeof(int), f);
	if(pmt->method_count <= 0)
	{
		fclose(f);
		free(pmt);
		return NULL;
	}
	
	pmt->methods = (PCODE_METHOD**)calloc(1,sizeof(PCODE_METHOD*)*pmt->method_count);
	
	for(idx=pmt->method_count-1; idx>=0; idx--){
		char szname[256];
		int i;
		PCODE_METHOD *pm;
		for(i=0;;i++){
			szname[i] = fgetc(f);
			if(szname[i] == 0)
				break;
		}
		
		pm = (PCODE_METHOD*)calloc(1,sizeof(PCODE_METHOD));
		fread(&pm->argc, 1, sizeof(int),f);
		fread(&pm->code_len, 1, sizeof(int),f);
		pm->codes = (unsigned char*)malloc(pm->code_len);
		fread(pm->codes, pm->code_len, 1, f);
		pmt->methods[idx] = pm;
		if(on_load)
			(*on_load)(pm, szname, param);
	}

	fclose(f);

	return pmt;

}

void DeletePCodeMethods(PCODE_METHOD_TABLE* pmt)
{
	int i;
	if(pmt == NULL)
		return;

	for(i=0; i < pmt->method_count; i++)
	{
		DeletePCodeMethod(pmt->methods[i]);
	}

	free(pmt->methods);
	free(pmt);
}


void ResetPCodeExecute(PCODE_ENV* env)
{
	if(env == NULL || env->main == NULL)
		return ;

	while(env->calldepath > 0)
	{
		register unsigned int* stack;
		stack = env->stack + env->main->argc;
		env->stack = (unsigned int*)stack[0];
		env->main = (PCODE_METHOD*)stack[2];	
		env->calldepath --;
	}
	env->ip = 0;
	env->stack_top = env->main->argc;
}

#endif

