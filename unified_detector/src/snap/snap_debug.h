/*
**************************************************************************************************
* Copyright (c) 2014 Mobile Communication Division, Samsung Electronics, Inc.
*
* All right reserved.
*
* This is the confidential and proprietary information of Samsung
* Electronics, Inc.
* ("Confidential Information"). You shall not disclose such Confidential
* Information and shall use it only in accordance with the terms of
* the license agreement you entered into with Samsung Electronics.
*
**************************************************************************************************/

/*!
\file    srib_debug.h
\brief   Declaration of Macros to use for debugging purposes
\author  Manoj Kumar M(manoj.kumar5@samsung.com)
\date    2018/08/03
<b>Revision History: </b>
- 2018/08/03 : Manoj Kumar M(manoj.kumar5@samsung.com) \n
Initial Release
*/

#pragma once

#include <chrono>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/system_properties.h>
#include <unistd.h>

typedef enum {
  LOG_NONE = 0,
  LOG_API = (1 << 0),
  LOG_MODULE = (1 << 1),
  LOG_SUBMODULE = (1 << 2),
  LOG_DETAIL = (1 << 3)
} IcsLogType;

#ifndef _WIN32
#ifndef LOG_TAG
#define LOG_TAG "DIS_DEBUG"
#endif

#include <android/log.h>

#define PRINT_INFO(...)                                                        \
  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define PRINT_DBG(...)                                                         \
  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define PRINT_WARN(...)                                                        \
  __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define PRINT_ERR(...)                                                         \
  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#else
#define PRINT_INFO(...) printf(__VA_ARGS__), printf("\n")
#define PRINT_DBG(...) printf(__VA_ARGS__), printf("\n")
#define PRINT_WARN(...) printf(__VA_ARGS__), printf("\n")
#define PRINT_ERR(...) printf(__VA_ARGS__), printf("\n")
#endif