#ifndef _BASE_EVENT_H_
#define _BASE_EVENT_H_

/**
 *  Integrates Nitro with Adaptive's event dictionary. See the following wiki
 *  page: https://wiki.adaptivecomputing.com:8443/display/ENG/Event+Dictionary
 */

namespace nitro {

/**
 * In java, which has rich enums, we'd make this an enum class. In C++, we can
 * make it either a class with static methods, or a namespace. I went with
 * namespace, since we don't need any instance methods.
 */

/** Identifiers for various codebase in our stack. */
enum KnownComponent {
	kcPOSIX = 0,
	kcMWM = 0x100,
	kcMWS = 0x200,
	kcVIEWPOINT = 0x300,
	kcMAM = 0x400,
	kcTORQUE = 0x500,
	kcMTM = 0x600
};

/** How attention-worthy/serious is the event? */
enum Severity {
	/**
	 * Event is notable, but not a problem.
	 */
	sevINFO,
	/**
	 * Something's wrong, but we're not sure whether the user will consider it
	 * a failure.
	 */
	sevWARNING,
	/**
	 * Failed to accomplish what was expected, from the user's perspective.
	 * However, the app is still usable.
	 */
	sevERROR,
	/**
	 * Application-wide problem. Continuing to use the app is impossible or
	 * at least not advised.
	 */
	sevFATAL
};

/** Who can understand and react to an event? */
enum Escalation {
	/**
	 * An ordinary user can understand and react to the event. For example,
	 * a batch is finished, or a user submitted a batch file with bad syntax.
	 */
	escUSER,
	/**
	 * A privileged user (a limited admin) can understand and react to the
	 * event. Can't think of any examples in MTM at present, but this level is
	 * required for general event dictionary compliance.
	 */
	escPOWERUSER,
	/**
	 * An admin with root access on the server is needed to be able to react
	 * to this event. For example, MTM is installed improperly.
	 */
	escADMIN,
	/**
	 * An Adaptive employee (e.g., support, engineering, or PS) is needed to
	 * react to this event. For example, MTM crashed unexpectedly.
	 */
	escINTERNAL
};

/** Unique identifiers for every event we can describe. */
enum EID {
	#define EVENT(name, severity, escalation, number, topic, msg, comments) \
		name = static_cast<int>(sev##severity) << 28 \
			| 0x600 << 16 \
			| static_cast<int>(esc##escalation) << 14 \
			| number,
	#include "base/event_tuples.h"
};

/**
 * @return the symbolic name for a particular event, if known to this codebase,
 * or the empty string for external events.
 */
char const * get_symbolic_name(int eid);

/**
 * @return the {@link Severity} for a particular event.
 */
Severity get_severity(int eid);

/**
 * @return a user-friendly label for the severity implied by an event id.
 *     This function is aware of the fact that all posix events are errors.
 */
const char * get_severity_label(int eid);

/**
 * @return the {@link Escalation} for a particular event.
 */
Escalation get_escalation(int eid);

/**
 * @return the 13-bit number that's unique to all events in a given codebase,
 *     but not unique across all codebases.
 */
int get_nonunique_number(int eid);

/**
 * @return the identifier for the codebase that originated an event.
 * @see KnownComponent
 */
KnownComponent get_component(int eid);

/**
 * @return the topic for an event, if known to this codebase, or the empty
 *     string for external events.
 */
char const * get_topic(int eid);

/**
 * @return the interp-style message string associated with a particular event,
 *     if known to this codebase, or the empty string for external events.
 */
char const * get_msg(EID event);

/**
 * Overload of get_msg() for posix errors. Unlike the other version, this one
 * must return a std::string because the message we return isn't one compiled
 * into our binary. Never returns an empty string; if error_code isn't
 * recognized, a placeholder value is used. Can be called with ordinary
 * event ids as well, if a string is desired.
 */
std::string get_msg(int eid);

/**
 * @return any descriptive comments about an event, if known to this codebase,
 *     or the empty string for external events.
 */
char const * get_comments(int eid);

/**
 * Allow iteration across all events, even though numbers may not be
 * contiguous, by iterating from i to count.
 */
EID get_item_id(size_t i);

/**
 * How many events are defined, so we can iterate across them?
 */
size_t get_item_count();

/**
 * Convert an event ID into standard form (0xXXXXXXXX if component != POSIX,
 * or just a decimal number for POSIX).
 */
std::string get_std_id_repr(int eid);

/**
 * Convert an event ID into standard form (0xXXXXXXXX if component != POSIX,
 * or just a decimal number for POSIX). Fill the specified buffer and null
 * terminate, as long as it's at least MIN_STD_ID_REPR bytes long.
 *
 * @return bytes copied, or 0 if buffer is too short.
 */
size_t get_std_id_repr(int eid, char * buf, size_t buflen);
const size_t MIN_STD_ID_REPR = 11;

/**
 * How many args should be used with this event?
 */
int get_arg_count(int eid);

} // end namespace nitro

#endif /* NITRO_BASE_EVENT_H_ */
