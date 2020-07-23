#pragma once
#pragma GCC system_header

#include <stdio.h>

#define COLR_NONE "\033[0m"
#define COLR_RED "\033[0;31m"
#define COLR_GREEN "\033[0;32m"
#define COLR_YELLOW "\033[1;33m"


#define LOG_DEBUG(format, ...) fprintf(stderr, COLR_GREEN "[DEBUG] [" format "] [%s,%s:%d]" COLR_NONE "\n", ## __VA_ARGS__, __FILE__, __func__, __LINE__)
#define LOG_WARN(format, ...) fprintf(stderr, COLR_YELLOW "[WARN ] [" format "] [%s,%s:%d]" COLR_NONE "\n", ## __VA_ARGS__, __FILE__, __func__, __LINE__)
#define LOG_ERROR(format, ...) fprintf(stderr, COLR_RED "[ERROR] [" format "] [%s,%s:%d]" COLR_NONE "\n", ## __VA_ARGS__, __FILE__, __func__, __LINE__)
