/* {{{ Copyright (C) 2014 The YunOS Project. All rights reserved. }}} */

#ifndef YUNOS_BASE_INCLUDE_LOG_LOG_H
#define YUNOS_BASE_INCLUDE_LOG_LOG_H

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef LOG
#define LOG(...) ((void)YUNOS_LOG(Error, LOG_TAG, __VA_ARGS__))
#endif

#ifndef YUNOS_LOG
#define YUNOS_LOG(priority, tag, ...)                                          \
  yunosLogPrint(kLogIdMain, kLogPriority##priority, tag, __VA_ARGS__)
#endif

/**
 * Represents the logging level.
 */
enum {
  kLogPriorityUnknown = 0,
  kLogPriorityDefault,
  kLogPriorityVerbose,
  kLogPriorityDebug,
  kLogPriorityInfo,
  kLogPriorityWarn,
  kLogPriorityError,
  kLogPriorityFatal,
  kLogPrioritySilent
};

enum {
  kLogIdMin = 0,

  kLogIdMain = 0,
  kLogIdRadio = 1,
  kLogIdEvents = 2,
  kLogIdSystem = 3,
  kLogIdCrash = 4,

  kLogIdMax
};

#define YUNOS_EXPORT __attribute__((visibility("default")))
#define YUNOS_INTERNAL __attribute__((visibility("hidden")))

YUNOS_EXPORT int yunosLogPrint(int id, int prio, const char *tag,
                               const char *fmt, ...)
    __attribute__((__format__(printf, 4, 5)));

#ifdef __cplusplus
}
#endif

#endif /* YUNOS_BASE_INCLUDE_LOG_LOG_H */
