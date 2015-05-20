/************************************
* file enc : utf8
* author   : wuyanyi09@gmail.com
* https://github.com/yanyiwu/limonp/blob/master/include/Logger.hpp
************************************/
#ifndef PAEKDUSAN_LOGGER_H
#define PAEKDUSAN_LOGGER_H

#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <stdio.h>
#include <cstdlib>
#include <cstdarg>
#include <ctime>
#include <cassert>

#define FILE_BASENAME strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__

#define LogDebug(fmt, ...) Paekdusan::Logger::LoggingF(Paekdusan::LL_DEBUG, FILE_BASENAME, __LINE__, fmt, ## __VA_ARGS__)
#define LogInfo(fmt, ...) Paekdusan::Logger::LoggingF(Paekdusan::LL_INFO, FILE_BASENAME, __LINE__, fmt, ## __VA_ARGS__)
#define LogWarn(fmt, ...) Paekdusan::Logger::LoggingF(Paekdusan::LL_WARN, FILE_BASENAME, __LINE__, fmt, ## __VA_ARGS__)
#define LogError(fmt, ...) Paekdusan::Logger::LoggingF(Paekdusan::LL_ERROR, FILE_BASENAME, __LINE__, fmt, ## __VA_ARGS__)
#define LogFatal(fmt, ...) Paekdusan::Logger::LoggingF(Paekdusan::LL_FATAL, FILE_BASENAME, __LINE__, fmt, ## __VA_ARGS__)

namespace Paekdusan {
    using namespace std;
    enum {
        LL_DEBUG = 0, LL_INFO = 1, LL_WARN = 2, LL_ERROR = 3, LL_FATAL = 4, LEVEL_ARRAY_SIZE = 5, CSTR_BUFFER_SIZE = 32
    };
    static const char * LOG_LEVEL_ARRAY[LEVEL_ARRAY_SIZE] = {"DEBUG", "INFO", "WARN", "ERROR", "FATAL"};
    static const char * LOG_FORMAT = "%s %s:%d %s %s\n";
    static const char * LOG_TIME_FORMAT = "%Y-%m-%d %H:%M:%S";

    class Logger {
    public:
        static void Logging(size_t level, const string& msg, const char* fileName, int lineno) {
            assert(level <= LL_FATAL);
            char buf[CSTR_BUFFER_SIZE];
            time_t timeNow;
            time(&timeNow);
            strftime(buf, sizeof(buf), LOG_TIME_FORMAT, localtime(&timeNow));
            fprintf(stderr, LOG_FORMAT, buf, fileName, lineno, LOG_LEVEL_ARRAY[level], msg.c_str());
        }
        static void LoggingF(size_t level, const char* fileName, int lineno, const char* const fmt, ...) {
#ifdef LOGGER_LEVEL
            if (level < LOGGER_LEVEL) return;
#endif
            int size = 256;
            string msg;
            va_list ap;
            while (1) {
                msg.resize(size);
                va_start(ap, fmt);
                int n = vsnprintf((char *) msg.c_str(), size, fmt, ap);
                va_end(ap);
                if (n > -1 && n < size) {
                    msg.resize(n);
                    break;
                }
                if (n > -1)
                    size = n + 1;
                else
                    size *= 2;
            }
            Logging(level, msg, fileName, lineno);
        }
    };
}

#endif