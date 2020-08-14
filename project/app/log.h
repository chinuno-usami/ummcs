#pragma once
#pragma GCC system_header

#include <stdio.h>

#define COLR_NONE "\033[0m"
#define COLR_RED "\033[0;31m"
#define COLR_GREEN "\033[0;32m"
#define COLR_YELLOW "\033[1;33m"


#define LOG_DEBUG(format, ...) do{\
    std::time_t t = std::time(0);\
    std::tm* now = std::localtime(&t);\
    fprintf(stderr, COLR_GREEN "%d-%02d-%02d %02d:%02d:%02d [DEBUG] [" format "] [%s,%s:%d]" COLR_NONE "\n",\
            now->tm_year+1900, \
            now->tm_mon+1, \
            now->tm_mday, \
            now->tm_hour, \
            now->tm_min, \
            now->tm_sec, \
            ## __VA_ARGS__, __FILE__, __func__, __LINE__); \
    }while(false)
#define LOG_WARN(format, ...) do{\
    std::time_t t = std::time(0);\
    std::tm* now = std::localtime(&t);\
    fprintf(stderr, COLR_YELLOW "%d-%02d-%02d %02d:%02d:%02d [WARN] [" format "] [%s,%s:%d]" COLR_NONE "\n",\
            now->tm_year+1900, \
            now->tm_mon+1, \
            now->tm_mday, \
            now->tm_hour, \
            now->tm_min, \
            now->tm_sec, \
            ## __VA_ARGS__, __FILE__, __func__, __LINE__); \
    }while(false)
#define LOG_ERROR(format, ...) do{\
    std::time_t t = std::time(0);\
    std::tm* now = std::localtime(&t);\
    fprintf(stderr, COLR_RED "%d-%02d-%02d %02d:%02d:%02d [ERROR] [" format "] [%s,%s:%d]" COLR_NONE "\n",\
            now->tm_year+1900, \
            now->tm_mon+1, \
            now->tm_mday, \
            now->tm_hour, \
            now->tm_min, \
            now->tm_sec, \
            ## __VA_ARGS__, __FILE__, __func__, __LINE__); \
    }while(false)
