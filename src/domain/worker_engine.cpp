#include <condition_variable>
#include <thread>
#include <string.h>

#include "base/dbc.h"
#include "base/file_lines.h"
#include "base/strutil.h"
#include "base/xlog.h"

#include "domain/cmdline.h"
#include "domain/worker_engine.h"
#include "domain/event_codes.h"
#include "domain/msg.h"

#include "zeromq/include/zmq.h"

using namespace std;
using namespace nitro::event_codes;

namespace nitro {

worker_engine::worker_engine(cmdline const & cmdline) :
		engine(cmdline) {

    _job_annonce_sub = zmq_socket(ctx,ZMQ_SUB);
    if (_job_annonce_sub) {
      string eth = cmdline.get_option("--ethernet") ? cmdline.get_option("--ethernet") : DEFAULT_ETHERNET_INTERFACE;    
      string endpoint = "epgm://" + eth + ";239.192.1.1:5555";
      
      zmq_setsockopt(_job_annonce_sub, ZMQ_SUBSCRIBE, _subscription.c_str(), _subscription.size());

      int rc = zmq_connect(_job_annonce_sub, endpoint.c_str());
      xlog("connect to pgm: %1", rc);
      rc = zmq_connect(_job_annonce_sub, DEFAULT_IPC_ENDPOINT);
      xlog("connect to ipc: %1", rc);
    }
}

worker_engine::~worker_engine() {
  int linger = 0;
  if (_job_annonce_sub) {
    zmq_setsockopt(_job_annonce_sub, ZMQ_LINGER, &linger, sizeof(linger));
    zmq_close(_job_annonce_sub);
  }
}

int worker_engine::do_run() {

  if (_job_annonce_sub) {
    Json::Value root;

    // Waits for request from the server
    xlog("waiting for coordinator...");
    string json = receive_full_msg(_job_annonce_sub);

    if (json.size()
        && deserialize_msg(json, root) )
    {
      switch (root["body"]["code"].asInt()) {
        case NITRO_REQUEST_HELP:
          xlog("got request help command");
          // TODO: send response 
          break;
          
        default:
          xlog("got unknown command");
          break;
        }
    }
  }
	return 0;
}

} /* namespace nitro */
