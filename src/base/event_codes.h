#ifndef _BASE_EVENT_IDS_H_
#define _BASE_EVENT_IDS_H_

#include "base/events.h"

/**
 * Define a bunch of event IDs for base.
 */
namespace base {
namespace event_codes {

#define EVENT(name, severity, escalation, number, topic, msg, comments) \
	const ecode_t name = static_cast<ecode_t>( number \
		| static_cast<int>(sev_##severity) << 28 \
		| static_cast<int>(kc_base) << 16 \
		| static_cast<int>(esc_##escalation) << 14);
#include "base/event_tuples.h"

// Force these events to be registered and linked into any binary that
// #includes this header.
bool publish();
static bool published = publish();

} // end namespace event_codes
} // end namespace base

#endif // sentry
