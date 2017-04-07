/*************************************************************************
* Copyright (C), 2015,
* XXXXXXXXXXX Co.
* 文件名: Library.h
* 作  者: zhangl		版本：1.0		日  期：2015/06/08
* 描  述：动态库调用函数定义（兼容Window/Linux）
* 其  他：
* 功能列表:
*   1.
* 修改日志：
*   No.    日期		作者			修改内容
*************************************************************************/
#ifndef LIBRARY_DEF_H
#define LIBRARY_DEF_H

#ifdef _WIN32    // For Windows
/************************************************************************/
/* 1. 头文件引用                                                        */
/************************************************************************/
#include "windows.h"

/************************************************************************/
/* 2. 动态库调用使用库函数再定义                                        */
/************************************************************************/
#define LIB_HANDLE     HINSTANCE        // 动态库句柄
#define Lib_Open(s)    LoadLibrary(s)   // 加载动态库
#define Lib_Close      FreeLibrary      // 关闭动态库
#define Lib_GetFun     GetProcAddress   // 获取动态库方法地址
#define Lib_GetErr     GetLastError     // 获取错误码

/************************************************************************/
/* 3. 动态库文件宏定义                                                  */
/************************************************************************/
#define NWFD_GRIB_LIB      "nwfd-grib2-win32.dll"
#define NWFD_MEMCACHEIMPL  "NwfdMemCachedImpl.dll"
#define NWFD_REDISIMPL     "NwfdRedisImpl.dll"
#define NWFD_STATION_LIB   "NwfdStationLib.dll"

#else  // For Linux
/************************************************************************/
/* 1. 头文件引用                                                        */
/************************************************************************/
#include "dlfcn.h"

/************************************************************************/
/* 2. 动态库调用使用库函数再定义                                        */
/************************************************************************/
#define LIB_HANDLE     void*                 // 动态库句柄
#define Lib_Open(s)    dlopen(s,RTLD_LAZY)   // 加载动态库
#define Lib_Close      dlclose               // 关闭动态库
#define Lib_GetFun     dlsym                 // 获取动态库方法地址
#define Lib_GetErr     dlerror               // 获取错误码

/************************************************************************/
/* 3. 动态库文件宏定义                                                  */
/************************************************************************/
#define NWFD_GRIB_LIB      "libNwfd-grib2.so"
#define NWFD_MEMCACHEIMPL  "libNwfdMemCachedImpl.so"
#define NWFD_REDISIMPL     "libNwfdRedisImpl.so"
#define NWFD_STATION_LIB   "libNwfdStation.so"
#endif

/************************************************************************/
/* 1. dll名称字符串转换                                                 */
/************************************************************************/
#ifdef UNICODE
#define Lib_Str(str)  str.toStdWString().data()
#else
#define Lib_Str(str)  str.toStdString().data()
#endif

#endif //LIBRARY_DEF_H
