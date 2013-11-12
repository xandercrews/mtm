#include <algorithm>
#include <mutex>
#include <vector>

#include "base/xlog.h"

using namespace std;

typedef std::vector<xlog_handler_func> handlers;

static handlers & _get_xlog_handlers() {
	static handlers the_handlers;
	return the_handlers;
}

mutex lh_mutex;

void register_xlog_handler(xlog_handler_func func, bool activate) {
	lock_guard<mutex> lock(lh_mutex);
	handlers & h = _get_xlog_handlers();
	auto found = std::find(h.begin(), h.end(), func);
	if (activate) {
		if (found == h.end()) {
			h.push_back(func);
		}
	} else {
		if (found != h.end()) {
			h.erase(found);
		}
	}
}

#define STD_CONTEXTUAL_ARGS source_fname, func, source_line

void __xlog__(loglevel_t level, STD_CONTEXTUAL_PARAMS, string const & msg) {
	lock_guard<mutex> lock(lh_mutex);
	for (auto callback: _get_xlog_handlers()) {
		callback(level, STD_CONTEXTUAL_ARGS, msg);
	}
}

void __xlog(STD_CONTEXTUAL_PARAMS, char const * msg, MANY_ARGS_IMPL) {
	arg const * args[] = {MANY_ARGS_PTR_LIST};
	string txt;
	interp_into(txt, msg, args, 9);
	__xlog__(ll_info/*TODO:fix*/, STD_CONTEXTUAL_ARGS, txt);
}

void __xlog(STD_CONTEXTUAL_PARAMS, char const * msg, THREE_ARGS) {
	arg const * args[] = {&a1, &a2, &a3};
	string txt;
	interp_into(txt, msg, args, 3);
	__xlog__(ll_info/*TODO:fix*/, STD_CONTEXTUAL_ARGS, txt);
}

void __xlog(STD_CONTEXTUAL_PARAMS, char const * msg, TWO_ARGS) {
	arg const * args[] = {&a1, &a2};
	string txt;
	interp_into(txt, msg, args, 2);
	__xlog__(ll_info/*TODO:fix*/, STD_CONTEXTUAL_ARGS, txt);
}

void __xlog(STD_CONTEXTUAL_PARAMS, char const * msg, ONE_ARG) {
	arg const * args[] = {&a1};
	string txt;
	interp_into(txt, msg, args, 1);
	__xlog__(ll_info/*TODO:fix*/, STD_CONTEXTUAL_ARGS, txt);
}

void __xlog(STD_CONTEXTUAL_PARAMS, char const * msg) {
	__xlog__(ll_info, STD_CONTEXTUAL_ARGS, msg);
}

void __xlog(STD_CONTEXTUAL_PARAMS, std::exception const & e) {
	__xlog__(ll_info, STD_CONTEXTUAL_ARGS, e.what());
}

void xlog_to_stdout(loglevel_t, STD_CONTEXTUAL_PARAMS,
		std::string const & msg) {
	static mutex stdout_mutex;
	lock_guard<mutex> lock(stdout_mutex);
	printf("%s, %s, line %d: %s\n", source_fname, func, source_line, msg.c_str());
}

bool should_trace(loglevel_t, char const *) {
	return true; // TODO: implement
}

void set_trace_level(loglevel_t, char const *) {
	// TODO: implement
}

void __trace__(loglevel_t level, char const *, STD_CONTEXTUAL_PARAMS,
		string const & msg) {
	static mutex stdout_mutex;
	lock_guard<mutex> lock(stdout_mutex);
	fprintf(stderr, "%d %s, %s, line %d: %s\n", static_cast<int>(level),
			source_fname, func, source_line, msg.c_str());
}

void __trace(loglevel_t level, char const * topics, STD_CONTEXTUAL_PARAMS,
		char const * msg, MANY_ARGS_IMPL) {
	arg const * args[] = {MANY_ARGS_PTR_LIST};
	string txt;
	interp_into(txt, msg, args, 9);
	__trace__(level, topics, STD_CONTEXTUAL_ARGS, txt);
}

void __trace(loglevel_t level, char const * topics, STD_CONTEXTUAL_PARAMS,
		char const * msg, THREE_ARGS) {
	arg const * args[] = {&a1, &a2, &a3};
	string txt;
	interp_into(txt, msg, args, 3);
	__trace__(level, topics, STD_CONTEXTUAL_ARGS, txt);
}

void __trace(loglevel_t level, char const * topics, STD_CONTEXTUAL_PARAMS,
		char const * msg, TWO_ARGS) {
	arg const * args[] = {&a1, &a2};
	string txt;
	interp_into(txt, msg, args, 2);
	__trace__(level, topics, STD_CONTEXTUAL_ARGS, txt);
}

void __trace(loglevel_t level, char const * topics, STD_CONTEXTUAL_PARAMS,
		char const * msg, ONE_ARG) {
	arg const * args[] = {&a1};
	string txt;
	interp_into(txt, msg, args, 1);
	__trace__(level, topics, STD_CONTEXTUAL_ARGS, txt);
}

void __trace(loglevel_t level, char const * topics, STD_CONTEXTUAL_PARAMS,
		char const * msg) {
	__trace__(level, topics, STD_CONTEXTUAL_ARGS, msg);
}
