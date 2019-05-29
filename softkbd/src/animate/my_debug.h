#ifndef __MY_DEBUG_H__
#define __MY_DEBUG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>

#ifdef DEBUG_TO_STDOUT    
    #define DEBUG_OUT_FD stdout
#else
    #define DEBUG_OUT_FD stderr
#endif


#ifdef FUN_DEBUG

    #define FUN_CALLED() fprintf(DEBUG_OUT_FD,  \
			 "{}-(%s)---(%s: %04d)-------\n", \
			 __FUNCTION__, __FILE__, __LINE__)

	#define FUN_START()  \
    do { \
        fprintf(DEBUG_OUT_FD, "{-(%s+)---(%s: %04d)-------\n", \
                __FUNCTION__, __FILE__, __LINE__); \
    }while (0)

	#define FUN_END()  \
    do { \
        fprintf(DEBUG_OUT_FD, "}-(%s+)---(%s: %04d)-------\n", \
                __FUNCTION__, __FILE__, __LINE__); \
    }while (0)

    /*usage: FUN_STEP(2,3,2) */
    #define FUN_STEP(args...) _fun_step_impl(__FUNCTION__, __FILE__,\
            __LINE__, #args)
    
    static void _fun_step_impl (const char* fun, const char* file, 
                    int line, char* args_msg)
    {
        char* ch = args_msg;
    
        fprintf(DEBUG_OUT_FD, "(%s)---STEP-", fun);
    
        if (ch != NULL)
        {
            while (*ch != '\0')
            {
                if (isalnum (*ch))
                {
                    fprintf(DEBUG_OUT_FD, "%c%c%c%c%c-", *ch, *ch, *ch,*ch ,*ch);
                }
                ch++;
            }
        }
        fprintf(DEBUG_OUT_FD, "-SETP---(%s: %d)\n", file, line);
    }

#else


	#define FUN_START()       
	#define FUN_END()	     
    #define FUN_STEP(args...)

#endif
/****************************************************************/

#ifdef VAL_DEBUG

    /*usage: TEST_VAL(abc, %f) */
	#define TEST_VAL(val, FORMAT) \
			fprintf(DEBUG_OUT_FD, "[%s]=<%s "#FORMAT">---(%s)-(%s: %d)--;\n", #val, #FORMAT, (val), \
                                        __FUNCTION__, __FILE__, __LINE__);

    /*usage: TEST_VALS("%d", abc, "%f", def) */
    #define TEST_VALS(args...) _test_vals(__FUNCTION__, __FILE__, __LINE__,\
                                          #args, ##args)

    typedef enum
    {
        PRINT_TYPE_ERROR = -1,
        PRINT_TYPE_INT,
        PRINT_TYPE_DOUBLE,
        PRINT_TYPE_STR,
    }PRINT_TYPE;

    #define _PRINT_A_VAL(fmt, name, type) \
            do \
            { \
                fprintf(DEBUG_OUT_FD, "[%s]-<%s ",name, fmt); \
                fprintf(DEBUG_OUT_FD, fmt, (type)va_arg(ap, type)); \
                fprintf(DEBUG_OUT_FD, ">  "); \
            }while (0)
    
    
    
    static PRINT_TYPE get_printtype_from_fmt(char fmt_char)
    {
        switch(fmt_char)
        {
                case 'd':
                case 'u':
                case 'o':
                case 'x':
                case 'X':
                case 'c':
                    return PRINT_TYPE_INT;
    
                case 'f':    
                case 'e':    
                case 'E':    
                case 'g':    
                case 'G':    
                    return PRINT_TYPE_DOUBLE;
    
                case 's':
                    return PRINT_TYPE_STR;
                    
                default:
                    assert (0);
                    return PRINT_TYPE_ERROR;
        }
    }
    
    
    
    static void get_varname_from_args(char* arg_names, char** start, char** tail)
    {
        char* arg_start;
        char* arg_tail = arg_names;
    
        /*go to "%."*/
        arg_start = arg_names + strspn(arg_names, ", ");
        arg_tail = arg_start + strcspn(arg_start, ", ");
    
        /*go to variable name*/
        arg_start = arg_tail + strspn(arg_tail, ", ");
        arg_tail = arg_start + strcspn(arg_start, ", ");
    
        /*if there is no ',' or ' ' , arg_tail to the tailed '\0'*/
        
        *arg_tail = '\0';
    
        *start = arg_start;
        *tail = arg_tail;
    }
    
    static void _test_vals(const char* fun, const char* file, int line, 
                    char* args_msg, ...)
    {
        va_list ap;
    
        /*go to the first arg name(%...)*/
        char* alloced_str = (char*)malloc(strlen(args_msg)+1);
        char* var_name_start;
        char* var_name_tail;
        char* fmt;
        
        va_start(ap, args_msg);
        
        alloced_str[0] = '\0';
        strcat(alloced_str, args_msg);
       
        get_varname_from_args(alloced_str, &var_name_start, &var_name_tail);
        
        while(var_name_tail != var_name_start)
        {
            /*find a variable name*/
    
            fmt = (char*)va_arg(ap, char*);
            switch (get_printtype_from_fmt (fmt[strlen(fmt) - 1]))
            {
                case PRINT_TYPE_INT:
                    _PRINT_A_VAL(fmt, var_name_start, int);
                        break;
    
                case PRINT_TYPE_DOUBLE:
                    _PRINT_A_VAL(fmt, var_name_start, double);
                    break;
    
                case PRINT_TYPE_STR:
                    _PRINT_A_VAL(fmt, var_name_start, int);
                    break;
            }
            get_varname_from_args(var_name_tail+1, &var_name_start, &var_name_tail);
        } 
        fprintf(DEBUG_OUT_FD, ".......(%s).(%s: %d)\n", fun, file, line);
    
        free(alloced_str);
        va_end(ap);
    }

#else

	#define TEST_VAL(val, FORMAT)	
    #define TEST_VALS(args...)    

#endif

/****************************************************************/

#ifdef INFO_DEBUG

	#define TEST_INFO(str) \
			fprintf(DEBUG_OUT_FD, "%s;\n", (str))

	#define TEST_SENTENCE(sent) \
			fprintf(DEBUG_OUT_FD, "%s;\n", #sent); \
			sent

#else

	#define TEST_INFO(str)		
	#define TEST_SENTENCE(sent)	sent

#endif

/****************************************************************/

#ifdef __cplusplus
}
#endif

#endif
