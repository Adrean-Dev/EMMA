#pragma once
#include <iostream>
#include <chrono>


#define Logger(level, sys, msg) std::cout << level << "{" << sys << "} " << msg << std::endl

#define LOGGER_INFO "[INFO]: " //Used to provide general information.
#define LOGGER_WARNING "[WARNING]: " //Used to provide information of something that could create glitches.
#define LOGGER_ERROR "[ERROR]: " //Used to provide information of something that could create bugs.
#define LOGGER_FATAL "[FATAL]: " //Used to provide information of something that could/will crash the program.


#define LOGGER_MICRO_SECONDS std::chrono::microseconds
#define LOGGER_MILLI_SECONDS std::chrono::milliseconds
#define LOGGER_SECONDS std::chrono::seconds

#define TimeShot std::chrono::steady_clock::now

#define elapsed_time(start, end, timeClip) std::chrono::duration_cast<timeClip>(end-start).count()