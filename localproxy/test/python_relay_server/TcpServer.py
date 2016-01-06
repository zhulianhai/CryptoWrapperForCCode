import SocketServer 
import socket, sys

HOST = '0.0.0.0' 
PORT = 8089
ADDR = (HOST, PORT) 

class MyRequestHandler(SocketServer.BaseRequestHandler): 
    def handle(self): 
        print 'connected from:', self.client_address 
        while True:
            data = self.request.recv(1024)
            #print >>sys.stderr, 'received "%s"' % data
            #print "[TcpServer]Got data from", self.client_address, "data..", data
            if(len(data)>0):
                self.request.send(data)
                continue
            else:
                print  "error.........................."
                break
        self.request.close()

tcpServ = SocketServer.ThreadingTCPServer(ADDR, MyRequestHandler) 
print 'tcp server.waiting for connection...' 
tcpServ.serve_forever()

            