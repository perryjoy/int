#ifndef __MAIN_H__
#define __MAIN_H__

#include <windows.h>


#ifdef BUILD_DLL
    #define DLL_EXPORT __declspec(dllexport)
#else
    #define DLL_EXPORT __declspec(dllimport)
#endif


#ifdef __cplusplus
extern "C"
{
#endif

    /*filename = name of the file to read from as C-string
    buffer = an adress of ptr to be set (allocation is put into this function)
    sz = size of the text read*/
    DLL_EXPORT int ReadToBufferA(char* filename, char** buffer, size_t* sz);

    /*filename = name of the file to read from as wChar-string
    buffer = an adress of ptr to be set (allocation is put into this function)
    sz = size of the text read*/
    DLL_EXPORT int ReadToBufferW(wchar_t* filename, char** buffer, size_t* sz);

#ifdef __cplusplus
}
#endif

#endif // __MAIN_H__
