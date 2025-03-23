#include <iostream>
#include <string>
#include <thread>

#include "./tcp_server/tcp_ssl_server.h"
#include "./tcp_client/tcp_ssl_client.h"

#include <atomic>

std::atomic<bool> stopThreads(false);

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
    std::string clientPort = "9090"; // Port for the client to connect to the proxy
    std::string serverAddress = "127.0.0.1"; // Address of the actual server
    std::string serverPort = "8080"; // Port of the actual server

    // Logger function
    auto logger = [](const std::string& message) {
        std::cout << message << std::endl;
    };

    // Create the SSL server (proxy endpoint for the client)
    CTCPSSLServer proxyServer(logger, clientPort);
    proxyServer.SetSSLCertFile("/home/shyskov/Documents/libs/sem6/kursach/cert/certs/proxy.crt");
    proxyServer.SetSSLKeyFile("/home/shyskov/Documents/libs/sem6/kursach/cert/certs/proxy.key");
    proxyServer.SetSSLCerthAuth("/home/shyskov/Documents/libs/sem6/kursach/cert/certs/root.crt");

    // Create the SSL client (proxy endpoint for the actual server)
    CTCPSSLClient proxyClient(logger);
    proxyClient.SetSSLCertFile("/home/shyskov/Documents/libs/sem6/kursach/cert/certs/proxy.crt");
    proxyClient.SetSSLKeyFile("/home/shyskov/Documents/libs/sem6/kursach/cert/certs/proxy.key");
    proxyClient.SetSSLCerthAuth("/home/shyskov/Documents/libs/sem6/kursach/cert/certs/root.crt");

    ASecureSocket::SSLSocket clientSocket;

    // Listen for incoming client connections
    if (proxyServer.Listen(clientSocket)) {
        std::cout << "[Proxy] Client connected to proxy on port " << clientPort << std::endl;

        // Connect to the actual server
        if (proxyClient.Connect(serverAddress, serverPort)) {
            std::cout << "[Proxy] Connected to server at " << serverAddress << ":" << serverPort << std::endl;

            // Start threads to handle bidirectional traffic
            std::thread clientToServerThread(transferData, std::ref(proxyServer), std::ref(clientSocket), std::ref(proxyClient));
            std::thread serverToClientThread(transferDataReverse, std::ref(proxyServer), std::ref(clientSocket), std::ref(proxyClient));

            // Wait for threads to finish
            clientToServerThread.join();
            serverToClientThread.join();
        } else {
            std::cerr << "[Proxy] Failed to connect to server." << std::endl;
        }

        proxyServer.Disconnect(clientSocket);
    } else {
        std::cerr << "[Proxy] Failed to start proxy server." << std::endl;
    }

    return 0;
}