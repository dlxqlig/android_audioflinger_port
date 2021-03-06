/*
 * Copyright (C) 2005 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//
// C/C++ logging functions.  See the logging documentation for API details.
//
// We'd like these to be available from C code (in case we import some from
// somewhere), so this has a C interface.
//
// The output will be correct when the log file is shared between multiple
// threads and/or multiple processes so long as the operating system
// supports O_APPEND.  These calls have mutex-protected data structures
// and so are NOT reentrant.  Do not use LOG in a signal handler.
//
#ifndef _LIBS_CUTILS_LOG_H
#define _LIBS_CUTILS_LOG_H

#include <stdio.h>
#include <unistd.h>

#define ALOG(...)
#define ALOGD(...)
#define ALOGI(...)
#define ALOGW(...)
#define ALOGV(...)
#define ALOGE(...)
#define LOGE(...)
#define LOGI(...)
#define LOGD(...)
#define LOGW(...)
#define LOGV(...)
#define LOG_ASSERT(...)
#define ALOG_ASSERT(...)
#define ALOGV_IF(...)
#define ALOGE_IF(...)
#define ALOGW_IF(...)
#define LOGE_IF(...)
#define LOGD_IF(...)
#define LOGW_IF(...)
#define LOG_ALWAYS_FATAL_IF(...)
#define LOG_ALWAYS_FATAL(...)
#define LOG_FATAL_IF(...)
#define IF_ALOGV() if (false)
#define IF_LOGV() if (false)
#define LOG_WARN(...)
#define LOG_FATAL(...)


#endif // _LIBS_CUTILS_LOG_H
