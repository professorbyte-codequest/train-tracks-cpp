#pragma once

#include <sys/time.h>
#include <iostream>
#include <sstream>

#define STRINGIZE(arg) STRINGIZE1(arg)
#define STRINGIZE1(arg) STRINGIZE2(arg)
#define STRINGIZE2(arg) #arg

#define CONCATENATE(arg1, arg2) CONCATENATE1(arg1, arg2)
#define CONCATENATE1(arg1, arg2) CONCATENATE2(arg1, arg2)
#define CONCATENATE2(arg1, arg2) arg1##arg2

#define FOR_EACH_1(what, x) what(x)
#define FOR_EACH_2(what, x, ...) \
    what(x);                     \
    FOR_EACH_1(what, __VA_ARGS__);
#define FOR_EACH_3(what, x, ...) \
    what(x);                     \
    FOR_EACH_2(what, __VA_ARGS__);
#define FOR_EACH_4(what, x, ...) \
    what(x);                     \
    FOR_EACH_3(what, __VA_ARGS__);
#define FOR_EACH_5(what, x, ...) \
    what(x);                     \
    FOR_EACH_4(what, __VA_ARGS__);
#define FOR_EACH_6(what, x, ...) \
    what(x);                     \
    FOR_EACH_5(what, __VA_ARGS__);
#define FOR_EACH_7(what, x, ...) \
    what(x);                     \
    FOR_EACH_6(what, __VA_ARGS__);
#define FOR_EACH_8(what, x, ...) \
    what(x);                     \
    FOR_EACH_7(what, __VA_ARGS__);

#define FOR_EACH_NARG(...) FOR_EACH_NARG_(__VA_ARGS__, FOR_EACH_RSEQ_N())
#define FOR_EACH_NARG_(...) FOR_EACH_ARG_N(__VA_ARGS__)
#define FOR_EACH_ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, N, ...) N
#define FOR_EACH_RSEQ_N() 8, 7, 6, 5, 4, 3, 2, 1, 0

#define FOR_EACH_(N, what, ...) CONCATENATE(FOR_EACH_, N)(what, __VA_ARGS__)
#define FOR_EACH(what, ...) FOR_EACH_(FOR_EACH_NARG(__VA_ARGS__), what, __VA_ARGS__)

#define LN_TRACE_ARG_TO_OSTR(x) ostr << STRINGIZE(x) << "=" << x << " ";


#define LOG(tp, ...) do {                                                                                \
                                                                                                                \
    struct timeval tv;                                                                                           \
    gettimeofday(&tv, NULL);                                                                                     \
    struct tm lt;                                                                                                \
    localtime_r(&tv.tv_sec, &lt);                                                                                \
    std::ostringstream ostr;                                                                                     \
    ostr << tp << " : ";                                                                              \
    FOR_EACH(LN_TRACE_ARG_TO_OSTR, __VA_ARGS__);                                                                 \
    fprintf(stderr, "%02d/%02d/%02d %02d:%02d:%02d.%06ld %s\n", lt.tm_mon + 1, lt.tm_mday, lt.tm_year % 100,      \
            lt.tm_hour, lt.tm_min, lt.tm_sec, (long)tv.tv_usec, ostr.str().c_str());                                   \
} while (0)
