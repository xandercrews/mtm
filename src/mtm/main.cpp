#include <stdio.h>

#include "common/cmdline.h"

using namespace mtm;

int main(int argc, char ** argv) {
	Cmdline cmdline(argc, (char const **)argv);
	if (cmdline.help_needed()) {
		printf("%s", cmdline.get_help().c_str());
		return 1;
	}
    return 0;
}

