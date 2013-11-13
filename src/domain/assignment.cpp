#include "base/dbc.h"
#include "base/guid.h"

#include "domain/assignment.h"
#include "domain/event_codes.h"

#include "json/json.h"

using std::string;

using namespace nitro::event_codes;

namespace nitro {

assignment::assignment(char const * id) :
		id(id && *id ? id : generate_guid().c_str()) {
}

assignment::assignment(char const * id, char const * lines) :
		id(id) {
	fill_from_lines(lines);
}

void assignment::fill_from_lines(char const * lines) {
	if (lines) {
		task::id_type n = 0;
		for (auto p = lines; *p;) {
			while (isspace(*p)) {
				++p;
			}
			if (*p == 0) {
				break;
			}
			auto end = strpbrk(p, "\r\n");
			if (!end) {
				end = strchr(p, 0);
			}
			while (end >= p && isspace(*end)) {
				--end;
			}
			++end;
			if (end > p) {
				ready_task(++n, end);
			}
			p = end;
		}
	}
}

char const * assignment::get_id() const {
	return id.c_str();
}

task * assignment::ready_task(task::id_type tid, char const * cmdline,
		char const * end_of_cmdline) {
	auto t = task::make(cmdline, end_of_cmdline, this, tid).release();
	if (t) {
		lists[task_status::ts_ready].push_back(task::handle(t));
	}
	return t;
}

void assignment::activate_task(task::id_type tid) {
	// Although this loop looks moderately expensive, it should, in practice,
	// be virtually instantaneous, because we are always going to activate
	// the first item in the ready list.
	tasklist_t & alist = lists[task_status::ts_active];
	tasklist_t & rlist = lists[task_status::ts_ready];
	for (auto i = rlist.begin(); i != rlist.end(); ++i) {
		task::handle & thandle = *i;
		if (thandle->get_id() == tid) {
			alist.push_back(task::handle(thandle.release()));
			rlist.erase(i);
			return;
		}
	}
}

void assignment::complete_task(task::id_type tid) {
	// Although this loop looks moderately expensive, it should, in practice,
	// be cheap, because we are always going to have only a handful of active
	// items.
	tasklist_t & alist = lists[task_status::ts_active];
	tasklist_t & clist = lists[task_status::ts_complete];
	for (auto i = alist.begin(); i != alist.end(); ++i) {
		task::handle & thandle = *i;
		if (thandle->get_id() == tid) {
			clist.push_back(task::handle(thandle.release()));
			alist.erase(i);
			return;
		}
	}
}

string assignment::get_request_msg() const {
	Json::Value root;
	root["messageId"] = generate_guid();
	root["senderId"] = "nitro@localhost"; // TODO: fix
	Json::Value body;
	body["code"] = events::get_std_id_repr(NITRO_HERE_IS_ASSIGNMENT);
	Json::Value tasks;
	tasklist_t const & rlist = lists[task_status::ts_ready];
	for (auto i = rlist.cbegin(); i != rlist.cend(); ++i) {
		auto key = interp("%1", (*i)->get_id());
		tasks[key] = (*i)->get_cmdline();
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
	root["body"]["code"] = events::get_std_id_repr(
			NITRO_ASSIGNMENT_PROGRESS_REPORT);
	Json::Value status;
	int counts[3];
	Json::Value pending(Json::arrayValue);
	Json::Value done(Json::arrayValue);
	for (task_status stat = task_status::ts_ready;
			stat <= task_status::ts_complete; ++stat) {
		counts[stat] = 0;
		Json::Value items(Json::arrayValue);
		tasklist_t const & tlist = lists[stat];
		for (auto i = tlist.cbegin(); i != tlist.cend(); ++i) {
			auto idstr = interp("%1", (*i)->get_id());
			items[counts[stat]++] = idstr;
		}
		auto name = get_status_name(stat);
		auto count_key = interp("%1_count", name);
		status[name] = items;
		status[count_key] = counts[stat];
	}
	root["body"]["status"] = status;
	Json::StyledWriter writer;
	return writer.write(root);
}

assignment::tasklist_t const & assignment::get_list_by_status(
		task_status ts) const {
	PRECONDITION(ts >= ts_ready && ts <= ts_complete);
	return lists[ts];
}

} // end namespace nitro
