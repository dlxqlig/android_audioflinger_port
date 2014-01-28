/*
 * Copyright (C) 2012 The Android Open Source Project
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

#define LOG_TAG "Log"

#include <utils/Log.h>

#include <stdarg.h>
#include <stdio.h>
#include <time.h>


namespace android {

#define  LOG_BUFFER_SIZE 512

int log_time(char* log_buffer)
{
    time_t t;;
    struct tm* tm_time;
    int log_size = 0;

    if (!log_buffer)
        return 0;

    time(&t);
    tm_time = localtime(&t);
    if (NULL == tm_time)
        return 0;

    log_size = snprintf(log_buffer, LOG_BUFFER_SIZE, "%4d%.2d%.2d %.2d:%.2d:%.2d ",
            (1900 + tm_time->tm_year),
            (1 + tm_time->tm_mon),
            tm_time->tm_mday,
            tm_time->tm_hour,
            tm_time->tm_min,
            tm_time->tm_sec);
    return log_size;

}

void log_print(const int log_level, const char* fn, const char* format, ...)
{
    char log_buffer[LOG_BUFFER_SIZE];
    int log_size = 0;
    log_size = log_time(log_buffer);

    va_list ap;
    va_start(ap, format);
    log_size += vsnprintf(log_buffer+log_size, LOG_BUFFER_SIZE-log_size, format, ap);
    va_end(ap);

    printf("%s\n", log_buffer);
}

} // namespace android
