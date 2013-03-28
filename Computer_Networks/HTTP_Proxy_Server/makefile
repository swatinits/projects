COMPILER =g++
CFLAGS = -Wall

All: HTTPProxyServer
HTTPProxyServer: server client

server: proxyserver.cpp defn.h
	$(COMPILER) proxyserver.cpp -o server
client: client.cpp defn.h
	$(COMPILER) client.cpp -o client

Clean:
	rm –rf*.o server client
