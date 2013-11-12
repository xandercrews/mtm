// We have to tweak defines to force the XIS-compatible version of strerror_r.
// See http://linux.die.net/man/3/strerror.
#define _POSIX_C_SOURCE 201311L
#undef _GNU_SOURCE

#include <algorithm>
#include <string.h>

#include "base/events.h"

/** Expose write access to event catalog for modules that need to register. */
events & events::default_catalog() {
	static events the_default_instance;
	return the_default_instance;
}

events const & events::catalog() {
	return default_catalog();
}

static bool compare_events_by_id(event_data const * a, event_data const * b) {
	return a->code < b->code;
}

bool events::publish_event_data(event_data const * data, size_t count) {
	// Pre-allocate space in the vector, for efficiency.
	if (items.empty()) {
		items.reserve(256);
	} else {
		items.reserve(items.size() + count);
	}
	for (size_t i = 0; i < count; ++i) {
		items.push_back(data);
		++data;
	}
	std::sort(items.begin(), items.end(), compare_events_by_id);
	return true;
}

static bool compare_event_to_id(event_data const * a, int ecode) {
	return a->code < ecode;
}

event_data const * events::find(int ecode) const {
	auto e = std::lower_bound(items.begin(), items.end(), ecode,
		compare_event_to_id);
	return e == items.end() || (*e)->code > ecode ? 0 : *e;
}

char const * events::get_symbolic_name(int ecode) const {
	auto e = find(ecode);
	return e ? e->name : "";
}

severity_t events::get_severity(int ecode) {
	if (get_component(ecode) == kc_posix) {
		return ecode ? sev_error : sev_info;
	}
	return static_cast<severity_t>(ecode >> 28 & 0x3);
}

escalation_t events::get_escalation(int ecode) {
	if (get_component(ecode) == kc_posix) {
		return ecode? esc_user : esc_admin;
	}
	return static_cast<escalation_t>(ecode >> 14 & 0x3);
}

int events::get_nonunique_number(int ecode) {
	return ecode & 0x3FFF;
}

known_component_t events::get_component(int ecode) {
	return static_cast<known_component_t>(ecode >> 16 & 0x0FFF);
}

char const * events::get_topic(int ecode) const {
	auto e = find(ecode);
	return e ? e->topic : "";
}

std::string events::get_msg(int ecode) const {
	if (get_component(ecode) == kc_posix) {
		std::string msg;
		char buf[512];
		buf[0] = 0;
		if (strerror_r(ecode, buf, sizeof(buf)) == 0
				&& buf[0]) {
			msg += buf;
			// Posix errors tend not to be punctuated as sentences...
			if (msg.size() > 0 && msg[msg.size() - 1] != '.') {
				msg += '.';
			}
		} else {
			msg += "(message unavailable).";
		}
		return msg;
	} else {
		return get_msg(static_cast<ecode_t>(ecode));
	}
}

char const * events::get_msg(ecode_t ecode) const {
	auto e = find(ecode);
	return e ? e->msg : "";
}

char const * events::get_comments(int ecode) const {
	auto e = find(ecode);
	return e ? e->comments : "";
}

std::string events::get_std_id_repr(int ecode) {
	char buf[MIN_STD_ID_REPR];
	get_std_id_repr(ecode, buf, sizeof(buf));
	return buf;
}

size_t events::get_std_id_repr(int ecode, char * buf, size_t buflen) {
	if (buflen >= MIN_STD_ID_REPR) {
		auto format = (get_component(ecode) == 0) ? "%d" : "0x%08X";
		return sprintf(buf, format, ecode);
	}
	return 0;
}

events::items_t const & events::get_items() const {
	return items;
}

const char * const SeverityLabels[4] = {
	"Info",
	"Warning",
	"Error",
	"Fatal error"
};

char const * events::get_severity_label(int ecode) {
	if (get_component(ecode) == kc_posix) {
		return ecode ? "Posix error" : "Posix info";
	}
	return SeverityLabels[get_severity(ecode)];
}

int events::get_arg_count(int ecode) const {
	// Since it's possible to have a string that references the same arg more
	// than once, we can't derive arg_count from counting the number of refs in
	// the message. Instead, find the highest arg number that appears in name.
	auto arg_count = 0;
	auto symbolic_name = get_symbolic_name(ecode);
	if (symbolic_name) {
		auto prev_was_underscore = false;
		for (auto p = symbolic_name; *p; ++p) {
			auto c = *p;
			if (isdigit(c)) {
				if (prev_was_underscore) {
					if (p[1] && !isdigit(p[1])) {
						auto n = c - '0';
						if (n > arg_count) {
							arg_count = n;
						}
					}
				}
			}
			prev_was_underscore = (c == '_');
		}
	}
	return arg_count;
}

