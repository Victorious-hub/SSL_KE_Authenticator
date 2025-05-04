CXX = g++
CXXFLAGS = -std=c++11 -Wall
LIBS = -lssl -lcrypto

DB_LIBS = -lsqlite3

SERVER_SOURCES = server.cpp ./tcp_client/tcp_ssl_client.cpp ./tcp_client/tcp_client.cpp ./utils/logger.cpp ./tcp_server/tcp_server.cpp ./tcp_server/tcp_ssl_server.cpp ./socket/secure_socket.cpp ./socket/socket.cpp
SERVER_TARGET = server

CLIENT_SOURCES = client.cpp ./utils/logger.cpp ./tcp_client/tcp_ssl_client.cpp ./tcp_client/tcp_client.cpp ./socket/secure_socket.cpp ./socket/socket.cpp
CLIENT_TARGET = client

PROXY_SOURCES = proxy.cpp ./utils/logger.cpp ./tcp_server/tcp_server.cpp ./tcp_server/tcp_ssl_server.cpp ./tcp_client/tcp_ssl_client.cpp ./tcp_client/tcp_client.cpp ./socket/secure_socket.cpp ./socket/socket.cpp
PROXY_TARGET = proxy

AUTH_SOURCES = auth_server.cpp ./utils/token_manager.cpp ./utils/logger.cpp ./db/sqlite3_provider.cpp ./tcp_server/tcp_server.cpp ./tcp_server/tcp_ssl_server.cpp ./socket/secure_socket.cpp ./socket/socket.cpp
AUTH_TARGET = auth_server

SQLITE3_SCRIPT_SOURCES = sqlite3_user.cpp ./db/sqlite3_provider.cpp ./utils/logger.cpp
SQLITE3_SCRIPT_TARGET = sqlite3_user

$(SQLITE3_SCRIPT_TARGET): $(SQLITE3_SCRIPT_SOURCES)
	$(CXX) $(CXXFLAGS) $(SQLITE3_SCRIPT_SOURCES) -o $(SQLITE3_SCRIPT_TARGET) -lssl -lcrypto -lsqlite3

$(SERVER_TARGET): $(SERVER_SOURCES)
	$(CXX) $(CXXFLAGS) $(SERVER_SOURCES) -o $(SERVER_TARGET) $(LIBS)

$(CLIENT_TARGET): $(CLIENT_SOURCES)
	$(CXX) $(CXXFLAGS) $(CLIENT_SOURCES) -o $(CLIENT_TARGET) $(LIBS)

$(PROXY_TARGET): $(PROXY_SOURCES)
	$(CXX) $(CXXFLAGS) $(PROXY_SOURCES) -o $(PROXY_TARGET) $(LIBS)

$(AUTH_TARGET): $(AUTH_SOURCES)
	$(CXX) $(CXXFLAGS) $(AUTH_SOURCES) -o $(AUTH_TARGET) $(LIBS) $(DB_LIBS)

clean:
	rm -f $(SERVER_TARGET) $(CLIENT_TARGET) $(AUTH_TARGET) $(PROXY_TARGET) *.o