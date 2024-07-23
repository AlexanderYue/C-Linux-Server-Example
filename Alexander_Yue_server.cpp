/*
Written by: Alexander Yue
Second Assignment
Cougarnet: AZYue
PID: 2079436
COSC 3360
Instructor: Professor Paris
The port is already defined here, and no need for user input. Defined port is 6969
*/





#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <filesystem>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

using namespace std;
namespace fs = filesystem;

#define PORT 6969

// Function to get the size of a file
uintmax_t getFileSize(const string& filename) {
    try {
        return fs::file_size("Repository/" + filename); // Get the file size using filesystem library
    } catch (const exception& ex) {
        //cerr << "Error: " << ex.what() << endl;
        return 0;
    }
}

// Function to send a file to the client
void sendFile(int newSocket, const char* filename) {
    char buffer[1024] = {0};
    // Construct the full path to the file inside the Repository folder
    string filePath = "Repository/";
    filePath += filename;

    // Open the file
    ifstream file(filePath, ios::binary);
    if (!file.is_open()) {
        cerr << "That file is missing! " << filename << endl;
        const char* errorMessage = "0";
        send(newSocket, errorMessage, 1, 0); // Send a single byte to indicate error
        return;
    }

    // Get the file size
    uintmax_t fileSize = getFileSize(filename);

    // Convert file size to string
    char fileSizeStr[32];
    snprintf(fileSizeStr, sizeof(fileSizeStr), "%ju", fileSize);

    // Send file size to the client
    send(newSocket, fileSizeStr, strlen(fileSizeStr), 0);

    if (fileSize != 0) {
        cout << "Sent: " << fileSize << " bytes." << endl;
    }

    // Read and send file contents
    while (!file.eof()) {
        file.read(buffer, sizeof(buffer));
        send(newSocket, buffer, file.gcount(), 0); // Send file contents
    }

    file.close(); // Close the file
}

int main() {
    int serverSocket, newSocket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[1024] = {0}; //creates the "buffer" which is the data that gets sent over the server

    // Create socket
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        cerr << "Socket creation error" << endl;
        return EXIT_FAILURE;
    }

    // Configure server address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket to the specified port
    if (bind(serverSocket, (struct sockaddr *)&address, sizeof(address)) < 0) {
        cerr << "Bind failed" << endl;
        return EXIT_FAILURE;
    }

    // Start listening for incoming connections
    if (listen(serverSocket, 3) < 0) {
        cerr << "Listen failed" << endl;
        return EXIT_FAILURE;
    }

    bool term = false; // Variable to track if the server should terminate

    // Main loop to accept incoming connections
    while (!term) {
        // Accept a new connection
        if ((newSocket = accept(serverSocket, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            cerr << "Accept failed" << endl;
            return EXIT_FAILURE;
        }

        // Inner loop to handle commands from the client
        while (true) {
            // Receive data from the client
            int valread = recv(newSocket, buffer, sizeof(buffer), 0);

            if (valread < 0) {
                cerr << "Error in receiving: " << buffer << endl;
                close(newSocket);
                break; // Exit the loop and terminate the server
            }

            buffer[valread] = '\0';
            cout << "Received command: " << buffer << endl;

            // Handle the received command
            if (strncmp(buffer, "terminate", 9) == 0) {
                cout << "Terminating server..." << endl;
                close(newSocket);
                term = true; // Set term to true to exit the outer loop
                break; // Exit the inner loop
            } else if (strncmp(buffer, "exit", 4) == 0) {
                cerr << "Client disconnected" << endl;
                break; // Don't close the socket, just exit the inner loop
            } else if (strncmp(buffer, "get ", 4) == 0) {
                const char* filename = buffer + 4;
                cout << "A client requested the file " << filename << endl;
                sendFile(newSocket, filename); // Send the requested file to the client
            } else {
                cerr << "Invalid request" << endl;
            }
        }
    }

    close(serverSocket); // Close the listening socket
    return 0;
}
