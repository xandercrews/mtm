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

EVENT(NITRO_HERE_IS_ASSIGNMENT, info, internal, 100,
		"domain.nitro.internal",
		"Here is an assignment of a few new commands to run.",
		"")

EVENT(NITRO_NEED_ASSIGNMENT, info, internal, 101,
		"domain.nitro.internal",
		"I am running out of work to do. Give me another assignment.",
		"")

EVENT(NITRO_ASSIGNMENT_PROGRESS_REPORT, info, internal, 102,
		"domain.nitro.internal",
		"Here is information about how much I've done on my assignment.",
		"")

EVENT(NITRO_BATCH_PROGRESS_REPORT, info, internal, 103,
		"domain.nitro.internal",
		"Here is information about how much I've done on my batch.",
		"")

EVENT(NITRO_BATCH_SUBMITTED, info, internal, 104,
		"domain.nitro.internal",
		"Here is a new batch to work on.",
		"")

EVENT(NITRO_TERMINATE_REQUEST, info, internal, 105,
		"domain.nitro.internal",
		"Please stop all work and exit.",
		"")

EVENT(NITRO_PING_REQUEST, info, internal, 106,
		"domain.nitro.internal",
		"Please respond if you're alive.",
		"")

EVENT(NITRO_PING_RESPONSE, info, internal, 107,
		"domain.nitro.internal",
		"I am alive, and I've responded as you asked.",
		"")

#undef EVENT
