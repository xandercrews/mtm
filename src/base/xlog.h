#ifndef _BASE_XLOG_H_
#define _BASE_XLOG_H_

#include "base/interp.h"

class error_event;

/**
 * See https://wiki.adaptivecomputing.com:8443/display/ENG/Log+Levels
 */
enum loglevel_t {
	ll_fatal,
	ll_error,
	ll_warning,
	ll_info,
	ll_trace1,
	ll_trace2,
	ll_trace3,
	ll_debug
};

/**
 * Logging looks a little mysterious here, but in practice it's super easy
 * to use. For example:
 *
 * xlog("hello world");
 * xlog("Hello, %1{your_name}. My name = %2{my_name}", your_name, my_name);
 * xlog(my_error);
 *
 * The audience of xlog() statements is end-users. Communicate accordingly.
 */
#define STD_CONTEXTUAL_PARAMS char const * source_fname, char const * func, \
		unsigned source_line

#define xlog(msg, ...) fprintf(stderr, "%s\n", interp(msg, ##__VA_ARGS__).c_str())
	//__xlog(__FILE__, __func__, __LINE__, msg, ##__VA_ARGS__)
DECLARE_INTERP_FUNC(void, __xlog,,STD_CONTEXTUAL_PARAMS,);
void __xlog(STD_CONTEXTUAL_PARAMS, std::exception const & error);
void __xlog(STD_CONTEXTUAL_PARAMS, error_event const & error);

typedef void (*xlog_handler_func)(loglevel_t level, STD_CONTEXTUAL_PARAMS,
		std::string const & msg);

/**
 * Specify a function that will get called when something needs to be xlogged.
 */
void register_xlog_handler(xlog_handler_func, bool activate);

void xlog_to_stdout(loglevel_t level, STD_CONTEXTUAL_PARAMS,
		std::string const & msg);

/**
 * Decide whether a particular trace message is worth formatting and persisting.
 *
 * @param level
 * @param topics
 *     A list of topics that are relevant to this message. If any of the topics
 *     is being xlogged in a granular fashion, it will cause the function to
 *     return true.
 *
 * This function is thread-safe.
 */
bool should_trace(loglevel_t level, char const * topics = 0);

/**
 * Provide granular control over tracing, on a per-topic basis.
 *
 * @param level
 *     What level is desired?
 * @param topics
 *     Which topic or topics are being adjusted. If topics is null or empty,
 *     then all tracing on all topics are adjusted. If topics is a comma-
 *     separated list or single word/phrase, then tracing granularity for each
 *     named topic in the list is adjusted.
 *
 * This function is thread-safe.
 */
void set_trace_level(loglevel_t level, char const * topics = 0);

/**
 * Tracing works the same way as xlogging, except that the audience is
 * internal employees, and each potential trace message has a level and zero
 * or more associated topics. These are used to short-circuit tracing as
 * appropriate.
 */
#define trace(lvl, msg, topics, ...)
//	if should_trace(lvl, topics)
//		__trace(lvl, topics, __FILE__, __func__, __LINE__, msg, ##__VA_ARGS__)

DECLARE_INTERP_FUNC(void, __trace,,loglevel_t level, char const * topics,
		STD_CONTEXTUAL_PARAMS,);

#endif // sentry
