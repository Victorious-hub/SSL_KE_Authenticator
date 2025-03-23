#include <iostream>
#include <string>
#include <thread>
#include <atomic>

#include "./tcp_server/tcp_ssl_server.h"
#include "./tcp_client/tcp_ssl_client.h"

std::atomic<bool> stopThreads(false);

// Function to handle authentication requests
void handleAuthRequest(CTCPSSLServer& proxyServer, ASecureSocket::SSLSocket& clientSocket, CTCPSSLClient& proxyAuthyClient) {
    char buffer[1024];
    while (!stopThreads) {
        int bytesReceived = proxyServer.Receive(clientSocket, buffer, sizeof(buffer), false);
        if (bytesReceived <= 0) {
            std::cerr << "[Proxy] Client disconnected or error occurred during authentication." << std::endl;
            stopThreads = true;
            break;
        }

        std::string authRequest(buffer, bytesReceived);
        std::cout << "[Proxy] Forwarding authentication request to auth_server: " << authRequest << std::endl;

        if (!proxyAuthyClient.Send(authRequest)) {
            std::cerr << "[Proxy] Failed to forward authentication request to auth_server." << std::endl;
            stopThreads = true;
            break;
        }

        char authResponse[1024] = {0};
        int authBytesReceived = proxyAuthyClient.Receive(authResponse, sizeof(authResponse), false);
        if (authBytesReceived <= 0) {
            std::cerr << "[Proxy] Failed to receive token from auth_server." << std::endl;
            stopThreads = true;
            break;
        }

        std::string token(authResponse, authBytesReceived);
        std::cout << "[Proxy] Received token from auth_server: " << token << std::endl;

        // Send the token back to the client
        if (!proxyServer.Send(clientSocket, token)) {
            std::cerr << "[Proxy] Failed to send token back to client." << std::endl;
            stopThreads = true;
            break;
        }
    }
}

// Function to handle regular client-to-server communication
void transferData(CTCPSSLServer& server, ASecureSocket::SSLSocket& clientSocket, CTCPSSLClient& proxyClient) {
    char buffer[1024];
    while (!stopThreads) {
        int bytesReceived = server.Receive(clientSocket, buffer, sizeof(buffer), false);
        if (bytesReceived <= 0) {
            std::cerr << "[Proxy] Client disconnected or error occurred while receiving data." << std::endl;
            stopThreads = true;
            break;
        }

        std::string data(buffer, bytesReceived);
        std::cout << "[Proxy] Forwarding to server: " << data << std::endl;

        if (!proxyClient.Send(data)) {
            std::cerr << "[Proxy] Failed to forward data to server." << std::endl;
            stopThreads = true;
            break;
        }
    }
}

// Function to handle server-to-client communication
void transferDataReverse(CTCPSSLServer& server, ASecureSocket::SSLSocket& clientSocket, CTCPSSLClient& proxyClient) {
    char buffer[1024];
    while (!stopThreads) {
        int bytesReceived = proxyClient.Receive(buffer, sizeof(buffer), false);
        if (bytesReceived <= 0) {
            std::cerr << "[Proxy] Server disconnected or error occurred while receiving data." << std::endl;
            stopThreads = true;
            break;
        }

        std::string data(buffer, bytesReceived);
        std::cout << "[Proxy] Forwarding to client: " << data << std::endl;

        if (!server.Send(clientSocket, data)) {
            std::cerr << "[Proxy] Failed to forward data to client." << std::endl;
            stopThreads = true;
            break;
        }
    }
}

int main() {
    std::string clientPort = "9090";
    std::string serverAddress = "127.0.0.1";
    std::string serverPort = "8080";
    std::string authServerAddress = "127.0.0.1";
    std::string authServerPort = "8081";

    auto logger = [](const std::string& message) {
        std::cout << message << std::endl;
    };

    CTCPSSLServer proxyServer(logger, clientPort);
    proxyServer.SetSSLCertFile("/home/shyskov/Documents/libs/sem6/kursach/cert/certs/proxy.crt");
    proxyServer.SetSSLKeyFile("/home/shyskov/Documents/libs/sem6/kursach/cert/certs/proxy.key");
    proxyServer.SetSSLCerthAuth("/home/shyskov/Documents/libs/sem6/kursach/cert/certs/root.crt");

    CTCPSSLClient proxyClient(logger);
    proxyClient.SetSSLCertFile("/home/shyskov/Documents/libs/sem6/kursach/cert/certs/proxy.crt");
    proxyClient.SetSSLKeyFile("/home/shyskov/Documents/libs/sem6/kursach/cert/certs/proxy.key");
    proxyClient.SetSSLCerthAuth("/home/shyskov/Documents/libs/sem6/kursach/cert/certs/root.crt");

    CTCPSSLClient proxyAuthyClient(logger);
    proxyAuthyClient.SetSSLCertFile("/home/shyskov/Documents/libs/sem6/kursach/cert/certs/proxy.crt");
    proxyAuthyClient.SetSSLKeyFile("/home/shyskov/Documents/libs/sem6/kursach/cert/certs/proxy.key");
    proxyAuthyClient.SetSSLCerthAuth("/home/shyskov/Documents/libs/sem6/kursach/cert/certs/root.crt");

    ASecureSocket::SSLSocket clientSocket;
    if (proxyServer.Listen(clientSocket)) {
        std::cout << "[Proxy] Client connected to proxy on port " << clientPort << std::endl;
        if (proxyAuthyClient.Connect(authServerAddress, authServerPort)) {
            std::cout << "[Proxy] Connected to auth_server at " << authServerAddress << ":" << authServerPort << std::endl;
            handleAuthRequest(proxyServer, clientSocket, proxyAuthyClient);
            if (proxyClient.Connect(serverAddress, serverPort)) {
                std::cout << "[Proxy] Connected to server at " << serverAddress << ":" << serverPort << std::endl;
                std::thread clientToServerThread(transferData, std::ref(proxyServer), std::ref(clientSocket), std::ref(proxyClient));
                std::thread serverToClientThread(transferDataReverse, std::ref(proxyServer), std::ref(clientSocket), std::ref(proxyClient));

                clientToServerThread.join();
                serverToClientThread.join();
            } else {
                std::cerr << "[Proxy] Failed to connect to server." << std::endl;
            }
        } else {
            std::cerr << "[Proxy] Failed to connect to auth_server." << std::endl;
        }

        proxyServer.Disconnect(clientSocket);
    } else {
        std::cerr << "[Proxy] Failed to start proxy server." << std::endl;
    }

    return 0;
}