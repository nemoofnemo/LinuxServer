#ifndef SVRLIB_H
#define SVRLIB_H

#include <unistd.h>
#include <time.h>
#include <string.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <utility>
#include <list>
#include <array>
#include <vector>
#include <map>
#include <set>
#include <regex>
#include <pthread.h>
#include <sys/time.h>
#include <stdarg.h>

//class Object should be base class of every other class.
class Object {
public:
    Object() {

    }

    virtual ~Object() {

    }
};

#endif // SVRLIB_H
