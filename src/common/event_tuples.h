/* DO NOT ADD A SENTRY TO THIS FILE. OMITTED DELIBERATELY. */


// EVENT(name, severity, alation, number, topic, msg, comments)
//
// Field descriptions/examples/valid values:
//
// name: [unique identifier]
// severity: INFO, WARNING, ERROR, FATAL
// escalation: USER, POWERUSER, ADMIN, INTERNAL
// number: [unique number for MTM--13 bits max]
// msg: [associated message that is shown, with interp-style substitution]
// comments: [description used for documentation/explanation purposes]

EVENT(NITRO_FUNC_NOT_IMPLEMENTED, ERROR, INTERNAL, 1,
		"domain.nitro.internal",
		"Function has not yet been implemented.",
		"This error is used when we've stubbed out code but do not expect it to"
		" be called in production environments. It's not helpful except for"
		" internal diagnostics.")

EVENT(NITRO_1FILE_BAD_HUGE_LINE_2BYTES, ERROR, USER, 2,
		"domain.nitro.input",
		"Input file %1{fname} is malformed because it contains a single line of"
		" text that's at least %2{bytes_read} bytes long. The batch must be"
		" abandoned.",
		"")

EVENT(NITRO_BAD_FNAME_NULL_OR_EMPTY, ERROR, INTERNAL, 3,
		"domain.nitro.input",
		"Batch cannot accept null or empty filename.",
		"")

EVENT(NITRO_1FILE_BAD_SEEMS_BINARY, ERROR, USER, 4,
		"domain.nitro.input",
		"Input file %1{fname} seems to be binary. The batch must be abandoned.",
		"")

EVENT(NITRO_1FILE_UNREADABLE, ERROR, USER, 5,
		"domain.nitro.input",
		"Input file %1{fname} is unavailable or unreadable. Check permissions"
		" and file name.",
		"")

EVENT(NITRO_1FILE_EMPTY, ERROR, USER, 6,
		"domain.nitro.input",
		"Input file %1{fname} is empty.",
		"")

EVENT(NITRO_FAILED_TO_LOCK_MUTEX_1POSIX_ERROR, FATAL, INTERNAL, 7,
		"domain.nitro.internal",
		"Failed to lock mutex; posix error = %1{errno}. The application must"
		" be terminated to avoid undefined and risky behavior. Restarting"
		" may or may not help.",
		"This is a fatal error because all multithreading in the application"
		" is suspect if we cannot lock a mutex.")

EVENT(NITRO_TIMED_OUT_AFTER_1MILLIS_LOCKING_MUTEX, ERROR, INTERNAL, 8,
		"domain.nitro.internal",
		"Timed out after waiting to acquire a mutex for %1 milliseconds."
		" This may indicate an overly busy system, or a deadlock caused by a"
		" coding error.",
		"")

EVENT(NITRO_FAILED_TO_UNLOCK_MUTEX_1POSIX_ERROR, FATAL, INTERNAL, 9,
		"domain.nitro.internal",
		"Failed to unlock mutex; posix error = %1{errno}. The application must"
		" be terminated to avoid undefined and risky behavior. Restarting"
		" may or may not help.",
		"This is a fatal error because all multithreading in the application"
		" is suspect if we cannot unlock a mutex.")

EVENT(NITRO_PRECONDITION_1EXPR_VIOLATED, FATAL, INTERNAL, 10,
		"domain.nitro.internal",
		"A function was called in a way that violated its requirements."
		" This makes the entire application's behavior undefined; it is a"
		" serious bug that needs to be reported immediately. The specific"
		" precondition that failed was: %1{expression}.",
		"When preconditions are violated, it means a function in the call"
		" graph is not behaving correctly. An orderly and rapid exit is the"
		" best way to protect the user from subsequent problems.")

EVENT(NITRO_CHECK_1EXPR_VIOLATED, FATAL, INTERNAL, 11,
		"domain.nitro.internal",
		"A function did not behave as expected."
		" This makes the entire application's behavior undefined; it is a"
		" serious bug that needs to be reported immediately. The specific"
		" check that failed was: %1{expression}.",
		"When checks are violated, it means a function in the call"
		" graph is not behaving correctly. An orderly and rapid exit is the"
		" best way to protect the user from subsequent problems.")

EVENT(NITRO_POSTCONDITION_1EXPR_VIOLATED, FATAL, INTERNAL, 12,
		"domain.nitro.internal",
		"A function did not exit in a way that satisfied its contract."
		" This makes the entire application's behavior undefined; it is a"
		" serious bug that needs to be reported immediately. The specific"
		" postcondition that failed was: %1{expression}.",
		"When preconditions are violated, it means a function in the call"
		" graph is not behaving correctly. An orderly and rapid exit is the"
		" best way to protect the user from subsequent problems.")

#undef EVENT
