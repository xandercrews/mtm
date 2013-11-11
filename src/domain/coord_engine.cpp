#include <algorithm>
#include <condition_variable>
#include <thread>
#include <string.h>

#include "base/dbc.h"
#include "base/file_lines.h"
#include "base/strutil.h"
#include "base/xlog.h"

#include "domain/cmdline.h"
#include "domain/coord_engine.h"
#include "domain/event_ids.h"
#include "domain/msg.h"

#include "zeromq/include/zmq.h"

using namespace std;
using namespace nitro::event_ids;

namespace nitro {

coord_engine::coord_engine(cmdline const & cmdline) :
		engine(cmdline), requester(0) {

	auto exec_host = cmdline.get_option("--exechost");
	if (!exec_host) {
		exec_host = getenv("exec_host");
	}
	if (exec_host) {
		try {
			file_lines fl(exec_host);
			while (true) {
				auto line = fl.next();
				if (line == nullptr) {
					break;
				}
#if 0
				string ln(line);
				split(ln, ',', hostlist);
#else
				hostlist.push_back(line);
#endif

			}
			auto pargs = cmdline.get_positional_args();
			for (auto batch: pargs) {
				batches.push_back(batch);
			}
			return;
		} catch (error_event const & e) {
			// Must not be a file. Try raw list.
#if 0
			string eh(exec_host);
			split(trim(eh), ',', hostlist);
#else
			hostlist.push_back(exec_host);
#endif
		}
	}

  // Create pub socket to send multicast job annonces 
  _job_annonce_pgm = zmq_socket(ctx, ZMQ_PUB);
  _job_annonce_ipc = zmq_socket(ctx, ZMQ_PUB);
  if (_job_annonce_pgm
      && _job_annonce_ipc) {

    // bind socket for remote connections
    std::string eth = cmdline.get_option("--ethernet") ? cmdline.get_option("--ethernet") : DEFAULT_ETHERNET_INTERFACE;    
    std::string endpoint = "epgm://" + eth + ";239.192.1.1:5555";
    int rc = zmq_bind(_job_annonce_pgm,endpoint.c_str());
    xlog("pub socket bind result: %1", rc);

    // bind socket for local connections
    rc = zmq_bind(_job_annonce_ipc, DEFAULT_IPC_ENDPOINT);
    xlog("ipc pub socket bind result: %1", rc);
  }
}

coord_engine::~coord_engine() {
  int linger = 0;
  if (_job_annonce_pgm) {
    zmq_setsockopt(_job_annonce_pgm, ZMQ_LINGER, &linger, sizeof(linger));
    zmq_close(_job_annonce_pgm);
  }
  if (_job_annonce_ipc) {
    zmq_setsockopt(_job_annonce_ipc, ZMQ_LINGER, &linger, sizeof(linger));
    zmq_close(_job_annonce_ipc);
  }
}

void coord_engine::enroll_workers_multi(int eid)
{
  if (_job_annonce_pgm
      && _job_annonce_ipc) 
  {
    static const string cfm_port="50000";

    // Clear workers list
    _workers.clear();

    string json_msg = serialize_msg(eid, cfm_port);
    zmq_msg_t msg, msg_copy;

    // Create 2 copy of message
    zmq_msg_init_size(&msg, json_msg.size() + _subscription.size());
    zmq_msg_init_size(&msg_copy, json_msg.size() + _subscription.size());
    memcpy(zmq_msg_data(&msg), _subscription.c_str(), _subscription.size());
    memcpy((char *)zmq_msg_data(&msg) + _subscription.size(), json_msg.c_str(), json_msg.size());
    zmq_msg_copy(&msg_copy, &msg);

    // Send messages
    int rc = zmq_sendmsg(_job_annonce_pgm, &msg, 0);
    xlog("pgm message sending: %1", rc);

    rc = zmq_sendmsg(_job_annonce_ipc, &msg_copy, 0);
    xlog("ipc message sending: %1", rc);

    // Wait for the workers responces
    void *cfm = zmq_socket(ctx, ZMQ_PULL);
    if (cfm) {
      string endpoint = string("tcp://*:") + cfm_port;
      zmq_bind(cfm, endpoint.c_str());

      zmq_pollitem_t items [1];
      items[0].socket = cfm;
      items[0].events = ZMQ_POLLIN;

      // Stop wating workers if no connection for 5 sec
      xlog("waiting confirmations from workers...");
      while ( (rc = zmq_poll(items, sizeof(items) / sizeof(items[0]), 5000)) > 0 )
      {
        for (unsigned i = 0; i < sizeof(items) / sizeof(items[0]); i++)
        {
          if (items[i].revents & ZMQ_POLLIN)
          {
            Json::Value root;
            string json = receive_full_msg(items[i].socket);

            if (deserialize_msg(json, root))
            {
              _workers.push_back(root["body"]["message"].asCString());
              xlog("Got response from worker: %1", root["body"]["message"].asCString());
            }
          }
        }
      }

      int linger = 0;  
      zmq_setsockopt(cfm, ZMQ_LINGER, &linger, sizeof(linger));
      zmq_close(cfm);
      xlog("... done waiting confirmations from workers");

      // TODO: connect directly to missed workerks
    }
  }
}

#define tryz(expr) rc = expr; if (rc) throw ERROR_EVENT(errno)

void coord_engine::enroll_workers(int eid) {
#if 1
	if (hostlist.empty()) {
		hostlist.push_back("localhost:36000");
	}
#endif
	if (!hostlist.empty()) {
		try {
			for (auto host: hostlist) {
				requester = zmq_socket(ctx, ZMQ_REQ);
				string endpoint = interp("tcp://%1", host);
				int rc;
				tryz(zmq_connect(requester, endpoint.c_str()));
				string msg = serialize_msg(eid);
				send_full_msg(requester, msg);
				auto response = receive_full_msg(requester);
				// TODO: check response
				xlog("Enrolled %1.", endpoint);
				zmq_close(requester);
			}
		} catch (error_event const & e) {
			zmq_close(requester);
			throw;
		}
	}
}

coord_engine::assignment_t coord_engine::next_assignment() {
	const auto MAX_SIZE = 1000;
	assignment_t new_a;
	while (true) {
		if (!current_batch_file) {
			if (batches.empty()) {
				return new_a;
			} else {
				auto fl = new file_lines(batches.front().c_str());
				current_batch_file = std::unique_ptr<file_lines>(fl);
				batches.erase(batches.begin());
			}
		}
		while (true) {
			auto line = current_batch_file->next();
			if (line) {
				if (!new_a) {
					new_a = assignment_t(new stringlist_t);
				}
				new_a->push_back(line);
				if (new_a->size() >= MAX_SIZE) {
					return new_a;
				}
			} else {
				current_batch_file.reset();
			}
		}
	}
}

void coord_engine::prioritize(assignment_t & asgn) {
	// This is just a demo of doing a sort.
	struct priority_func {
		int operator ()(std::string const & a, std::string const & b) {
			// Here we could parse strings a and b and see if userprio was set,
			// and generate a sort key for each. In the long run, we probably
			// want to change the assignment_t datatype so it's not a list of
			// strings, but rather a list of parsed data that already has
			// sort key calculated.
			return a.size() - b.size();
		}
	};
	std::sort((*asgn).begin(), (*asgn).end(), priority_func());
}

void coord_engine::distribute(assignment_t & asgn) {
	auto host = hostlist.begin();
    for (auto cmd : *asgn) {
		auto endpoint = interp("tcp://%1", *host);
		void * requester;
    	try {
			requester = zmq_socket(ctx, ZMQ_REQ);
			int rc;
			tryz(zmq_connect(requester, endpoint.c_str()));
			auto msg = serialize_msg(NITRO_HERE_IS_ASSIGNMENT, cmd);
			send_full_msg(requester, msg);
			// Process ACK
			receive_full_msg(requester);
			++host;
			if (host == hostlist.end()) {
				host = hostlist.begin();
			}
			zmq_close(requester);
    	} catch (error_event const & e) {
    		zmq_close(requester);
    	}
    }
}

int coord_engine::do_run() {
	// This is totally the wrong way to do dispatch of assignments. I'm
	// completely abusing zmq by short-circuiting its own fair share routing
	// and by creating and destroying sockets right and left. I've only done
	// it this way to get some logic running that I can improve incrementally.

    enroll_workers_multi(NITRO_REQUEST_HELP);
	enroll_workers(NITRO_REQUEST_HELP);

	while (true) {
		auto assignment = next_assignment();
		if (!assignment) {
			break;
		}
		prioritize(assignment);
		// who's not busy?
		distribute(assignment);
	}

	enroll_workers(NITRO_TERMINATE_REQUEST);
	this_thread::sleep_for(chrono::milliseconds(100));
	int linger = 0;
	zmq_setsockopt(requester, ZMQ_LINGER, &linger, sizeof(linger));
	zmq_close(requester);
	xlog("Completed all batches.");

	return 0;
}

#if 0
	while (more_files) {
		try {
			Batch batch(file);
			Chunk = get_chunk();
			chunk.prioritize();
			chunk.find_eligible_jobs();
			chunk.distribute();
		} catch (std::runtime_error const & e) {

		}
	}
#endif
} // end namespace nitro
