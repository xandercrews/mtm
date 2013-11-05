#include "domain/event_ids.h"

namespace nitro {
namespace event_ids {

event_data const ITEMS[] = {
	#define EVENT(name, severity, escal, num, topic, msg, comments) \
	{ name, #name, sev_##severity, esc_##escal, num, topic, msg, comments },
	#include "domain/event_tuples.h"
};

static const size_t ITEM_COUNT = sizeof(ITEMS) / sizeof(event_data);

bool publish() {
	static bool published = false;
	if (!published) {
		published = events::default_catalog().publish_event_data(ITEMS, ITEM_COUNT);
	}
	return published;
}

} // end namespace event_ids
} // end namespace nitro

