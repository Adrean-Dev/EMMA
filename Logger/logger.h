#pragma once
#include <iostream>
#include <chrono>


#define Log(level, sys, msg) std::cout << level << "{" << sys << "} " << msg << std::endl

#define INFO "[INFO]: " //Used to provide general information.
#define WARNING "[WARNING]: " //Used to provide information of something that could create glitches.
#define ERROR "[ERROR]: " //Used to provide information of something that could create bugs.
#define FATAL "[FATAL]: " //Used to provide information of something that could/will crash the program.


#define MICRO_SECONDS std::chrono::microseconds
#define MILLI_SECONDS std::chrono::milliseconds
#define SECONDS std::chrono::seconds

#define TimeShot std::chrono::steady_clock::now

#define elapsed_time(start, end, timeClip) std::chrono::duration_cast<timeClip>(end-start).count()