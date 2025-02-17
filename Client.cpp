#include "Client.h"
#include "Server.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <unistd.h>

int Client::getClientSocket() {
    return _clientSocket;
}

int Client::getClientID() {
    return _clientID;
}

//thread Client::getClientThread() {
//    return std::move(_clientThread);
//}

Client::Client(int PORT, int ID) {
    _clientID = ID;
    _setupClient(AF_INET, SOCK_STREAM, PORT);
}
Client::~Client() {
    if (_clientSocket >= 0) {
        cout << "[CLIENT]  Closing client socket..." << endl;
        close(_clientSocket);
    }
}
void Client::connectToServerOnPort(const char* address, int port) {

    // Define Server Address
    int ADDR = inet_addr(address);

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = ADDR;

    // Connect to Server
    int conn = connect(_clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    if (conn < 0) {
        cout << "[CLIENT]  Connecting to server failed" << endl;
        perror("Connect failed");
        close(_clientSocket);
    } else { cout << "[CLIENT]  Connect to server success" << endl; }

}
void Client::connectToServer(Server* server) {

    // Define Server Address
    int ADDR = inet_addr("127.0.0.1");

    sockaddr_in serverAddress;
    serverAddress.sin_family = server->getipProtocol();
    serverAddress.sin_port = htons(server->getPort());
    serverAddress.sin_addr.s_addr = ADDR;

    // Connect to Server
    int conn = connect(_clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    if (conn < 0) {
        cout << "[CLIENT]  Connecting to server failed" << endl;
        perror("Connect failed");
        close(_clientSocket);
    } else { cout << "[CLIENT]  Connect to server success" << endl; }

}
void Client::sendMessage(string messageStr) {
    Message msg;
    msg.type = MessageType::MESSAGE;
    msg.content = messageStr;
    msg.size = messageStr.size();
    thread th(&Client::_sendMessage, this, msg);
    th.detach();
}
void Client::sendHeartbeat(int intervalSeconds) {
    _heartbeatThread = thread(&Client::_sendHeartbeat, this, intervalSeconds);
    _heartbeatThread.detach();
}
void Client::_setupClient(int IPPROTOCOL, int STREAM, int PORT) {
            
    // Most of this setup code is from: https://www.geeksforgeeks.org/socket-programming-in-cpp/

    // Create Client Socket
    int clientSocket = socket(IPPROTOCOL, STREAM, 0); // (IPv, TCP Type Socket, file descriptor)
    if (clientSocket < 0) {
        cout << "[CLIENT]  Error creating socket" << endl;
        return;
    } else { cout << "[CLIENT]  Socket created" << endl; }

    _clientSocket = clientSocket;
    _IPPROTOCOL = IPPROTOCOL;
    _PORT = PORT;

}

void Client::_sendMessage(Message msg) {
    if (_clientSocket <= 0) { cerr << "[CLIENT]  Socket is invalid, cannot send message" << endl; }

    //cout << "[CLIENT]  Attempting to send message" << endl;
    //const char* message = messageStr.c_str();

    size_t dataSize;
    char* serializedData = msg.serialize(dataSize);
    int sends = send(_clientSocket, serializedData, dataSize, 0);
    if (sends < 1) {
        cout << "[CLIENT]  Sending data to server failed" << endl;
        close(_clientSocket);
        return;
    } //else { cout << "[CLIENT]  Message sent" << endl; }

}
void Client::_sendHeartbeat(int intervalSeconds) {
    auto t = chrono::system_clock::now();
    while (1) {
        auto now = chrono::system_clock::now();
        chrono::duration<double> elapsed = now - t;
        auto pulseRate = chrono::seconds(intervalSeconds);
        //string pulseMsg = "[PULSE][" + to_string(_clientID) + "]";
        Message pulse;
        pulse.content = "IM ALIVE";
        pulse.type = MessageType::PULSE;
        if (elapsed >= pulseRate) { _sendMessage(pulse); t = chrono::system_clock::now(); }
        // ChatGPT suggested this:
        this_thread::sleep_for(chrono::milliseconds(100)); // Prevent high CPU usage
    }
}