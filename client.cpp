#include "./tcp_client/tcp_ssl_client.h"
#include <iostream>
#include <string>

int main() {
    std::string serverAddress = "127.0.0.1"; // Server address
    std::string port = "9090";              // Server port

    // Define a logger function
    auto logger = [](const std::string& message) {
        std::cout << message << std::endl;
    };

    // Create the SSL client
    CTCPSSLClient client(logger);

    // Set certificate paths
    client.SetSSLCertFile("/home/shyskov/Documents/libs/sem6/kursach/cert/certs/client.crt");       // Client certificate
    client.SetSSLKeyFile("/home/shyskov/Documents/libs/sem6/kursach/cert/certs/client.key");        // Client private key
    client.SetSSLCerthAuth("/home/shyskov/Documents/libs/sem6/kursach/cert/certs/root.crt");      // Trusted CA certificate

    // Connect to the SSL server
    if (client.Connect(serverAddress, port)) {
        std::cout << "Connected to SSL server at " << serverAddress << ":" << port << std::endl;

        while (true) {
            // Prompt the user for input
            std::cout << "Enter a message to send to the server (or type 'exit' to quit): ";
            std::string userInput;
            std::getline(std::cin, userInput);

            // Exit the loop if the user types "exit"
            if (userInput == "exit") {
                std::cout << "Exiting client..." << std::endl;
                break;
            }

            // Create a JWT-like token with a payload
            std::string header = "{\"alg\":\"HS256\",\"typ\":\"JWT\"}";
            std::string payload = "{\"user\":\"example_user\",\"role\":\"admin\"}";
            std::string signature = "example_signature"; // Normally, this would be generated using a secret key

            // Base64 encode the header, payload, and signature (for simplicity, no actual Base64 encoding here)
            std::string token = header + "." + payload + "." + signature;

            // Send the token and user input as part of the message
            std::string message = "Token: " + token + "\nMessage: " + userInput;
            if (client.Send(message)) {
                std::cout << "Message sent to server: " << message << std::endl;
            } else {
                std::cerr << "Failed to send message to server." << std::endl;
                break; // Exit the loop if sending fails
            }

            // Receive a response from the server
            char buffer[1024] = {0};
            int bytesReceived = client.Receive(buffer, sizeof(buffer), false);
            if (bytesReceived > 0) {
                std::cout << "Server says: " << std::string(buffer, bytesReceived) << std::endl;
            } else {
                std::cerr << "Failed to receive message from server." << std::endl;
                break; // Exit the loop if receiving fails
            }
        }

        // Disconnect from the server
        client.Disconnect();
    } else {
        std::cerr << "Failed to connect to SSL server." << std::endl;
    }

    return 0;
}