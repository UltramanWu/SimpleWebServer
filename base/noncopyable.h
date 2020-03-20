//
// Created by wu on 3/18/20.
//

#ifndef SIMPLEWEBSERVER_NONCOPYABLE_H
#define SIMPLEWEBSERVER_NONCOPYABLE_H

class noncopyable {
protected:
    noncopyable(){}
    ~noncopyable(){}

private:
    noncopyable(const noncopyable&);
    noncopyable&operator=(const noncopyable&);
};



#endif //SIMPLEWEBSERVER_NONCOPYABLE_H
