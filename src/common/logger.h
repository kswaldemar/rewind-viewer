//
// Created by valdemar on 21.10.17.
//

#pragma once

#include <cstdio>

#define LOG(type, format, ...) {fprintf(stderr, type " " format "\n", ##__VA_ARGS__);}

#ifndef NDEBUG
#  define LOG_INFO(format, ...)  LOG("\033[1;34m" "INFO"    "\033[0m::", format, ##__VA_ARGS__);
#  define LOG_DEBUG(format, ...) LOG("DEBUG::",                          format, ##__VA_ARGS__);
#else
#  define LOG_INFO
#  define LOG_DEBUG
#endif

#define LOG_WARN(format, ...)  LOG("\033[1;33m" "WARNING" "\033[0m::", format, ##__VA_ARGS__);
#define LOG_ERROR(format, ...) LOG("\033[1;31m" "ERROR"   "\033[0m::", format, ##__VA_ARGS__);
