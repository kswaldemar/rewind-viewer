//
// Created by valdemar on 21.10.17.
//

#pragma once

#include <loguru.hpp>

//@formatter:off
#define LOG_INFO(format, ...)   LOG_F(INFO,      format, ##__VA_ARGS__)
#define LOG_WARN(format, ...)   LOG_F(WARNING,   format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...)  LOG_F(ERROR,     format, ##__VA_ARGS__)
#define LOG_FATAL(format, ...)  LOG_F(FATAL,     format, ##__VA_ARGS__)
#define LOG_DEBUG(format, ...)  LOG_F(1,         format, ##__VA_ARGS__)
#define LOG_V2(format, ...)     LOG_F(2,         format, ##__VA_ARGS__)
#define LOG_V3(format, ...)     LOG_F(3,         format, ##__VA_ARGS__)
#define LOG_V8(format, ...)     LOG_F(8,         format, ##__VA_ARGS__)
#define LOG_V9(format, ...)     LOG_F(9,         format, ##__VA_ARGS__)
//@formatter:on
