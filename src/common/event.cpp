// We have to tweak defines to force the XIS-compatible version of strerror_r.
// See http://linux.die.net/man/3/strerror.
#define _POSIX_C_SOURCE 201311L
#undef _GNU_SOURCE

#include <map>
#include <string.h>

#include "common/event.h"

namespace mtm {
namespace event {

struct EventData {
	EID id;
	char const * name;
	Severity severity;
	Escalation escalation;
	int number;
	char const * topic;
	char const * msg;
	char const * comments;
};

EventData const ITEMS[] = {
	#define EVENT(name, severity, escalation, number, topic, msg, comments) \
	{ name, #name, severity, escalation, number, topic, msg, comments },
	#include "common/event_tuples.h"
};

static const size_t ITEM_COUNT = sizeof(ITEMS) / sizeof(EventData);

static EventData const * get_item_by_id(int eid) {
	typedef std::map<int, size_t> DataMap;
	static DataMap the_map;
	static bool inited = false;
	if (!inited) {
		inited = true;
		size_t i = 0;
		#define EVENT(name, severity, escalation, number, topic, msg, comments) \
			the_map[name] = i++;
		#include "common/event_tuples.h"
	}

	try {
		// Make sure we always access the map read-only, once init-ed.
		static const DataMap & the_readonly_map = the_map;
		size_t index = the_readonly_map.at(eid);
		EventData const & ed = ITEMS[index];
		return &ed;
	} catch (std::out_of_range const &) {
		return NULL;
	}
}

char const * get_symbolic_name(int eid) {
	EventData const * e = get_item_by_id(eid);
	return e ? e->name : "";
}

Severity get_severity(int eid) {
	if (get_component(eid) == kcPOSIX) {
		return sevERROR;
	}
	return static_cast<Severity>(eid >> 28 & 0x3);
}

Escalation get_escalation(int eid) {
	if (get_component(eid) == kcPOSIX) {
		return escADMIN;
	}
	return static_cast<Escalation>(eid >> 14 & 0x3);
}

int get_nonunique_number(int eid) {
	return eid & 0x3FFF;
}

KnownComponent get_component(int eid) {
	return static_cast<KnownComponent>(eid >> 16 & 0x0FFF);
}

char const * get_topic(int eid) {
	EventData const * e = get_item_by_id(eid);
	return e ? e->topic : "";
}

std::string get_msg(int eid) {
	if (get_component(eid) == kcPOSIX) {
		std::string msg;
		char buf[512];
		buf[0] = 0;
		if (strerror_r(eid, buf, sizeof(buf)) == 0
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
		return get_msg(static_cast<EID>(eid));
	}
}

char const * get_msg(EID eid) {
	EventData const * e = get_item_by_id(eid);
	return e ? e->msg : "";
}

char const * get_comments(int eid) {
	EventData const * e = get_item_by_id(eid);
	return e ? e->comments : "";
}

EID get_item_id(size_t i) {
	return (i < ITEM_COUNT) ? ITEMS[i].id : static_cast<EID>(0);
}

size_t get_item_count() {
	return ITEM_COUNT;
}

std::string get_std_id_repr(int eid) {
	char buf[MIN_STD_ID_REPR];
	get_std_id_repr(eid, buf, sizeof(buf));
	return buf;
}

size_t get_std_id_repr(int eid, char * buf, size_t buflen) {
	if (buflen >= MIN_STD_ID_REPR) {
		auto format = (get_component(eid) == 0) ? "%d" : "0x%08X";
		return sprintf(buf, format, eid);
	}
	return 0;
}

const char * const SeverityLabels[4] = {
	"Info",
	"Warning",
	"Error",
	"Fatal"
};

char const * get_severity_label(int eid) {
	if (get_component(eid) == kcPOSIX) {
		return "Posix error";
	}
	return SeverityLabels[get_severity(eid)];
}

} // end event namespace
} // end mtm namespace

