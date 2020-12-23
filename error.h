
#ifndef ERROR_H_INCLUDED
#define ERROR_H_INCLUDED
#define DONE 0


#ifdef USING_DBG
#include <stdio.h>
#define REPORT(err) {printf( "error code %i returned, line %d, file(%s)\n", err, __LINE__, __FILE__ ); \
HandleError(err);}
#else
#define REPORT(err) {HandleError(err);}
#endif // USING_DBG



typedef enum mem_err
{
    MEM_ERR_NULLPTR_DEREFERENCING = 1,
    MEM_ERR_HANGING_POINTER_OCCURED,
    MEM_ERR_FAILED_TO_ALLOCATE
} mem_err;

typedef enum winapi_err
{
    WINAPI_ERR_WIN_HANGING_HANDLER = 100,
    WINAPI_ERR_HDC_DATA_UNREACHED
} winapi_err;

typedef enum math_err
{
    MATH_ERR_DIVIDING_BY_ZERO = 200,
} math_err;


typedef enum env_err
{
  ENV_ERR_FAILED_TO_FIND = 300
} env_err;


void HandleError (int err_code);


#endif // ERROR_H_INCLUDED
