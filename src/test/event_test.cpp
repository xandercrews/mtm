#include <map>

#include "gtest/gtest.h"
#include "common/event.h"
#include "common/interp.h"

using namespace std;
using namespace mtm;

TEST(EventTest, no_dups) {
	typedef std::map<int, char const *> Map;
	Map names_by_number;
	size_t total = event::get_item_count();
	for (size_t i = 0; i < total; ++i) {
		event::EID id = event::get_item_id(i);
		auto number = event::get_number(id);
		auto name = event::get_name(id);
		if (names_by_number.find(number) == names_by_number.end()) {
			names_by_number[number] = name;
		} else {
			ADD_FAILURE() << "Duplicate number " << number << " for " << name
					<< " and " << names_by_number[number] << ".";
		}
	}
}

TEST(EventTest, valid_names_and_argrefs) {
	size_t total = event::get_item_count();
	for (size_t i = 0; i < total; ++i) {

		event::EID id = event::get_item_id(i);
		auto name = event::get_name(id);
		for (auto p = name; *p; ++p) {
			if (toupper(*p) != *p) {
				ADD_FAILURE() << name << " is not upper-case.";
				break;
			}
		}
		if (strncmp(name, "MTM_", 4) != 0) {
			ADD_FAILURE() << name << " does not begin with \"MTM_\".";
		}

		// Scan the message text. Notice which args have been used; verify
		// that all argrefs are formatted correctly. Check capitalization and
		// punctuation.
		bool used_argrefs[] = {false, false, false, false, false, false, false,
				false, false};
		auto msg = event::get_msg(id);
		string prefix = interp("Message for %1 (\"%2\") ", name, msg);
		if (isalpha(*msg) && toupper(*msg) != *msg) {
			ADD_FAILURE() << prefix << "doesn't begin with a capital letter.";
		}
		if (strlen(msg) < 10) {
			ADD_FAILURE() << prefix << "is too short to be useful.";
		} else {
			auto end = strchr(msg, 0) - 1;
			if (*end != '.') {
				if (!(*end >= '1' && *end <= '9' && end[-1] == '%')) {
					if (*end != '}') {
						ADD_FAILURE() << prefix << "isn't punctuated as a"
								" complete sentence.";
					}
				}
			}
		}
		if (strstr(msg, "  ")) {
			ADD_FAILURE() << prefix << "uses multiple spaces in a row.";
		}
		for (auto p = msg; *p; ++p) {

			char c = *p;
			auto argref_found = false;
			char const * bad_argref = NULL;

			if (c == '%') {
				if (p[1] == 0) {
					ADD_FAILURE() << prefix << "ends with an orphaned %.";
				} else if (p[1] == '%') {
					++p;
				} else if (p[1] >= '1' && p[1] <= '9') {
					argref_found = true;
				} else {
					bad_argref = p;
				}
			}

			if (argref_found) {
				// Remember that we saw this argref.
				used_argrefs[p[1] - '1'] = true;

				// Look at any format spec following the arg ref.
				//   p[0] -> %
				//   p[1] -> digit
				//   p[2] -> {
				if (p[2] == '{') {

					// Doubled {{ means no format spec.
					if (p[3] == '{') {
						p += 3;
					} else {

						auto fmtspec_end = strchr(p + 3, '}');
						if (fmtspec_end) {
							p = fmtspec_end;
						} else {
							ADD_FAILURE() << prefix
									<< "has an orphaned { at offset "
									<< p + 3 - msg << ".";
						}
					}
				}
			}
			if (bad_argref) {
				ADD_FAILURE() << prefix << "has an invalid argref at offset "
						<< p - msg	<< ".";
			}
		}

		// Now check argrefs against name; we expect name to have the digit
		// for each argref in it, and we expect no gaps in argrefs.
		auto last_argref = 0;
		for (int i = 9; i >= 1; --i) {
			if (used_argrefs[i - 1]) {
				last_argref = i;
				break;
			}
		}
		for (int i = 1; i <= last_argref; ++i) {

			auto digit = strchr(name, '0' + i);
			auto in_name = !(
					digit == NULL
					|| digit == name
					|| digit[-1] != '_'
					|| !isalpha(digit[1]));

			if (!used_argrefs[i - 1]) {
				if (in_name) {
					ADD_FAILURE() << prefix << "never refers to arg " << i
							<< ".";
				} else {
					ADD_FAILURE() << prefix << "never refers to arg " << i
							<< ", and its symbolic name doesn't contain \"_"
							<< i << "x\""
								" (where x is a mnemonic describing the arg).";
				}
			} else if (!in_name) {
				ADD_FAILURE() << prefix << "refers to arg " << i
						<< ", but its symbolic name doesn't contain \"_" << i
						<< "x\" (where x is a mnemonic describing the arg).";
			}
		}
	}
}
