#include "Client.h"
#include "Server.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <unistd.h>
#include <filesystem>
#include <fstream>
#include <json/json.h>
#include <curl/curl.h>

// ChatGPT wrote this
string getPublicIPaddress() {
    CURL* curl;
    CURLcode res;
    string publicIP;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "http://checkip.amazonaws.com");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, +[](void* ptr, size_t size, size_t nmemb, string* data) {
            data->append((char*)ptr, size * nmemb);
            return size * nmemb;
        });
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &publicIP);

        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }

    publicIP.erase(remove(publicIP.begin(), publicIP.end(), '\n'), publicIP.end());

    return publicIP;
}

int Client::getClientSocket() {
    return _clientSocket;
}

//struct sockaddr_in Client::getClientIP() {
//    return _clientIP;
//}

//thread Client::getClientThread() {
//    return std::move(_clientThread);
//}

Client::Client(int PORT, int ID) {
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

    // Get Client IP and set (could be local addr if set before socket created)
    //struct sockaddr_in clientAddr;
    //socklen_t clientAddrLen = sizeof(clientAddr);
    //getsockname(_clientSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
    //_clientIP = clientAddr;

    _publicIP = getPublicIPaddress();
    cout << "[CLIENT]  Public IP: " << _publicIP << endl;

    loadConfig();

}

void Client::sendMessage(string messageStr) {
    Message msg;
    msg.type = MessageType::MESSAGE;
    msg.content = messageStr;
    msg.size = messageStr.size();

    //Convert public IP (string) to in_addr
    struct in_addr addr;
    inet_pton(AF_INET, _publicIP.c_str(), &addr);
    msg.sourceAddress = addr.s_addr;

    thread th(&Client::_sendMessage, this, msg);
    th.detach();
}
void Client::sendHeartbeat(int intervalSeconds) {
    _heartbeatThread = thread(&Client::_sendHeartbeat, this, intervalSeconds);
    _heartbeatThread.detach();
}

void Client::_sendMessage(Message msg) {
    if (_clientSocket <= 0) { cerr << "[CLIENT]  Socket is invalid, cannot send message" << endl; }

    //cout << "[CLIENT]  Attempting to send message" << endl;

    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    getsockname(_clientSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
    
    msg.sourceAddress = clientAddr.sin_addr.s_addr;
    char ipStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clientAddr.sin_addr, ipStr, INET_ADDRSTRLEN);
    cout << "[CLIENT] Sending message from IP: " << ipStr << endl;

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

        // Send pulse every intervalSeconds
        auto now = chrono::system_clock::now();
        chrono::duration<double> elapsed = now - t;
        auto pulseRate = chrono::seconds(intervalSeconds);

        // Build pulse
        Message pulse;
        string fileList = "";
        getFileList(fileList);
        pulse.content = fileList;
        pulse.type = MessageType::PULSE;
        if (elapsed >= pulseRate) { _sendMessage(pulse); t = chrono::system_clock::now(); }
        // ChatGPT suggested this:
        this_thread::sleep_for(chrono::milliseconds(100)); // Prevent high CPU usage
    }
}

void Client::waitForServerResponse() {
    _serverMonitoring = thread(&Client::_handleServerResponse, this);
    _serverMonitoring.detach();
}

void Client::_handleServerResponse() {
    while (true) {
        char buffer[1024] = {0};
        int bytesRecvd = recv(_clientSocket, buffer, sizeof(buffer), 0);

        if (bytesRecvd <= 0) {
            cout << "[CLIENT] Couldn't recv from server" << endl;
            break;
        }

        Message response = Message::deserialize(buffer, sizeof(buffer));
        if (response.type == MessageType::ACKNOWLEDGE) {
            cout << "[CLIENT]  Recieved acknowledgement from server" << endl;
        }

        this_thread::sleep_for(chrono::milliseconds(100));
    }
}

void Client::loadConfig() {

    std::ifstream configFile("config.json");
    if (!configFile) {
        std::cout << "Error: could not open config.json" << endl;
    }

    Json::Value config;
    configFile >> config;

    _homeDirectory = config["home_directory"].asString();

}
void Client::getFileList(string& fileList) {
    using namespace filesystem;

    if (!exists(_homeDirectory)) {
        fileList = "[DIR-DNE]"; // DIR Does not exist
        return; 
    }

    if (!is_directory(_homeDirectory)) {
        fileList = "[DIR-IND]"; // DIR is not dir
        return;
    }

    if (exists(_homeDirectory) && is_directory(_homeDirectory)) {

        for (const auto& entry : directory_iterator(_homeDirectory)) {
            fileList += "[" + entry.path().filename().string() + "]";
        }
    }
}