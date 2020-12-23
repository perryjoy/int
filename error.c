
#include "error.h"
#include <stdio.h>

int HandleWinapiError (int err_code);
int HandleTextError (int err_code);
int HandleMathError (int err_code);

int (*error_handling_funs[]) (int) = {HandleWinapiError, HandleTextError, HandleMathError};

void HandleError (int err_code)
{
    int i;
    for (i=0; i<sizeof(error_handling_funs)/sizeof(error_handling_funs[0]); i++)
    {
        if (error_handling_funs[i](err_code) == 0)
            break;
    }
}



int HandleWinapiError (int err_code)
{
    switch (err_code)
    {
    case DONE:
        printf("Trying to handle error when there is no such\n");
        return 0;
    case MEM_ERR_NULLPTR_DEREFERENCING:
        printf("Trying to dereference a nullptr\n");
        return 0;
    case MEM_ERR_FAILED_TO_ALLOCATE:
        printf("Failed to allocate memory\n");
        return 0;
    default:
        return 1;
    }
}


int HandleTextError (int err_code)
{
    switch (err_code)
    {
    case DONE:
        printf("Trying to handle error when there is no such\n");
        return 0;
    case MEM_ERR_NULLPTR_DEREFERENCING:
        printf("Trying to dereference a nullptr\n");
        return 0;
    case MEM_ERR_FAILED_TO_ALLOCATE:
        printf("Failed to allocate memory\n");
        return 0;
    default:
        return 1;
    }
}

int HandleMathError (int err_code)
{
    switch (err_code)
    {
    case MATH_ERR_DIVIDING_BY_ZERO:
        printf("Trying to divide by zero\n");
        return 0;
    default:
        return 1;
    }
}
