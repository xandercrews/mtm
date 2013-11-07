import time, os, sys, zmq

NITRO_REQUEST_HELP = 0x0600C065
NITRO_AFFIRM_HELP = 0x0600C066
NITRO_HERE_IS_ASSIGNMENT = 0x0600C064
NITRO_TERMINATE_REQUEST = 0x0600C06C

def main():
  context = zmq.Context()
  socket = context.socket(zmq.REP)
  socket.bind("tcp://*:36000")
  print("Listening on port 36000...");
  start_time = None
  end_time = None
  while True:
    msg = socket.recv()
    i = msg.find('0x')
    j = msg.find('"', i)
    code = eval(msg[i:j])
    if code == NITRO_REQUEST_HELP:
       print("Got enroll request. Responded with AFFIRM.")
       socket.send('{"body":{"code":"0x%08X"}}' % NITRO_AFFIRM_HELP)
       start_time = os.times()[4]
    elif code == NITRO_HERE_IS_ASSIGNMENT:
       i = msg.find('"fake')
       j = msg.find('"', i + 1)
       cmdline = msg[i+1:j]
       print("Got assignment: %s" % cmdline)
       socket.send("ACK")
    elif code == NITRO_TERMINATE_REQUEST:
       end_time = os.times()[4]
       print("Batch ended; elapsed time: %.2f secs" % (end_time - start_time))
       sys.exit(0)
    else:
      print(msg)
      socket.send("ACK")

if __name__ == '__main__':
  main()
