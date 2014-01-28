#include <fstream>
#include <string>

using namespace std;

typedef enum _LOG_LEVEL
{
    LOG_LEVEL_INVALID = -1,
    LOG_LEVEL_ERROR = 0,
    LOG_LEVEL_WARNING = 1,
    LOG_LEVEL_INFO = 2,
    LOG_LEVEL_DEBUG = 3
} LOG_LEVEL;

typedef enum _LOG_MODE
{
    LOG_STDOUT = 0,
    LOG_FILE = 1
} LOG_MODE;

class Logger {

public:
    Logger(){};
    virtual ~Logger(){};

    static int SetLogLevel(const char* plevel);
    static int SetLogMode(const char* log_mode);
    static int SetLogSize(const char* log_size);

    static void LogOut(const int log_level, const char* format, ...);

private:
    Logger(const Logger&);
    void operator=(const Logger&);

    static void Init();
    static void PrepareUploadLog();

};

#define ENABLE_ENTRY_LOGGER 0
class EntryLogger {

public:
    EntryLogger(const char *pname): _name(pname) {
#if ENABLE_ENTRY_LOGGER
        if (!_name.empty())
            Logger::LogOut(LOG_LEVEL_ERROR, "%s, bgn", _name.c_str());
#endif
    }

    virtual ~EntryLogger() {
#if ENABLE_ENTRY_LOGGER
        if (!_name.empty())
            Logger::LogOut(LOG_LEVEL_ERROR, "%s, end", _name.c_str());
#endif
    }

private:
    const std::string _name;
};

