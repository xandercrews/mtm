#include <stdio.h>

#include "common/cmdline.h"
#include "common/engine.h"

using namespace mtm;

int main(int argc, char ** argv) {

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
}

