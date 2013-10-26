/**
 *  Integrates MTM with Adaptive's event dictionary. See the following wiki
 *  page: https://wiki.adaptivecomputing.com:8443/display/ENG/Event+Dictionary
 */

#ifndef MTM_COMMON_EVENT_H_
#define MTM_COMMON_EVENT_H_

namespace mtm {

/**
 * In java, which has rich enums, we'd make this an enum class. In C++, we can
 * make it either a class with static methods, or a namespace. I went with
 * namespace, since we don't need any instance methods.
 */
namespace event {

/** How attention-worthy/serious is the event? */
enum Severity {
	/**
	 * Event is notable, but not a problem.
	 */
	INFO,
	/**
	 * Something's wrong, but we're not sure whether the user will consider it
	 * a failure.
	 */
	WARNING,
	/**
	 * Failed to accomplish what was expected, from the user's perspective.
	 * However, the app is still usable.
	 */
	ERROR,
	/**
	 * Application-wide problem. Continuing to use the app is impossible or
	 * at least not advised.
	 */
	FATAL
};

/** Provide standard labels for the 4 severities. */
extern const char * const SeverityLabels[4];

/** Who can understand and react to an event? */
enum Escalation {
	/**
	 * An ordinary user can understand and react to the event. For example,
	 * a batch is finished, or a user submitted a batch file with bad syntax.
	 */
	USER,
	/**
	 * A privileged user (a limited admin) can understand and react to the
	 * event. Can't think of any examples in MTM at present, but this level is
	 * required for general event dictionary compliance.
	 */
	POWERUSER,
	/**
	 * An admin with root access on the server is needed to be able to react
	 * to this event. For example, MTM is installed improperly.
	 */
	ADMIN,
	/**
	 * An Adaptive employee (e.g., support, engineering, or PS) is needed to
	 * react to this event. For example, MTM crashed unexpectedly.
	 */
	INTERNAL
};

/** This value identifies MTM to other subsystems in a stack. */
const int MTM_COMPONENT_ID = 0x600;

/** Unique identifiers for every event we can describe. */
enum EID {
	#define EVENT(name, severity, escalation, number, topic, msg, comments) \
		name = static_cast<int>(severity) << 28 \
			| MTM_COMPONENT_ID << 16 \
			| static_cast<int>(escalation) << 14 \
			| number,
	#include "common/event_tuples.h"
};

/** @return the symbolic name for a particular EventID */
char const * get_name(EID event);

/** @return the severity for a particular EID */
Severity get_severity(EID event);

/** @return the escalation for a particular EID */
Escalation get_escalation(EID event);

/** @return the 13-bit number that's unique across all MTM's events. */
int get_number(EID event);

/** @return the topic for this MTM event. */
char const * get_topic(EID event);

/** @return the interp-style string associated with an MTM event. */
char const * get_msg(EID event);

/** @return any descriptive comments about an event, for doc purposes. */
char const * get_comments(EID event);

/**
 * Allow iteration across all events, even though numbers may not be
 * contiguous, by iterating from i to count.
 */
EID get_item_id(size_t i);

/**
 * How many events are defined, so we can iterate across them?
 */
size_t get_item_count();

} // end event namespace
} // end namespace mtm

#endif /* MTM_COMMON_EVENT_H_ */
