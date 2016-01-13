import socket, sys
import time 
  
address = ('127.0.0.1', 8090)  
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
while True:
	try:  
		s.connect(address)
		break
	except Exception, e:
		time.sleep(0.1)
		continue
print('connect ok')
while 1:
    print "Enter data to transmit:"
    data = sys.stdin.readline().strip()
    s.send(data)
	
    buffer = s.recv(512)
    if len(buffer) > 0:  
		print 'received is',buffer   
		continue
    else:
		break 
  
s.close()  