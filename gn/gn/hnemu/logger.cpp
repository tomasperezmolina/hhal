#include <cstdio>
#include <cstdarg>

#include "logger.h"

#define LOGGER_LEVEL 7

namespace mango {



void ConsoleLogger::Debug(const char *fmt, ...) {
#if LOGGER_LEVEL > 7
    va_list argptr;
    va_start(argptr, fmt);
    fprintf(stderr, "[D] ");
    vfprintf(stderr, fmt, argptr);
    fprintf(stderr, "\n");
    va_end(argptr);
#endif
}

void ConsoleLogger::Info(const char *fmt, ...) {
#if LOGGER_LEVEL > 6
    va_list argptr;
    va_start(argptr, fmt);
    fprintf(stderr, "[I] ");
    vfprintf(stderr, fmt, argptr);
    fprintf(stderr, "\n");
    va_end(argptr);
#endif
}

void ConsoleLogger::Notice(const char *fmt, ...) {
#if LOGGER_LEVEL > 5
    va_list argptr;
    va_start(argptr, fmt);
    fprintf(stderr, "[N] ");
    vfprintf(stderr, fmt, argptr);
    fprintf(stderr, "\n");
    va_end(argptr);
#endif
}

void ConsoleLogger::Warn(const char *fmt, ...) {
#if LOGGER_LEVEL > 4
    va_list argptr;
    va_start(argptr, fmt);
    fprintf(stderr, "[W] ");
    vfprintf(stderr, fmt, argptr);
    fprintf(stderr, "\n");
    va_end(argptr);
#endif
}


void ConsoleLogger::Error(const char *fmt, ...) {
#if LOGGER_LEVEL > 3
    va_list argptr;
    va_start(argptr, fmt);
    fprintf(stderr, "[E] ");
    vfprintf(stderr, fmt, argptr);
    fprintf(stderr, "\n");
    va_end(argptr);
#endif
}

void ConsoleLogger::Crit(const char *fmt, ...) {
#if LOGGER_LEVEL > 2
    va_list argptr;
    va_start(argptr, fmt);
    fprintf(stderr, "[C] ");
    vfprintf(stderr, fmt, argptr);
    fprintf(stderr, "\n");
    va_end(argptr);
#endif
}

void ConsoleLogger::Alert(const char *fmt, ...) {
#if LOGGER_LEVEL > 1
    va_list argptr;
    va_start(argptr, fmt);
    fprintf(stderr, "[A] ");
    vfprintf(stderr, fmt, argptr);
    fprintf(stderr, "\n");
    va_end(argptr);
#endif
}

void ConsoleLogger::Fatal(const char *fmt, ...) {
#if LOGGER_LEVEL > 0
    va_list argptr;
    va_start(argptr, fmt);
    fprintf(stderr, "[F] ");
    vfprintf(stderr, fmt, argptr);
    fprintf(stderr, "\n");
    va_end(argptr);
#endif
}

}


