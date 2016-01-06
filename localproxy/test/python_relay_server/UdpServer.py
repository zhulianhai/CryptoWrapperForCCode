#!/usr/bin/env python
# UDP Echo Server -  udpserver.py
# code by www.cppblog.com/jerryma
import socket, traceback
import threading

host = '0.0.0.0'
port = 8089

count = 0
last_count = -1;
def OnTimer():
    print count
    global t        #Notice: use global variable!
    t = threading.Timer(5.0, OnTimer)
    t.start()
	
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
s.bind((host, port))

t = threading.Timer(5.0, OnTimer)
t.start()

print 'udp server.waiting for connection...' 
while 1:
    try:
        message, address = s.recvfrom(8192)
#        print "[UdpServer]Got data from", address, "data.."
#        message += "reply by server"
        s.sendto(message, address)
        count = count + 1
    except (KeyboardInterrupt, SystemExit):
        raise
    except:
        traceback.print_exc()
