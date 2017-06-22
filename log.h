#ifndef __LOG_H
#define __LOG_H

#if ( defined(_LOG_ALL) )
#define LOG_LEVEL 0
#elif ( defined(_LOG_TRACE) )
#define LOG_LEVEL 1
#elif ( defined(_LOG_DEBUG) )
#define LOG_LEVEL 2
#elif ( defined(_LOG_INFO) )
#define LOG_LEVEL 3
#elif ( defined(_LOG_WARN) )
#define LOG_LEVEL 4
#elif ( defined(_LOG_ERROR) )
#define LOG_LEVEL 5
#elif ( defined(_LOG_FATAL) )
#define LOG_LEVEL 6
#elif ( defined(_LOG_NONE) )
#define LOG_LEVEL 7
#else
#define LOG_LEVEL 3
#endif

#if ( defined(_LOG_NO_COLOR) )
#define LOG_PREFIX_ALL   "   all"
#define LOG_PREFIX_TRACE " trace"
#define LOG_PREFIX_DEBUG " debug"
#define LOG_PREFIX_INFO  "  info"
#define LOG_PREFIX_WARN  "  warn"
#define LOG_PREFIX_ERROR " error"
#define LOG_PREFIX_FATAL " fatal"
#else
#define LOG_PREFIX_ALL   " " "\x1B[37m" "  all" "\x1B[0m"
#define LOG_PREFIX_TRACE " " "\x1B[36m" "trace" "\x1B[0m"
#define LOG_PREFIX_DEBUG " " "\x1B[34m" "debug" "\x1B[0m"
#define LOG_PREFIX_INFO  " " "\x1B[32m" " info" "\x1B[0m"
#define LOG_PREFIX_WARN  " " "\x1B[33m" " warn" "\x1B[0m"
#define LOG_PREFIX_ERROR " " "\x1B[31m" "error" "\x1B[0m"
#define LOG_PREFIX_FATAL " " "\x1B[35m" "fatal" "\x1B[0m"
#endif

#if LOG_LEVEL <= 0
#define log_all(fmt, ...) \
	log_(LOG_PREFIX_ALL, fmt, ##__VA_ARGS__)
#else
#define log_all(fmt, ...) (void) 0
#endif

#if LOG_LEVEL <= 1
#define log_trace(fmt, ...) \
	log_(LOG_PREFIX_TRACE, __FILE__ ":%d " fmt, __LINE__, ##__VA_ARGS__)
#else
#define log_trace(fmt, ...) (void) 0
#endif

#if LOG_LEVEL <= 2
#define log_debug(fmt, ...) \
	log_(LOG_PREFIX_DEBUG, __FILE__ ":%d " fmt, __LINE__, ##__VA_ARGS__)
#else
#define log_debug(fmt, ...) (void) 0
#endif

#if LOG_LEVEL <= 3
#define log_info(fmt, ...) \
	log_(LOG_PREFIX_INFO, fmt, ##__VA_ARGS__)
#else
#define log_info(fmt, ...) (void) 0
#endif

#if LOG_LEVEL <= 4
#define log_warn(fmt, ...) \
	log_(LOG_PREFIX_WARN, fmt, ##__VA_ARGS__)
#else
#define log_warn(fmt, ...) (void) 0
#endif

#if LOG_LEVEL <= 5
#include <string.h>
#include <errno.h>
#define log_error(fmt, ...) \
	log_(LOG_PREFIX_ERROR, __FILE__ ":%d " fmt, __LINE__, ##__VA_ARGS__)
#define log_perror(fmt, ...) \
	log_(LOG_PREFIX_ERROR, __FILE__ ":%d " fmt ": %s", __LINE__, ##__VA_ARGS__, strerror(errno))
#else
#define log_error(fmt, ...) (void) 0
#define log_perror(fmt, ...) (void) 0
#endif

#if LOG_LEVEL <= 6
#define log_fatal(fmt, ...) \
	log_(LOG_PREFIX_FATAL, __FILE__ ":%d " fmt, __LINE__, ##__VA_ARGS__)
#else
#define log_fatal(fmt, ...) (void) 0
#endif


int log_start(const char *filepath);
int log_end(void);
void log_(const char *prefix, const char *fmt, ...);

#endif
