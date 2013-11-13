#include "base/dbc.h"
#include "base/guid.h"

#include "domain/assignment.h"
#include "domain/event_codes.h"

#include "json/json.h"

using std::string;

using namespace nitro::event_codes;

namespace nitro {

assignment::assignment(char const * id) :
		id(id && *id ? id : generate_guid().c_str()), taskmap() {
}

assignment::assignment(char const * id, char const * lines) : id(id), taskmap() {
	fill_from_lines(lines);
}

void assignment::fill_from_lines(char const * lines) {
	if (lines) {
		task::id_type n = 0;
		for (auto p = lines; *p;) {
			while (isspace(*p)) {
				++p;
			}
			auto end = strpbrk(lines, "\r\n");
			if (!end) {
				end = strchr(lines, 0);
			}
			while (end >= p && isspace(*end)) {
				--end;
			}
			++end;
			if (end > p) {
				add_task(++n, end);
			}
			p = end;
		}
	}
}

char const * assignment::get_id() const {
	return id.c_str();
}

task * assignment::add_task(task::id_type tid, char const * cmdline,
		char const * end_of_cmdline) {
	auto t = task::make(cmdline, end_of_cmdline, this, tid).release();
	if (t) {
		taskmap.insert(std::make_pair(t->get_id(), task::handle(t)));
	}
	return t;
}

void assignment::complete_task(task::id_type tid) {
	auto t = taskmap.find(tid);
	if (t != taskmap.end()) {
		taskmap[tid].reset();
	}
}

assignment::taskmap_t const & assignment::get_taskmap() const {
	return taskmap;
}

string assignment::get_request_msg() const {
	Json::Value root;
	root["messageId"] = generate_guid();
	root["senderId"] = "nitro@localhost"; // TODO: fix
	Json::Value body;
	body["code"] = events::get_std_id_repr(NITRO_HERE_IS_ASSIGNMENT);
	Json::Value tasks;
	for (auto i = taskmap.cbegin(); i != taskmap.cend(); ++i) {
		auto key = interp("%1", i->first);
		tasks[key] = i->second->get_cmdline();
	}
	body["tasks"] = tasks;
	root["body"] = body;
	Json::StyledWriter writer;
	return writer.write(root);
}

string assignment::get_status_msg() const {
	Json::Value root;
	char guid[GUID_BUF_LEN];
	generate_guid(guid, sizeof(guid));
	root["messageId"] = guid;
	root["senderId"] = "nitro@localhost"; // TODO: fix
	root["body"]["code"] = events::get_std_id_repr(NITRO_ASSIGNMENT_PROGRESS_REPORT);
	Json::Value status;
	int pending_count = 0;
	int done_count = 0;
	Json::Value pending(Json::arrayValue);
	Json::Value done(Json::arrayValue);
	for (auto i = taskmap.cbegin(); i != taskmap.cend(); ++i) {
		auto idstr = interp("%1", i->first);
		if (i->second) {
			pending[pending_count++] = idstr;
		} else {
			done[done_count++] = idstr;
		}
	}
	status["assigned_count"] = done_count + pending_count;
	status["done_count"] = done_count;
	status["done"] = done;
	status["pending_count"] = pending_count;
	status["pending"] = pending;
	root["body"]["status"] = status;
	Json::StyledWriter writer;
	return writer.write(root);
}


} // end namespace nitro
