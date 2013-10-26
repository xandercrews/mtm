#include <map>

#include "common/event.h"

namespace mtm {
namespace event {

const char * const SeverityLabels[4] = {
	"Info",
	"Warning",
	"Error",
	"Fatal"
};

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

static EventData const * get_item_by_id(EID id) {
	typedef std::map<EID, size_t> DataMap;
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
		size_t index = the_readonly_map.at(id);
		EventData const & ed = ITEMS[index];
		return &ed;
	} catch (std::out_of_range const &) {
		return NULL;
	}
}

char const * get_name(EID event) {
	EventData const * e = get_item_by_id(event);
	return e ? e->name : "";
}

Severity get_severity(EID event) {
	EventData const * e = get_item_by_id(event);
	return e ? e->severity : static_cast<Severity>(0);
}

Escalation get_escalation(EID event) {
	EventData const * e = get_item_by_id(event);
	return e ? e->escalation : static_cast<Escalation>(0);
}

int get_number(EID event) {
	EventData const * e = get_item_by_id(event);
	return e ? e->number : 0;
}

char const * get_topic(EID event) {
	EventData const * e = get_item_by_id(event);
	return e ? e->topic : "";
}

char const * get_msg(EID event) {
	EventData const * e = get_item_by_id(event);
	return e ? e->msg : "";
}

char const * get_comments(EID event) {
	EventData const * e = get_item_by_id(event);
	return e ? e->comments : "";
}

EID get_item_id(size_t i) {
	return (i < ITEM_COUNT) ? ITEMS[i].id : static_cast<EID>(0);
}

size_t get_item_count() {
	return ITEM_COUNT;
}

} // end event namespace
} // end mtm namespace

