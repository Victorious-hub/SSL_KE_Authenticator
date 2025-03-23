CXX = g++
CXXFLAGS = -std=c++11 -Wall
LIBS = -lssl -lcrypto

SERVER_SOURCES = server.cpp ./utils/logger.cpp ./tcp_server/tcp_server.cpp ./tcp_server/tcp_ssl_server.cpp ./socket/secure_socket.cpp ./socket/socket.cpp
SERVER_TARGET = server

CLIENT_SOURCES = client.cpp ./utils/logger.cpp ./tcp_client/tcp_ssl_client.cpp ./tcp_client/tcp_client.cpp ./socket/secure_socket.cpp ./socket/socket.cpp
CLIENT_TARGET = client

PROXY_SOURCES = proxy.cpp ./utils/logger.cpp ./tcp_server/tcp_server.cpp ./tcp_server/tcp_ssl_server.cpp ./tcp_client/tcp_ssl_client.cpp ./tcp_client/tcp_client.cpp ./socket/secure_socket.cpp ./socket/socket.cpp
PROXY_TARGET = proxy

$(SERVER_TARGET): $(SERVER_SOURCES)
	$(CXX) $(CXXFLAGS) $(SERVER_SOURCES) -o $(SERVER_TARGET) $(LIBS)

$(CLIENT_TARGET): $(CLIENT_SOURCES)
	$(CXX) $(CXXFLAGS) $(CLIENT_SOURCES) -o $(CLIENT_TARGET) $(LIBS)

$(PROXY_TARGET): $(PROXY_SOURCES)
	$(CXX) $(CXXFLAGS) $(PROXY_SOURCES) -o $(PROXY_TARGET) $(LIBS)

clean:
	rm -f $(SERVER_TARGET) $(CLIENT_TARGET) *.o