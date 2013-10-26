/* DO NOT ADD A SENTRY TO THIS FILE. OMITTED DELIBERATELY. */


// EVENT(name, severity, escalation, number, topic, msg, comments)
//
// Field descriptions/examples/valid values:
//
// name: [unique identifier]
// severity: INFO, WARNING, ERROR, FATAL
// escalation: USER, POWERUSER, ADMIN, INTERNAL
// number: [unique number for MTM--13 bits max]
// msg: [associated message that is shown, with interp-style substitution]
// comments: [description used for documentation/explanation purposes]

EVENT(MTM_FUNC_NOT_IMPLEMENTED, sevERROR, escINTERNAL, 1,
		"domain.mtm.general",
		"Function has not yet been implemented.",
		"This error is used when we've stubbed out code but do not expect it to"
		" be called in production environments. It's not helpful except for"
		" internal diagnostics.")

EVENT(MTM_1FILE_BAD_HUGE_LINE_2BYTES, sevERROR, escUSER, 2,
		"domain.mtm.input",
		"Input file %1{fname} is malformed because it contains a single line of"
		" text that's at least %2{bytes_read} bytes long. The batch must be"
		" abandoned.",
		"")

EVENT(MTM_BAD_FNAME_NULL_OR_EMPTY, sevERROR, escINTERNAL, 3,
		"domain.mtm.input",
		"Batch cannot be built with null or empty filename.",
		"")

EVENT(MTM_1FILE_BAD_SEEMS_BINARY, sevERROR, escUSER, 4,
		"domain.mtm.input",
		"Input file %1{fname} seems to be binary. The batch must be abandoned.",
		"")

#undef EVENT
