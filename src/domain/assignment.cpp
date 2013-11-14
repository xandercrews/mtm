#include <mutex>

#include "base/dbc.h"
#include "base/guid.h"

#include "domain/assignment.h"
#include "domain/event_codes.h"

#include "json/json.h"

using std::string;
using std::lock_guard;

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
		// Technically, it should be unnecessary to lock here, because
		// this function is only called in a constructor. However, I've
		// added the lock because I don't want the function to get moved
		// into a more general callability, only to have thread safety break.
		lock_guard<std::mutex> lock(mutex);
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
			do {
				--end;
			} while (end >= p && isspace(*end));
			++end;
			if (end > p) {
				_ready_task(++n, p, end);
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
	lock_guard<std::mutex> lock(mutex);
	return _ready_task(tid, cmdline, end_of_cmdline);
}

task * assignment::_ready_task(task::id_type tid, char const * cmdline,
		char const * end_of_cmdline) {
	auto t = task::make(cmdline, end_of_cmdline, this, tid).release();
	if (t) {
		lists[task_status::ts_ready].push_back(task::handle(t));
	}
	return t;
}

char const * assignment::activate_task(task::id_type tid) {
	lock_guard<std::mutex> lock(mutex);
	char const * cmdline = nullptr;
	// Although this loop looks moderately expensive, it should, in practice,
	// be virtually instantaneous, because we are always going to activate
	// the first item in the ready list.
	tasklist_t & rlist = lists[ts_ready];
	for (tasklist_t::iterator i = rlist.begin(); i != rlist.end(); ++i) {
		task::handle & thandle = *i;
		if (thandle->get_id() == tid) {
			cmdline = thandle->get_cmdline();
			tasklist_t & alist = lists[ts_active];
			alist.push_back(task::handle(thandle.release()));
			rlist.erase(i);
			break;
		}
	}
	return cmdline;
}

bool assignment::complete_task(task::id_type tid) {
	lock_guard<std::mutex> lock(mutex);
	// Although this loop looks moderately expensive, it should, in practice,
	// be cheap, because we are always going to have only a handful of active
	// items.
	tasklist_t & alist = lists[ts_active];
	for (tasklist_t::iterator i = alist.begin(); i != alist.end(); ++i) {
		task::handle & thandle = *i;
		if (thandle->get_id() == tid) {
			tasklist_t & clist = lists[ts_complete];
			clist.push_back(task::handle(thandle.release()));
			alist.erase(i);
			break;
		}
	}
	return _is_complete();
}

bool assignment::_is_complete() const {
	return lists[ts_ready].empty() && lists[ts_active].empty();
}

bool assignment::is_complete() const {
	lock_guard<std::mutex> lock(mutex);
	return _is_complete();
}

string assignment::get_request_msg() const {
	lock_guard<std::mutex> lock(mutex);
	Json::Value root;
	root["messageId"] = generate_guid();
	root["senderId"] = "nitro@localhost"; // TODO: fix
	Json::Value body;
	body["code"] = events::get_std_id_repr(NITRO_HERE_IS_ASSIGNMENT);
	Json::Value tasks;
	tasklist_t const & rlist = lists[task_status::ts_ready];
	for (tasklist_t::const_iterator i = rlist.cbegin(); i != rlist.cend();
			++i) {
		auto key = interp("%1", (*i)->get_id());
		tasks[key] = (*i)->get_cmdline();
	}
	body["tasks"] = tasks;
	root["body"] = body;
	Json::StyledWriter writer;
	return writer.write(root);
}

string assignment::get_status_msg() const {
	lock_guard<std::mutex> lock(mutex);
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
		for (tasklist_t::const_iterator i = tlist.cbegin(); i != tlist.cend();
				++i) {
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
