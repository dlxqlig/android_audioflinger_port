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
#ifndef _LIBS_UTILS_LOG_H
#define _LIBS_UTILS_LOG_H

#include <cutils/log.h>
#include <string>

namespace android {

enum LOG_LEVEL_ {
	LOG__INTERNAL_VERBOSE = 0,
	LOG__INTERNAL_DEBUG = 1,
	LOG__INTERNAL_INFO = 2,
	LOG__INTERNAL_WARN = 3,
	LOG__INTERNAL_ERROR = 4,
	LOG__INTERNAL_FATAL = 5,
};

#undef ALOG
#undef ALOGD
#undef ALOGI
#undef ALOGW
#undef ALOGV
#undef ALOGE
#undef LOGE
#undef LOGI
#undef LOGD
#undef LOGW
#undef LOGV


#define ALOG(...) do { log_print(LOG__INTERNAL_ERROR, __FUNCTION__, __VA_ARGS__); } while(0)
#define ALOGD(...) do { log_print(LOG__INTERNAL_DEBUG, __FUNCTION__, __VA_ARGS__); } while(0)
#define ALOGI(...) do { log_print(LOG__INTERNAL_INFO, __FUNCTION__, __VA_ARGS__); } while(0)
#define ALOGW(...) do { log_print(LOG__INTERNAL_WARN, __FUNCTION__, __VA_ARGS__); } while(0)
#define ALOGV(...) do { log_print(LOG__INTERNAL_VERBOSE, __FUNCTION__, __VA_ARGS__); } while(0)
#define ALOGE(...)  do { log_print(LOG__INTERNAL_ERROR, __FUNCTION__, __VA_ARGS__); } while(0)
#define LOGE(...)  do { log_print(LOG__INTERNAL_ERROR, __FUNCTION__, __VA_ARGS__); } while(0)
#define LOGI(...)  do { log_print(LOG__INTERNAL_INFO, __FUNCTION__, __VA_ARGS__); } while(0)
#define LOGD(...)  do { log_print(LOG__INTERNAL_DEBUG, __FUNCTION__, __VA_ARGS__); } while(0)
#define LOGW(...) do { log_print(LOG__INTERNAL_WARN, __FUNCTION__, __VA_ARGS__); } while(0)
#define LOGV(...)  do { log_print(LOG__INTERNAL_DEBUG, __FUNCTION__, __VA_ARGS__); } while(0)


void log_print(const int log_level, const char* fn, const char* format, ...);



#define ENABLE_ENTRY_LOGGER 1
using namespace std;
class EntryLogger {

public:
    EntryLogger(const char *pname): _name(pname) {
#if ENABLE_ENTRY_LOGGER
        if (!_name.empty())
        	printf("[%s] bgn.\n", _name.c_str());
#endif
    }

    virtual ~EntryLogger() {
#if ENABLE_ENTRY_LOGGER
        if (!_name.empty())
        	printf("[%s] end.\n", _name.c_str());;
#endif
    }

private:
    const std::string _name;
};

}
#endif // _LIBS_UTILS_LOG_H
