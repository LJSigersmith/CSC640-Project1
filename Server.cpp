#include "Server.h"
#include <sys/socket.h>
#include <iostream>
#include <unistd.h>
//#include <arpa/inet.h>
#include <netinet/in.h>
#include "Message.h"

int Server::getServerSocket() { return _serverSocket; }
int Server::getPort() { return _PORT; }
int Server::getAddr() { return _ADDR; }
int Server::getipProtocol() { return _IPPROTOCOL; }

Server::Server(int PORT) {

    _setupServer(AF_INET, SOCK_STREAM, PORT);

    _clientMonitoringThread = thread(&Server::_monitorClients, this);
    _clientMonitoringThread.detach();

    _startServer();
    //_serverThread = thread(&Server::_startServer, this);
    //_serverThread.detach();
    
}
Server::~Server() {
    if (_serverSocket >= 0) {
        cout << "[SERVER]  Closing server socket..." << endl;
        //close(_serverSocket);
    }
}

void Server::_sendAcknowledgement(Message acknowledgeMsg) {
    
    struct in_addr destAddr;
    destAddr.s_addr = static_cast<in_addr_t>(acknowledgeMsg.destinationAddress);
    string clientIP = inet_ntoa(destAddr);

    if (_clientSockets.find(clientIP) != _clientSockets.end()) {
        int clientSocket = _clientSockets[clientIP];

        size_t dataSize;
        char* serializedData = acknowledgeMsg.serialize(dataSize);
        send(clientSocket, serializedData, dataSize, 0);

        cout << "[SERVER]  Sent acknowledgement" << endl;
    } else {
        cout << "[SERVER]  Client socket not found" << endl;
    }
}

void Server::_monitorClients() {

    cout << "[SERVER]  Monitoring Clients Active" << endl;

    while (true) {
        for (auto clientPair : _clientLastPulse) {

            string clientIP = clientPair.first;

            // If client is dead, don't check it
            if (!_clientStatus[clientIP]) {
                continue;
            }

            auto lastPulse = clientPair.second;
            auto now = chrono::steady_clock::now();
            auto duration = chrono::duration_cast<chrono::seconds>(now - lastPulse);
            
            if (duration > chrono::seconds(30)) {
                cout << "[SERVER]   Client " << clientIP << " inactive" << endl;
                _clientStatus[clientIP] = false;
                //_clientLastPulse.erase(clientIP);
            }
        }
        this_thread::sleep_for(chrono::seconds(1));
    }
}

void Server::_processPulse(Message pulse) {

    vector<string> fileList;
    size_t start = 0;
    size_t end;


    string pulseContent = pulse.content;
    // ChatGPT
    while ((end = pulseContent.find("[", start)) != string::npos) {
        size_t closeBracket = pulseContent.find("]", end);
        if (closeBracket != string::npos) {
            string filename = pulseContent.substr(end + 1, closeBracket - end - 1);
            fileList.push_back(filename);
        }
        start = closeBracket + 1;
    }

    // Convert source address to iP string
    struct in_addr srcAddr;
    srcAddr.s_addr = static_cast<in_addr_t>(pulse.sourceAddress);
    string clientIP = inet_ntoa(srcAddr);

    // Update Status/Time for Client
    _clientStatus[clientIP] = true;
    _clientLastPulse[clientIP] = chrono::steady_clock::now();

    // Update Filemap for Client
    _clientFilemaps[clientIP] = fileList;
    cout << "[SERVER]  Updated File Map for " << clientIP << endl;
    for (auto client : _clientFilemaps) {
        cout << "          " << client.first << endl;
        for (auto f : client.second) {
            cout << "          " << f << endl;
        }
    }

    // Send acknowledgement
    Message acknowledgementMsg;
    acknowledgementMsg.type = MessageType::ACKNOWLEDGE;
    acknowledgementMsg.content = "ACKNOWLEDGED";
    acknowledgementMsg.sourceAddress = inet_addr("server.ip");
    acknowledgementMsg.destinationAddress = pulse.sourceAddress;

    _sendAcknowledgement(acknowledgementMsg);
}

void Server::_processMessage(Message receivedMsg) {

        struct in_addr srcAddr;
        srcAddr.s_addr = static_cast<in_addr_t>(receivedMsg.sourceAddress);
        struct in_addr destAddr;
        destAddr.s_addr = static_cast<in_addr_t>(receivedMsg.destinationAddress);

        cout << "[SERVER] Received Message:\n"
            << "  Type: " << receivedMsg.messageTypeToString(receivedMsg.type) << "\n"
            << "  Source: " << inet_ntoa({srcAddr}) << "\n"
            << "  Destination: " << inet_ntoa({destAddr}) << "\n"
            << "  Content: " << receivedMsg.content << "\n";

        switch (receivedMsg.type)
        {
        case MessageType::PULSE:
            _processPulse(receivedMsg);
            break;
        
        default:
            break;
        }

}

void Server::_setupServer(int IPPROTOCOL, int STREAM, int PORT) {

    // Create socket
    int serverSocket = socket(IPPROTOCOL, STREAM, 0); // (IPv4, TCP Type Socket, file descriptor)
    if (serverSocket < 0) {
        cout << "[SERVER]  Failed to create socket" << endl;
        return;
    } else { cout << "[SERVER]  Socket created" <<  endl; }

    // Define Server Address
    int ADDR = INADDR_ANY; // Don't bind socket to specific IP, listen to all available IPs

    sockaddr_in serverAddress;
    serverAddress.sin_family = IPPROTOCOL;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = ADDR;

    // Bind Server Socket
    int bindResult = ::bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    if (bindResult < 0) {
        cout << "[SERVER]  Failed to bind server socket" << endl;
        close(serverSocket);           
    } else { cout << "[SERVER]  Server socket bound" << endl; }

    _serverSocket = serverSocket;
    _IPPROTOCOL = IPPROTOCOL;
    _PORT = PORT;
    _ADDR = ADDR;

    cout << "[SERVER]  Server setup complete" << endl;

}
void Server::_startServer() {
    while (1) {
        // Listen for connections
        int BACKLOG = 5; // # connections allowed to queue
        if (listen(_serverSocket, BACKLOG) < 0) {
            cout << "[SERVER]  Listening failed" << endl;
            //close(_serverSocket);
            break;
        } else {
            cout << "[SERVER]  Listening success" << endl;
        }

        sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        // Accept Client Connections
        int clientSocket = accept(_serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSocket < 0) {
            cout << "[SERVER]  Accepting client connections failed" << endl;
            //close(_serverSocket);
        } else {
            cout << "[SERVER]  Accepted client connections" << endl;

            char clientIP[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
            
            _clientSockets[clientIP] = clientSocket;
        }

        // ChatGPT
        thread clientThread([clientSocket, this]() {
            char buffer[1024] = {0};

            while (true) {

                // Step 1: Read the fixed-size part of the Message
                char headerBuffer[sizeof(MessageType) + sizeof(int) * 3]; // type + source + destination + contentSize
                int bytesReceived = recv(clientSocket, headerBuffer, sizeof(headerBuffer), 0);

                if (bytesReceived <= 0) {
                    cerr << "[SERVER] Client disconnected or error receiving data" << endl;
                    break;
                }

                // Step 2: Extract metadata (excluding content)
                const char* ptr = headerBuffer;
                MessageType type;
                int messageTypeInt;
                int sourceAddress, destinationAddress, contentSize;

                memcpy(&messageTypeInt, ptr, sizeof(int));
                ptr += sizeof(int);
                type = static_cast<MessageType>(messageTypeInt);
                memcpy(&sourceAddress, ptr, sizeof(int));
                ptr += sizeof(int);
                memcpy(&destinationAddress, ptr, sizeof(int));
                ptr += sizeof(int);
                memcpy(&contentSize, ptr, sizeof(int));

                // Step 3: Read the content separately
                string content;
                if (contentSize > 0) {
                    vector<char> contentBuffer(contentSize);
                    bytesReceived = recv(clientSocket, contentBuffer.data(), contentSize, 0);
                    if (bytesReceived <= 0) {
                        cerr << "[SERVER] Error receiving message content" << endl;
                        break;
                    }
                    content.assign(contentBuffer.begin(), contentBuffer.end());
                }

                // Step 4: Reconstruct Message
                Message receivedMsg;
                receivedMsg.type = type;
                receivedMsg.sourceAddress = sourceAddress;
                receivedMsg.destinationAddress = destinationAddress;
                receivedMsg.content = content;
                receivedMsg.size = contentSize;
                _processMessage(receivedMsg);
            }
            close(clientSocket);
        });
        clientThread.detach();
    }
}