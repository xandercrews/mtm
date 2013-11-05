#include <map>
#include <errno.h>

#include "gtest/gtest.h"
#include "base/event_ids.h"
#include "base/interp.h"
#include "domain/event_ids.h"
#include "test/test_util.h"

using namespace std;

events const & catalog = events::catalog();

TEST(event_test, no_dups) {
	typedef std::map<int, char const *> Map;
	Map names_by_number;
	auto dm = catalog.get_items();
	for (auto i = dm.begin(); i != dm.end(); ++i) {
		auto item = **i;
		auto id = item.id;
		auto name = item.name;
		if (names_by_number.find(id) == names_by_number.end()) {
			names_by_number[id] = name;
		} else {
			ADD_FAILURE() << "Duplicate number " << id << " for " << name
					<< " and " << names_by_number[id] << ".";
		}
	}
}

TEST(event_test, valid_names_and_argrefs) {
	auto dm = catalog.get_items();
	for (auto i = dm.begin(); i != dm.end(); ++i) {
		auto item = **i;
		auto id = item.id;
		auto name = item.name;
		for (auto p = name; *p; ++p) {
			if (toupper(*p) != *p) {
				ADD_FAILURE() << name << " is not upper-case.";
				break;
			}
		}

		// Scan the message text. Notice which args have been used; verify
		// that all argrefs are formatted correctly. Check capitalization and
		// punctuation.
		bool used_argrefs[] = {false, false, false, false, false, false, false,
				false, false};
		auto msg = catalog.get_msg(id);
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

TEST(event_test, known_event_properties) {
	auto e = base::event_ids::E_NOT_IMPLEMENTED;
	EXPECT_STREQ("E_NOT_IMPLEMENTED", catalog.get_symbolic_name(e));
	EXPECT_EQ(sev_error, catalog.get_severity(e));
	EXPECT_EQ(kc_base, catalog.get_component(e));
	EXPECT_EQ(esc_internal, catalog.get_escalation(e));
	EXPECT_EQ(1, catalog.get_nonunique_number(e));
	EXPECT_STREQ("domain.base.internal", catalog.get_topic(e));
	expect_str_contains(catalog.get_msg(e), "not yet been implemented");
	expect_str_contains(catalog.get_comments(e), "stubbed");
	EXPECT_EQ(0, catalog.get_arg_count(e));
}

TEST(event_test, multi_arg_count) {
	auto e = nitro::event_ids::NITRO_1FILE_BAD_HUGE_LINE_2BYTES;
	EXPECT_EQ(2, catalog.get_arg_count(e));
}

eid_t get_foreign_event_id() {
	return static_cast<eid_t>(
			static_cast<int>(sev_warning) << 28
			| static_cast<int>(kc_mwm) << 16
			| static_cast<int>(esc_poweruser) << 14
			| 1234
			);
}

TEST(event_test, unknown_event_properties) {
	auto e = get_foreign_event_id();
	EXPECT_STREQ("", catalog.get_symbolic_name(e));
	EXPECT_EQ(sev_warning, catalog.get_severity(e));
	EXPECT_EQ(kc_mwm, catalog.get_component(e));
	EXPECT_EQ(esc_poweruser, catalog.get_escalation(e));
	EXPECT_EQ(1234, catalog.get_nonunique_number(e));
	EXPECT_STREQ("", catalog.get_topic(e));
	EXPECT_STREQ("", catalog.get_msg(e));
	EXPECT_STREQ("", catalog.get_comments(e));
	EXPECT_EQ(0, catalog.get_arg_count(e));
}

TEST(event_test, posix_properties) {
	auto e = ENOENT;
	EXPECT_STREQ("", catalog.get_symbolic_name(e));
	EXPECT_EQ(sev_error, catalog.get_severity(e));
	EXPECT_EQ(kc_posix, catalog.get_component(e));
	EXPECT_EQ(esc_user, catalog.get_escalation(e));
	EXPECT_EQ(2, catalog.get_nonunique_number(e));
	EXPECT_STREQ("", catalog.get_topic(e));
	expect_str_contains(catalog.get_msg(e).c_str(), "such file or directory");
	EXPECT_STREQ("", catalog.get_comments(e));
	EXPECT_EQ(0, catalog.get_arg_count(e));
}
