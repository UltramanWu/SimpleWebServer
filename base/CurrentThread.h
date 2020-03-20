//
// Created by wu on 3/18/20.
//

#ifndef SIMPLEWEBSERVER_CURRENTTHREAD_H
#define SIMPLEWEBSERVER_CURRENTTHREAD_H

#include <stdint.h>
#include <termio.h>
#include <unistd.h>
#include <syscall.h>
#include <cstdio>

namespace CurrentThread{

    extern __thread int  t_cachedTid;
    extern __thread char t_tidString[32];
    extern __thread int  t_tidStringLength;
    extern __thread const char* t_threadName;

    inline int tid(){
        if(__builtin_expect(t_cachedTid==0,0)){
            if(t_cachedTid == 0){
                t_cachedTid = static_cast<pid_t>(::syscall(SYS_gettid));
                t_tidStringLength = snprintf(t_tidString, sizeof(t_tidString),"%5d",t_cachedTid);
            }
        }
        return t_cachedTid;
    }

    inline const char* tidString(){
        return t_tidString;
    }

    inline int tidStringLength(){
        return t_tidStringLength;
    }

    inline const char* name() { return t_threadName; }

}


#endif //SIMPLEWEBSERVER_CURRENTTHREAD_H
