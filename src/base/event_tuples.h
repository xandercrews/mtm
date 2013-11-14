/* DO NOT ADD A SENTRY TO THIS FILE. OMITTED DELIBERATELY. */


// EVENT(name, severity, escalation, number, topic, msg, comments)
//
// Field descriptions/examples/valid values:
//
// name: [unique identifier]
// severity: info, warning, error, fatal
// escalation: user, poweruser, admin, internal
// number: [unique number for MTM--13 bits max]
// msg: [associated message that is shown, with interp-style substitution]
// comments: [description used for documentation/explanation purposes]

EVENT(E_NOT_IMPLEMENTED, error, internal, 1,
		"domain.base.internal",
		"Functionality has not yet been implemented.",
		"This error is used when we've stubbed out code but do not expect it to"
		" be called in production environments. It's not helpful except for"
		" internal diagnostics.")

EVENT(E_INPUT_FILE_1PATH_UNREADABLE, error, user, 2,
		"domain.base.input",
		"Input file %1{fname} is unavailable or unreadable. Check permissions"
		" and file name.",
		"")

EVENT(E_INPUT_FILE_1PATH_EMPTY, error, user, 3,
		"domain.base.input",
		"Input file %1{fname} is empty.",
		"")

EVENT(E_BAD_FNAME_NULL_OR_EMPTY, error, internal, 4,
		"domain.nitro.input",
		"Cannot accept null or empty filename.",
		"")

EVENT(E_FAILED_TO_LOCK_MUTEX_1POSIX_ERROR, fatal, internal, 5,
		"domain.base.internal",
		"Failed to lock mutex; posix error = %1{errno}. The application must"
		" be terminated to avoid undefined and risky behavior. Restarting"
		" may or may not help.",
		"This is a fatal error because all multithreading in the application"
		" is suspect if we cannot lock a mutex.")

EVENT(E_TIMED_OUT_AFTER_1MILLIS_LOCKING_MUTEX, error, internal, 6,
		"domain.base.internal",
		"Failed to lock mutex within %1 milliseconds."
		" This may indicate an overly busy system, or a deadlock caused by a"
		" coding error.",
		"")

EVENT(E_FAILED_TO_UNLOCK_MUTEX_1POSIX_ERROR, fatal, internal, 7,
		"domain.base.internal",
		"Failed to unlock mutex; posix error = %1{errno}. The application must"
		" be terminated to avoid undefined and risky behavior. Restarting"
		" may or may not help.",
		"This is a fatal error because all multithreading in the application"
		" is suspect if we cannot unlock a mutex.")

EVENT(E_DEADLOCK, fatal, internal, 8,
		"domain.base.internal",
		"A deadlock was detected. This is a serious bug that needs to be"
		" immediately. The application must be terminated to avoid undefined"
		" and risky behavior. Restarting may or may not help.",
		"Deadlocks are an indication that a coder did not acquire and release"
		" locks in a consistent order, or used recursion inappropriately.")

EVENT(E_PRECONDITION_1EXPR_VIOLATED, fatal, internal, 9,
		"domain.base.internal",
		"A function was called in a way that violated its requirements."
		" This makes the entire application's behavior undefined; it is a"
		" serious bug that needs to be reported immediately. The specific"
		" precondition that failed was: %1{expression}.",
		"When preconditions are violated, it means a function in the call"
		" graph is not behaving correctly. An orderly and rapid exit is the"
		" best way to protect the user from subsequent problems.")

EVENT(E_CHECK_1EXPR_VIOLATED, fatal, internal, 10,
		"domain.base.internal",
		"A function did not behave as expected."
		" This makes the entire application's behavior undefined; it is a"
		" serious bug that needs to be reported immediately. The specific"
		" check that failed was: %1{expression}.",
		"When checks are violated, it means a function in the call"
		" graph is not behaving correctly. An orderly and rapid exit is the"
		" best way to protect the user from subsequent problems.")

EVENT(E_POSTCONDITION_1EXPR_VIOLATED, fatal, internal, 11,
		"domain.base.internal",
		"A function did not exit in a way that satisfied its contract."
		" This makes the entire application's behavior undefined; it is a"
		" serious bug that needs to be reported immediately. The specific"
		" postcondition that failed was: %1{expression}.",
		"When postconditions are violated, it means a function in the call"
		" graph is not behaving correctly. An orderly and rapid exit is the"
		" best way to protect the user from subsequent problems.")

EVENT(E_ILLEGAL_CALL_IN_STATE_1NAME, error, internal, 12,
		"domain.base.internal",
		"Function was called while the state of an object or of the system"
		" (%1{state descrip}) makes the call illegal. This is a coding error"
		" that should be reported; its seriousness is unknown.",
		"")

EVENT(E_CMDLILNE_MISSING_VALUE_FOR_1OPTION, error, user, 13,
		"domain.base.input",
		"Expected %1 option to be followed by a value.",
		"")

EVENT(E_CMDLINE_UNRECOGNIZED_1SWITCH, error, user, 14,
		"domain.base.input",
		"%1 looks like a switch of some kind, but is not recognized.",
		"")

EVENT(E_1FILE_BAD_HUGE_LINE_2BYTES, error, user, 15,
		"domain.base.input",
		"Input file %1{fname} is malformed because it contains a single line of"
		" text that's at least %2{bytes_read} bytes long. It cannot be"
		" processed line-by-line.",
		"")

EVENT(E_1FILE_BAD_SEEMS_BINARY, error, user, 16,
		"domain.base.input",
		"Input file %1{fname} seems to be binary; it cannot be processed"
		" line-by-line.",
		"")

EVENT(I_1APP_STARTING, info, user, 17,
		"domain.base.housekeeping",
		"Input file %1{fname} seems to be binary; it cannot be processed"
		" line-by-line.",
		"")

#undef EVENT
