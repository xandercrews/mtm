import os, sys, zmq

def main():
    context = zmq.Context()
    socket = context.socket(zmq.SUB)
    socket.setsockopt(zmq.SUBSCRIBE, "")
    socket.connect("tcp://10.1.13.255:47001")
    print("Listening on port 47001...");
    while True:
        msg = socket.recv()
        print(msg)

if __name__ == '__main__':
  main()
