#include <stdio.h>

#include "common/cmdline.h"
#include "common/engine.h"

using namespace nitro;

int main(int argc, char ** argv) {

	// Unlike Adaptive's other C/C++ code, this codebase is a green field,
	// giving us an opportunity to use many cutting-edge constructs from C++ 93
	// and C++11. Part of that modernization is that we use exceptions to
	// communicate errors in many cases, which is why the body of our main()
	// function is enclosed in try...catch. Some subroutines called by main()
	// do their own exception handling--but in case they don't, we trap here,
	// write a coherent message, and make sure we return 1.
	//
	// Our general rule of thumb: If a function can fail to do its job without
	// surprising us particularly, we return normal output for success, and
	// null/false for failure. (Note the difference from C exit codes, where 1
	// would be an error and 0 a success.) But if failure is a truly
	// exceptional case, we throw exceptions.
	//
	// An example of a function that should return a scalar is a string-
	// scanning routine that looks for a token that may or may not be present.
	// An example of a function that should throw an exception is one that
	// writes to disk, only to discover that the disk is full.

	try {

		Cmdline cmdline(argc, (char const **)argv);
		if (cmdline.help_needed()) {
			printf("%s", cmdline.get_help().c_str());
			return 1;
		}

		Engine engine;

		char const * port = cmdline.get_option("--port");
		if (port) {
			engine.set_port(atoi(port));
		}

		if (cmdline.has_flag("--slave")) {
			engine.set_slave_mode(true);
		}

		return engine.run();

	} catch (std::exception const & e) {
		printf("Fatal error: %s\n", e.what());
		return 1;
	}
}

