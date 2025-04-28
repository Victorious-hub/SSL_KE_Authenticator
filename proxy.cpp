#include <iostream>
#include <string>
#include <thread>
#include <atomic>

#include "./tcp_server/tcp_ssl_server.h"
#include "./tcp_client/tcp_ssl_client.h"

std::atomic<bool> stopThreads(false);

// Function to handle authentication with the auth server
bool authenticateWithAuthServer(CTCPSSLServer& proxyServer, ASecureSocket::SSLSocket& clientSocket, CTCPSSLClient& proxyAuthyClient, std::string& token) {
    char buffer[1024];
    int bytesReceived = proxyServer.Receive(clientSocket, buffer, sizeof(buffer), false);
    if (bytesReceived <= 0) {
        std::cerr << "[Proxy] Client disconnected or error occurred during authentication." << std::endl;
        return false;
    }

    std::string clientMessage(buffer, bytesReceived);
    std::cout << "[Proxy] Received message from client: " << clientMessage << std::endl;

    if (clientMessage.rfind("AUTH ", 0) == 0) {
        // Forward the AUTH request to the auth server
        if (!proxyAuthyClient.Send(clientMessage)) {
            std::cerr << "[Proxy] Failed to send AUTH request to auth_server." << std::endl;
            proxyServer.Send(clientSocket, "ERROR: Authentication server unavailable.");
            return false;
        }

        char authResponse[1024] = {0};
        int authBytesReceived = proxyAuthyClient.Receive(authResponse, sizeof(authResponse), false);
        if (authBytesReceived <= 0) {
            std::cerr << "[Proxy] Failed to receive token from auth_server." << std::endl;
            proxyServer.Send(clientSocket, "ERROR: Authentication failed.");
            return false;
        }

        token = std::string(authResponse, authBytesReceived);
        std::cout << "[Proxy] Received token from auth_server: " << token << std::endl;

        // Send the token back to the client
        if (!proxyServer.Send(clientSocket, token)) {
            std::cerr << "[Proxy] Failed to send token back to client." << std::endl;
            return false;
        }

        return true;
    }

    std::cerr << "[Proxy] Invalid authentication request format." << std::endl;
    proxyServer.Send(clientSocket, "ERROR: Invalid authentication request.");
    return false;
}

// Function to verify the token with the auth server
bool verifyTokenWithAuthServer(CTCPSSLClient& proxyAuthyClient, const std::string& token) {
    std::string authRequest = "VERIFY " + token;
    std::cout << "[Debug] Sending VERIFY request to auth_server: " << authRequest << std::endl;

    if (!proxyAuthyClient.Send(authRequest)) {
        std::cerr << "[Proxy] Failed to send token verification request to auth_server." << std::endl;
        return false;
    }

    char authResponse[1024] = {0};
    int authBytesReceived = proxyAuthyClient.Receive(authResponse, sizeof(authResponse), false);
    if (authBytesReceived <= 0) {
        std::cerr << "[Proxy] Failed to receive verification response from auth_server." << std::endl;
        return false;
    }

    std::string authResult(authResponse, authBytesReceived);
    std::cout << "[Debug] Received verification response from auth_server: " << authResult << std::endl;

    return authResult == "VERIFIED";
}

// Function to forward messages to the server
bool forwardMessageToServer(CTCPSSLClient& proxyClient, const std::string& userMessage, std::string& serverResponse) {
    if (!proxyClient.Send(userMessage)) {
        std::cerr << "[Proxy] Failed to forward message to server." << std::endl;
        return false;
    }

    char buffer[1024] = {0};
    int bytesReceived = proxyClient.Receive(buffer, sizeof(buffer), false);
    if (bytesReceived <= 0) {
        std::cerr << "[Proxy] Failed to receive response from server." << std::endl;
        return false;
    }

    serverResponse = std::string(buffer, bytesReceived);
    return true;
}

std::string trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r");
    size_t end = str.find_last_not_of(" \t\n\r");
    return (start == std::string::npos) ? "" : str.substr(start, end - start + 1);
}


// Function to handle client requests after authentication
void handleClientRequests(CTCPSSLServer& proxyServer, ASecureSocket::SSLSocket& clientSocket, CTCPSSLClient& proxyAuthyClient, CTCPSSLClient& proxyClient, const std::string& token) {
    char buffer[1024];
    while (!stopThreads) {
        int bytesReceived = proxyServer.Receive(clientSocket, buffer, sizeof(buffer), false);
        if (bytesReceived <= 0) {
            std::cerr << "[Proxy] Client disconnected or error occurred." << std::endl;
            break;
        }

        std::string clientMessage(buffer, bytesReceived);
        std::cout << "[Proxy] Received message from client: " << clientMessage << std::endl;

        size_t tokenPos = clientMessage.find("Token: ");
        size_t messagePos = clientMessage.find("Message: ");
        if (tokenPos == std::string::npos || messagePos == std::string::npos || tokenPos >= messagePos) {
            std::cerr << "[Proxy] Invalid message format from client." << std::endl;
            proxyServer.Send(clientSocket, "ERROR: Invalid message format.");
            continue;
        }

        std::string receivedToken = trim(clientMessage.substr(tokenPos + 7, messagePos - (tokenPos + 7)));
        std::string userMessage = clientMessage.substr(messagePos + 9);

        // Debugging tokens
        std::cout << "[Debug] Received token: [" << receivedToken << "]" << std::endl;
        std::cout << "[Debug] Expected token: [" << token << "]" << std::endl;

        if (receivedToken != token) {
            std::cerr << "[Proxy] Token mismatch." << std::endl;
            proxyServer.Send(clientSocket, "ERROR: Invalid token.");
            continue;
        }

        if (!verifyTokenWithAuthServer(proxyAuthyClient, receivedToken)) {
            std::cerr << "[Proxy] Token verification failed." << std::endl;
            proxyServer.Send(clientSocket, "ERROR: Invalid token.");
            continue;
        }

        std::string serverResponse;
        if (!forwardMessageToServer(proxyClient, userMessage, serverResponse)) {
            proxyServer.Send(clientSocket, "ERROR: Server unavailable.");
            break;
        }

        if (!proxyServer.Send(clientSocket, serverResponse)) {
            std::cerr << "[Proxy] Failed to send server response back to client." << std::endl;
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

    // Initialize the proxy server
    CTCPSSLServer proxyServer(logger, clientPort);
    proxyServer.SetSSLCertFile("/home/shyskov/Documents/libs/sem6/kursach/cert/certs/proxy.crt");
    proxyServer.SetSSLKeyFile("/home/shyskov/Documents/libs/sem6/kursach/cert/certs/proxy.key");
    proxyServer.SetSSLCerthAuth("/home/shyskov/Documents/libs/sem6/kursach/cert/certs/root.crt");

    // Vector to store threads for handling multiple clients
    std::vector<std::thread> clientThreads;

    while (true) {
        ASecureSocket::SSLSocket clientSocket;

        // Listen for a new client connection
        if (proxyServer.Listen(clientSocket)) {
            std::cout << "[Proxy] New client connected to proxy on port " << clientPort << std::endl;

            // Create a new thread for each client connection
            clientThreads.emplace_back(std::thread(
                [](CTCPSSLServer& proxyServer, ASecureSocket::SSLSocket clientSocket, const std::string& authServerAddress, const std::string& authServerPort, const std::string& serverAddress, const std::string& serverPort) {
                    auto logger = [](const std::string& message) {
                        std::cout << message << std::endl;
                    };

                    // Initialize the auth server client
                    CTCPSSLClient proxyAuthyClient(logger);
                    proxyAuthyClient.SetSSLCertFile("/home/shyskov/Documents/libs/sem6/kursach/cert/certs/proxy.crt");
                    proxyAuthyClient.SetSSLKeyFile("/home/shyskov/Documents/libs/sem6/kursach/cert/certs/proxy.key");
                    proxyAuthyClient.SetSSLCerthAuth("/home/shyskov/Documents/libs/sem6/kursach/cert/certs/root.crt");

                    // Initialize the main server client
                    CTCPSSLClient proxyClient(logger);
                    proxyClient.SetSSLCertFile("/home/shyskov/Documents/libs/sem6/kursach/cert/certs/proxy.crt");
                    proxyClient.SetSSLKeyFile("/home/shyskov/Documents/libs/sem6/kursach/cert/certs/proxy.key");
                    proxyClient.SetSSLCerthAuth("/home/shyskov/Documents/libs/sem6/kursach/cert/certs/root.crt");

                    if (proxyAuthyClient.Connect(authServerAddress, authServerPort)) {
                        std::cout << "[Proxy] Connected to auth_server at " << authServerAddress << ":" << authServerPort << std::endl;

                        std::string token;
                        if (authenticateWithAuthServer(proxyServer, clientSocket, proxyAuthyClient, token)) {
                            if (proxyClient.Connect(serverAddress, serverPort)) {
                                std::cout << "[Proxy] Connected to server at " << serverAddress << ":" << serverPort << std::endl;
                                handleClientRequests(proxyServer, clientSocket, proxyAuthyClient, proxyClient, token);
                            } else {
                                std::cerr << "[Proxy] Failed to connect to server." << std::endl;
                            }
                        }
                    } else {
                        std::cerr << "[Proxy] Failed to connect to auth_server." << std::endl;
                    }

                    proxyServer.Disconnect(clientSocket);
                    std::cout << "[Proxy] Client disconnected." << std::endl;
                },
                std::ref(proxyServer), std::move(clientSocket), authServerAddress, authServerPort, serverAddress, serverPort));
        } else {
            std::cerr << "[Proxy] Failed to accept client connection." << std::endl;
        }
    }

    // Join all threads before exiting (optional, if the server is stopped)
    for (auto& thread : clientThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    return 0;
}