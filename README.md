# EMMA
Engine of Minimal Modules &amp; Abstractions.

### 🤔 What is EMMA?
EMMA is a type of engine for video game development, including a lot of libraries within it.

The engine have libraries and files that represents each one a major system.

**Here is the list of the libraries in EMMA:**
- Logger
- MECA
- Pokit

### ⭐ What is the purpose of EMMA?
EMMA is designed to be an easy-to-use and light engine for c++.

---
## Modules
### 🐞 Logger
**Something in your code it's not working and you don't know why?**

Logger will give you a helping hand! and you are going to know at any moment what's going on in your c++ programs.

This is a simple header-only library that helps you with the debugging.

#### How you use it?
At the moment, Logger gives you the option to output important information through the terminal and to benchmark your code.

First you have `Logger`, our signature dish! and it serves for outputting info:
```cpp
Logger(LOGGER_INFO, "MAIN", "Hello world!");
```
This outputs something like this:
```
[INFO]: {MAIN} Hello world! 
```
> As you can see, the 'MAIN' part is only a name for your current execution flow; you can name it anything you want.

For `Logger` there are four flags that you can use to categorize the output:
- `LOGGER_INFO`: Used to provide general information.
- `LOGGER_WARNING`: Used to provide information of something that could create glitches.
- `LOGGER_ERROR`: Used to provide information of something that could create bugs.
- `LOGGER_FATAL`: Used to provide information of something that could/will crash the program.

### 🧍 MECA
**Are you tired of always having a lot of entities that your game cannot handle?**

MECA (better known as MacMa's Entity Component Architecture :P) will make creating entities fun again! giving you a lot of customizations and optimizations.

This is a header-only library that uses concepts from the Entity Component System, Sparse sets, and so on. These fundamentals gives MECA a solid performance in high number and complexity of entities, but also making it easy to use.
