#ifndef _NITRO_DOMAIN_EVENT_H_
#define _NITRO_DOMAIN_EVENT_H_

#include "base/events.h"

/**
 * Define a bunch of event IDs for nitro.
 */
namespace nitro {
namespace event_codes {

#define EVENT(name, severity, escalation, number, topic, msg, comments) \
	const ecode_t name = static_cast<ecode_t>( number \
		| static_cast<int>(sev_##severity) << 28 \
		| static_cast<int>(kc_nitro) << 16 \
		| static_cast<int>(esc_##escalation) << 14);
#include "domain/event_tuples.h"

// Force these events to be registered and linked into any binary that
// #includes this header.
bool publish();
static bool published = publish();

} // end namespace event_codes
} // end namespace nitro

#endif // sentry
