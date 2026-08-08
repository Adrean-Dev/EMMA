#pragma once
#include <iostream>


#define Log(level, sys, msg) std::cout << level << "{" << sys << "} " << msg << std::endl


#define INFO "[INFO]: " //Used to provide general information.
#define WARNING "[WARNING]: " //Used to provide information of something that could create glitches.
#define ERROR "[ERROR]: " //Used to provide information of something that could create bugs.
#define FATAL "[FATAL]: " //Used to provide information of something that could/will crash the program.
