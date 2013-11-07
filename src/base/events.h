#ifndef _BASE_EVENT_H_
#define _BASE_EVENT_H_

#include <string>
#include <vector>

/**
 *  Integrates codebases with Adaptive's event dictionary. See this wiki
 *  page: https://wiki.adaptivecomputing.com:8443/display/ENG/Event+Dictionary
 */

/** Identifiers for various codebase in our stack. */
enum known_component_t {
	kc_posix = 0,
	kc_base = 1,
	kc_mwm = 0x100,
	kc_mws = 0x200,
	kc_viewpoint = 0x300,
	kc_mam = 0x400,
	kc_torque = 0x500,
	kc_nitro = 0x600
};

/** How attention-worthy/serious is the event? */
enum severity_t {
	/**
	 * Event is notable, but not a problem.
	 */
	sev_info,
	/**
	 * Something's wrong, but we're not sure whether the user will consider it
	 * a failure.
	 */
	sev_warning,
	/**
	 * Failed to accomplish what was expected, from the user's perspective.
	 * However, the app is still usable.
	 */
	sev_error,
	/**
	 * Application-wide problem. Continuing to use the app is impossible or
	 * at least not advised.
	 */
	sev_fatal
};

/** Who can understand and react to an event? */
enum escalation_t {
	/**
	 * An ordinary user can understand and react to the event. For example,
	 * a batch is finished, or a user submitted a batch file with bad syntax.
	 */
	esc_user,
	/**
	 * A privileged user (a limited admin) can understand and react to the
	 * event. Can't think of any examples in MTM at present, but this level is
	 * required for general event dictionary compliance.
	 */
	esc_poweruser,
	/**
	 * An admin with root access on the server is needed to be able to react
	 * to this event. For example, MTM is installed improperly.
	 */
	esc_admin,
	/**
	 * An Adaptive employee (e.g., support, engineering, or PS) is needed to
	 * react to this event. For example, MTM crashed unexpectedly.
	 */
	esc_internal
};

/**
 * A type for the unique identifiers for every event we can describe. We only
 * define the type here, not its possible values.
 */
enum eid_t {
	SUCCESS = 0
};

/**
 * A structure that holds info about known events. We compile one instance of
 * this struct into our binary, for every tuple in event_tuples.h. We can then
 * look up info about events at runtime.
 */
class event_data {
public:
	eid_t id;
	char const * name;
	severity_t sev;
	escalation_t esc;
	int number;
	char const * topic;
	char const * msg;
	char const * comments;
};

/**
 * A class that we can use to look up information about an event.
 */
class events {
public:

	/**
	 * @return the symbolic name for a particular event, if known to this
	 * codebase, or the empty string for external events.
	 */
	char const * get_symbolic_name(int eid) const;

	/**
	 * @return the {@link Severity} for a particular event.
	 */
	static severity_t get_severity(int eid);

	/**
	 * @return a user-friendly label for the severity implied by an event id.
	 *     This function is aware of the fact that all posix events are errors.
	 */
	static const char * get_severity_label(int eid);

	/**
	 * @return the {@link Escalation} for a particular event.
	 */
	static escalation_t get_escalation(int eid);

	/**
	 * @return the 13-bit number that's unique to all events in a given codebase,
	 *     but not unique across all codebases.
	 */
	static int get_nonunique_number(int eid);

	/**
	 * @return the identifier for the codebase that originated an event.
	 * @see KnownComponent
	 */
	static known_component_t get_component(int eid);

	/**
	 * @return the topic for an event, if known to this codebase, or the empty
	 *     string for external events.
	 */
	char const * get_topic(int eid) const;

	/**
	 * @return the interp-style message string associated with a particular event,
	 *     if known to this codebase, or the empty string for external events.
	 *
	 * @note overload below. Splitting these is deliberate (and different from
	 *     most of the other event-handling methods here), because how messages
	 *     are looked up differs.
	 */
	char const * get_msg(eid_t event) const;

	/**
	 * Overload of get_msg() for posix errors. Unlike the other version, this one
	 * must return a std::string because the message we return isn't one compiled
	 * into our binary. Never returns an empty string; if error_code isn't
	 * recognized, a placeholder value is used. Can be called with ordinary
	 * event ids as well, if a string is desired.
	 */
	std::string get_msg(int eid) const;

	/**
	 * @return any descriptive comments about an event, if known to this codebase,
	 *     or the empty string for external events.
	 */
	char const * get_comments(int eid) const;

	/**
	 * Convert an event ID into standard form (0xXXXXXXXX if component != POSIX,
	 * or just a decimal number for POSIX).
	 */
	static std::string get_std_id_repr(int eid);

	/**
	 * Convert an event ID into standard form (0xXXXXXXXX if component != POSIX,
	 * or just a decimal number for POSIX). Fill the specified buffer and null
	 * terminate, as long as it's at least MIN_STD_ID_REPR bytes long.
	 *
	 * @return bytes copied, or 0 if buffer is too short.
	 */
	static size_t get_std_id_repr(int eid, char * buf, size_t buflen);
	static const size_t MIN_STD_ID_REPR = 11;

	/**
	 * How many args should be used with this event?
	 */
	int get_arg_count(int eid) const;

	/**
	 * Look up event data like this:
	 *
	 *     auto my_symname = events::catalog().get_symbolic_name(eid);
	 */
	static events const & catalog();

	/**
	 * Call at startup to make events in another component known to the system,
	 * like this:
	 *
	 *     events::default_catalog().publish_event_data(dat, dat_count);
	 */
	static events & default_catalog();
	bool publish_event_data(event_data const * data, size_t count);

	/** Allow iteration over internal collection. */
	typedef std::vector<event_data const *> items_t;
	items_t const & get_items() const;
	event_data const * find(int id) const;
private:
	items_t items;
};

#endif // sentry
