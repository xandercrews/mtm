import time, os, sys, zmq, json

NITRO_BATCH_PROGRESS_REPORT = 0x0600C06A

def main():
  context = zmq.Context()
  socket = context.socket(zmq.SUB)  
  socket.connect("tcp://localhost:35000");
  socket.setsockopt(zmq.SUBSCRIBE, "");
  print("Listening on port 35000...");
  start_time = None
  end_time = None
  while not end_time:
    msg = socket.recv()
    i = msg.find('0x')
    j = msg.find('"', i)
    code = eval(msg[i:j])
    if code == NITRO_BATCH_PROGRESS_REPORT:
       msg = msg[:-1]       
       text = json.loads(msg)
       print text["body"]["message"]
    else:
      print(msg)
      socket.send("ACK")

if __name__ == '__main__':
  main()
