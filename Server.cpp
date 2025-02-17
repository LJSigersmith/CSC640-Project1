#include "Server.h"
#include <sys/socket.h>
#include <iostream>
#include <unistd.h>
//#include <arpa/inet.h>
#include <netinet/in.h>
#include "Message.cpp"

int Server::getServerSocket() { return _serverSocket; }
int Server::getPort() { return _PORT; }
int Server::getAddr() { return _ADDR; }
int Server::getipProtocol() { return _IPPROTOCOL; }

Server::Server(int PORT) {

    _setupServer(AF_INET, SOCK_STREAM, PORT);
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

void Server::_rcvData(string data) {

    //cout << "[SERVER]  Recieved data: " << data << endl;

    // Find data type (first [])
    // string dataType;
    // size_t start = data.find('[');
    // size_t end = data.find(']');

    
    // if (start != string::npos && end != string::npos && start < end) {
    //      dataType = data.substr(start+1, end - start - 1);
    //  }
    // auto it = msgTypeMap.find(dataType);
    // MessageType msgType = (it != msgTypeMap.end()) ? it->second : MessageType::UNKNOWN;

    // switch (msgType)
    // {
    //  case MessageType::PULSE:
    //      cout << "[SERVER]  {PULSE} " << endl;
    //      break;
    // // case MessageType::COMMAND:
    // //     break;
    //  default:
    //      cout << "[SERVER] {UNKNOWN} " << endl;
    //  }

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

        // Accept Client Connections
        int clientSocket = accept(_serverSocket, nullptr, nullptr);
        if (clientSocket < 0) {
            cout << "[SERVER]  Accepting client connections failed" << endl;
            //close(_serverSocket);
        } else {
            cout << "[SERVER]  Accepted client connections" << endl;
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

                // Debugging: Print received message
                struct in_addr srcAddr;
                srcAddr.s_addr = static_cast<in_addr_t>(receivedMsg.sourceAddress);
                struct in_addr destAddr;
                destAddr.s_addr = static_cast<in_addr_t>(receivedMsg.destinationAddress);

                cout << "[SERVER] Received Message:\n"
                    << "  Type: " << receivedMsg.messageTypeToString(receivedMsg.type) << "\n"
                    << "  Source: " << inet_ntoa({srcAddr}) << "\n"
                    << "  Destination: " << inet_ntoa({destAddr}) << "\n"
                    << "  Content: " << receivedMsg.content << "\n";

                // Call your function to handle received data
                _rcvData(receivedMsg.content);
            }
            close(clientSocket);
        });
        clientThread.detach();
    }
}