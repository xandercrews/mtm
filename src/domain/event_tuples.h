/* DO NOT ADD A SENTRY TO THIS FILE. OMITTED DELIBERATELY. */


// EVENT(name, severity, escalation, number, topic, msg, comments)
//
// Field descriptions/examples/valid values:
//
// name: [unique identifier]
// severity: info, WARNING, error, fatal
// escalation: user, POWERUSER, ADMIN, internal
// number: [unique number for MTM--13 bits max]
// msg: [associated message that is shown, with interp-style substitution]
// comments: [description used for documentation/explanation purposes]

EVENT(NITRO_EXPECTED_PORT_NUM_AFTER_1SWITCH_NOT_2VAL, error, user, 1,
		"domain.nitro.input",
		"Expected numeric port value > 1024 and < 65536 after %1{port_switch},"
		" not \"%2{value}\".",
		"")

EVENT(NITRO_EXPECTED_UNIQUE_1SWITCH_BUT_2NUM_REUSED, error, user, 2,
		"domain.nitro.input",
		"Expected %1{port_switch} to get a unique value, but port %2{value} is"
		" re-used.",
		"")

EVENT(NITRO_BAD_WALLTIME_1VALUE, error, user, 3,
		"domain.nitro.input",
		"Invalid walltime value \"%1\". Expected raw seconds, m:s, h:m:s, or"
		" d:h:m:s.",
		"")

EVENT(NITRO_MSG_TOO_BIG_1SIZE, error, admin, 4,
		"domain.nitro.input",
		"Receive a message too big to accept (size >= %1). Communication"
		" discarded.",
		"")

EVENT(NITRO_HERE_IS_ASSIGNMENT, info, internal, 100,
		"domain.nitro.coordinate",
		"Here is an assignment of a few new commands to run.",
		"")

EVENT(NITRO_REQUEST_HELP, info, internal, 101,
		"domain.nitro.coordinate",
		"You've been asked to help me. Will you?",
		"")

EVENT(NITRO_AFFIRM_HELP, info, internal, 102,
		"domain.nitro.coordinate",
		"Sure, I'll help.",
		"")

EVENT(NITRO_DENY_HELP_1REASON, info, internal, 103,
		"domain.nitro.coordinate",
		"No can do. Sorry. %1",
		"")

EVENT(NITRO_NEED_ASSIGNMENT, info, internal, 104,
		"domain.nitro.coordinate",
		"I am running out of work to do. Give me another assignment.",
		"")

EVENT(NITRO_ASSIGNMENT_PROGRESS_REPORT, info, internal, 105,
		"domain.nitro.status",
		"Here is information about how much I've done on my assignment.",
		"")

EVENT(NITRO_1ASSIGNMENT_COMPLETE, info, internal, 106,
		"domain.nitro.status",
		"Assignment \"%1\" is complete.",
		"")

EVENT(NITRO_BATCH_PROGRESS_REPORT, info, internal, 107,
		"domain.nitro.status",
		"Here is information about how much I've done on my batch.",
		"")

EVENT(NITRO_BATCH_SUBMITTED, info, internal, 108,
		"domain.nitro.coordinate",
		"Here is a new batch to work on.",
		"")

EVENT(NITRO_TERMINATE_REQUEST, info, internal, 109,
		"domain.nitro.internal",
		"Please stop all work and exit.",
		"")

EVENT(NITRO_PING_REQUEST, info, internal, 110,
		"domain.nitro.coordinate",
		"Please respond if you're alive.",
		"")

EVENT(NITRO_PING_RESPONSE, info, internal, 111,
		"domain.nitro.coordinate",
		"I am alive, and I've responded as you asked.",
		"")

EVENT(NITRO_ACCEPT_ASSIGNMENT, info, internal, 112,
		"domain.nitro.coordinate",
		"Assignment accepted.",
		"")

EVENT(NITRO_REJECT_ASSIGNMENT_1REASON, info, internal, 113,
		"domain.nitro.coordinate",
		"Assignment rejected. Reason: %1.",
		"")

#undef EVENT
