#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include <syslog.h>
#include <iomanip>
#include <string.h>

#include "aud-dec/Logger.h"

using namespace std;

const string LOGGER_FILE_NAME = "/var/log/aud_render";
const string LOGGER_EXT = ".log";

const int LOG_FILE_NAME_SIZE = 128;

const int LOG_SIZE_256K = 256*1024;
const int LOG_SIZE_512K = 512*1024;
const int LOG_SIZE_1M = 1024*1024;
const int LOG_SIZE_2M = 2*1024*1024;
const int LOG_SIZE_1K = 1024;
const int MAX_LOG_FILES = 100;

static string _file_name;
static ofstream _ostream;
static int _log_level = LOG_LEVEL_DEBUG;
static int _log_mode = LOG_STDOUT;
static int _log_size = LOG_SIZE_2M;
static int LOG_BUFFER_SIZE = 1024;
static int _log_index = 0;

void
Logger::Init()
{
    try {
        _file_name.clear();
        _file_name.append(LOGGER_FILE_NAME).append(LOGGER_EXT); 

        if ( 0 != access(_file_name.c_str(), 0)) {
            if (_ostream.is_open())
                _ostream.close();
        }
        else {
            if (_ostream.is_open())
                return;
        }

        _ostream.open(_file_name.c_str(), ios::out|ios::app);
        if (!_ostream.is_open()) {
            printf("failed to open log file[%s].\n", _file_name.c_str());
            return;
        }

        _ostream.seekp(0, ios::end);
    } catch (...) {
        printf("[%s] unexpected exception occur.\n", __FUNCTION__);
    }
}

void
Logger::LogOut(const int log_level, const char* format, ...)
{
    if (log_level > _log_level)
        return;

    try {

        char log_buffer[LOG_BUFFER_SIZE];
        va_list ap;
        va_start(ap, format);
        vsprintf(log_buffer, format, ap);
        va_end(ap);

        if (LOG_FILE == _log_mode) {
            // TODO:lock

            Init();
            if (_ostream.is_open() && _ostream.good()) {

                time_t t;;
                struct tm* tm_time;

                time(&t);
                tm_time = localtime(&t);
                if (NULL == tm_time) {
                    printf("[%s] failed to localtime.\n", __FUNCTION__);
                    return;
                }
                _ostream << (1900 + tm_time->tm_year) << "/"
                        << (1 + tm_time->tm_mon) << "/"
                        << tm_time->tm_mday << " "
                        << tm_time->tm_hour << ":"
                        << tm_time->tm_min << ":"
                        << tm_time->tm_sec << " ";

                _ostream<< log_buffer << endl;

                std::streampos pos = _ostream.tellp();
                if ((pos != -1) && pos >= _log_size) {
                    PrepareUploadLog();
                }
            }
        }
        else {
            printf("%s\n", log_buffer);
        }
    } catch (...) {
        printf("[%s] unexpected exception occur.\n", __FUNCTION__);
    }
}

void
Logger::PrepareUploadLog()
{
    char file_name[LOG_FILE_NAME_SIZE];
    memset(file_name, 0, LOG_FILE_NAME_SIZE*sizeof(char));

    try {
        _ostream.flush();
        _ostream.close();

        sprintf(file_name, "%s.%d%s", LOGGER_FILE_NAME.c_str(), _log_index++, LOGGER_EXT.c_str());

        if (0 != rename(_file_name.c_str(), file_name)) {
            printf("[%s] failed to rename file\n", __FUNCTION__);
        }

        Init();

    } catch(...) {
        printf("[%s]unexpected exception occur.\n", __FUNCTION__);
    }
}

int
Logger::SetLogLevel(const char* log_level)
{
    if (!log_level) {
        printf("[%s] log level is null.\n", __FUNCTION__);
        return -1;
    }

    if (0 == strcasecmp(log_level, "DEBUG")) {
         _log_level = LOG_LEVEL_DEBUG;
    } else if (0 == strcasecmp(log_level, "INFO")) {
         _log_level = LOG_LEVEL_INFO;
    } else if (0 == strcasecmp(log_level, "WARNING")) {
         _log_level = LOG_LEVEL_WARNING;
    } else if (0 == strcasecmp(log_level, "ERROR")) {
         _log_level = LOG_LEVEL_ERROR;
    }
    return 0;
}

int
Logger::SetLogMode(const char* log_mode)
{
    if (!log_mode) {
        printf("[%s] log mode is null.\n", __FUNCTION__);
        return -1;
    }

    if (0 == strcasecmp(log_mode, "STD"))
        _log_mode = LOG_STDOUT;
    else if (0 == strcasecmp(log_mode, "FILE"))
        _log_mode = LOG_FILE;
    else
        _log_mode = LOG_STDOUT;

    return 0;
}

int
Logger::SetLogSize(const char* log_size)
{
    if (!log_size) {
        printf("[%s] log size is null.\n", __FUNCTION__);
        return -1;
    }

    if (0 == strcasecmp(log_size, "256K"))
        _log_size = LOG_SIZE_256K;
    else if (0 == strcasecmp(log_size, "512K"))
        _log_size = LOG_SIZE_512K;
    else if (0 == strcasecmp(log_size, "1M"))
        _log_size = LOG_SIZE_1M;
    else if (0 == strcasecmp(log_size, "2M"))
        _log_size = LOG_SIZE_2M;
    else if (0 == strcasecmp(log_size, "1K"))
        _log_size = LOG_SIZE_1K;
    else
        _log_size = LOG_SIZE_1M;

    return 0;
}
