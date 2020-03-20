//
// Created by wu on 3/18/20.
//

#include "CurrentThread.h"

namespace CurrentThread{
    __thread int  t_cachedTid = 0;
    __thread char t_tidString[32];
    __thread int  t_tidStringLength = 0;
    __thread const char* t_threadName = "Unkonwn";
}