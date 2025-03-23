#include "./tcp_server/tcp_ssl_server.h"
#include <iostream>
#include <string>

int main() {
    auto logger = [](const std::string& message) {
        std::cout << message << std::endl;
    };

    std::string port = "8080";
    CTCPSSLServer server(logger, port);

    // Set certificate paths
    server.SetSSLCertFile("/home/shyskov/Documents/libs/sem6/kursach/cert/certs/server.crt");       // Server certificate
    server.SetSSLKeyFile("/home/shyskov/Documents/libs/sem6/kursach/cert/certs/server.key");        // Server private key
    server.SetSSLCerthAuth("/home/shyskov/Documents/libs/sem6/kursach/cert/certs/root.crt");      // Trusted CA certificate

    ASecureSocket::SSLSocket clientSocket;

    if (server.Listen(clientSocket)) {
        std::cout << "SSL Server is running on port " << port << std::endl;

        while (true) {
            char buffer[256];
            int bytesReceived = server.Receive(clientSocket, buffer, sizeof(buffer), false);
            if (bytesReceived > 0) {
                std::string receivedMessage(buffer, bytesReceived);
                std::cout << "Received: " << receivedMessage << std::endl;

                // Respond to the client
                std::string response = "Server received: " + receivedMessage;
                if (!server.Send(clientSocket, response)) {
                    std::cerr << "Failed to send response to client." << std::endl;
                    break; // Exit the loop if sending fails
                }
            } else {
                std::cerr << "Client disconnected or error occurred." << std::endl;
                break; // Exit the loop if receiving fails
            }
        }

        server.Disconnect(clientSocket);
        std::cout << "Connection closed." << std::endl;
    } else {
        std::cerr << "Failed to start the SSL server." << std::endl;
    }

    return 0;
}