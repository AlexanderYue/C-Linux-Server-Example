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
#include <arpa/inet.h>
#include <sys/socket.h>

using namespace std;

#define PORT 6969

// Function to receive a file from the server
void getFile(int sock, const char* filename) {
    char buffer[1024] = {0};
    int bytesReceived = 0;

    // Receive file size
    bytesReceived = recv(sock, buffer, sizeof(buffer), 0);
    buffer[bytesReceived] = '\0';
    uintmax_t fileSize = stoull(buffer); // Convert received string to uintmax_t

    // Check if the file size is valid
    if (fileSize == 0) {
        cerr << "File " << filename << " is missing" << endl;
    }
    else {
        cout << "Received file " << filename << " (" << fileSize << " bytes)" << endl;
    }

    // Open file for writing
    ofstream file(filename, ios::binary);
    if (!file.is_open()) {
        cerr << "Error: Could not open file for writing" << endl;
        return;
    }
    recv(sock, buffer, sizeof(buffer), 0); //recieve file data
    file<<buffer<<endl; //write to file
    file.close();// Close file
}

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    const char* serverIP = "127.0.0.1"; // IP address of the server

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        cerr << "Socket creation error" << endl;
        return EXIT_FAILURE;
    }

    // Configure server address
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, serverIP, &serv_addr.sin_addr) <= 0) {
        cerr << "Invalid address/ Address not supported" << endl;
        return EXIT_FAILURE;
    }

    // Connect to server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        cerr << "Connection Failed" << endl;
        return EXIT_FAILURE;
    }

    string command;
    bool quit = false;

    // Main loop for sending commands to the server
    while (!quit) {
        // Prompt user for command
        cout << "Enter command (get filename, exit, or terminate): ";
        getline(cin, command);
        cout << command << endl;

        // Send command to server
        if (send(sock, command.c_str(), command.length(), 0) < 0) {
            cerr << "Send failed" << endl;
            break;
        }

        // Handle command
        if (command == "exit" || command == "terminate") {
            quit = true; //sets quit to true and ends the while loop
        } else if (command.substr(0, 4) == "get ") {
            char filename[1024] = {0}; //creates variable to hold filename
            strcpy(filename, command.substr(4).c_str());
            getFile(sock, filename); //calls function to get requested file
        } else {
            cerr << "Invalid command" << endl;
        }
    }

    // Close socket
    close(sock);
    return 0;
}
