#include "Server.h"
#include "Client.h"
#include <iostream>
#include <cstring>
#include <fstream>
#include <iostream>
#include <json/json.h>

std::string SERVER_IP;
int SERVER_PORT;

void loadConfig() {
    std::ifstream configFile("config.json");
    if (!configFile) {
        std::cout << "Error: could not open config.json" << std::endl;
        exit(1);
    }

    Json::Value config;
    configFile >> config;

    SERVER_IP = config["server_ip"].asString();
    SERVER_PORT = config["server_port"].asInt();

    std::cout << "[SERVER]  Loaded Config: Server IP = " << SERVER_IP << ", Port = " << SERVER_PORT << std::endl;
}

void start_clients_and_server(int numClients, int startingPort) {
    
    using namespace std;

    Server server(startingPort);

    vector<Client*> clients;
    for (int i = 0; i < numClients; i++) {
        cout << "  [MAIN]  Making Client " << i << std::endl;
        Client* client = new Client(startingPort, i);
        client->connectToServer(&server);
        std::string initMsg = "Client " + std::to_string(i) + " connected";
        client->sendMessage(initMsg);
        client->sendHeartbeat(10);
    }
    
    //Client client(startingPort);

    //client.connectToServer(&server);
    //client.sendMessage("Initial Connection");
    //client.sendHeartbeat(5);

}

void startServer(int serverPort) {

    Server* server = new Server(serverPort);

}

void startClient(int serverPort) {

    Client* client = new Client(serverPort, serverPort);

    string addrStr = SERVER_IP;
    const char* addr = addrStr.c_str();
    client->connectToServerOnPort(addr, serverPort);
    std::string initMsg = "Client " + std::to_string(serverPort) + " connected";
    //client->sendMessage(initMsg);
    client->sendHeartbeat(5);
    client->waitForServerResponse();

    // Move this to a Client method
    bool inputStatus = true;
    while (true) {
        if (inputStatus) {
            cout << flush;
            cout << "(0) View Files" << endl;
            cout << "(1) Change Pulse Rate" << endl;
            cout << "(2) Quit Client" << endl;
            cout << ">> ";
            inputStatus = false;
        }
        string input;
        getline(cin, input);
        if (input == "0") {
            cout << "Viewing Files" << endl;
            this_thread::sleep_for(chrono::seconds(2));
            inputStatus = true;
        } else if (input == "1") {
            cout << "Changing Pulse Rate" << endl;
            this_thread::sleep_for(chrono::seconds(2));
            inputStatus = true;
        } else if (input == "2") {
            break;
        }

        this_thread::sleep_for(chrono::milliseconds(100));
    }

}

int main(int argc, char* argv[]) {

    loadConfig();

    char* opt = argv[1]; // 0 for server, 1 for client

    if (strcmp(opt, "0") == 0) {
        startServer(SERVER_PORT);
    } else {
        startClient(SERVER_PORT);
    }

    //while(1) {}

    return 0;
}