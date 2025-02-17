#include <thread>
#include <filesystem>
#include "Message.h"

using namespace std;
class Server;

class Client {

    public :
        int getClientSocket();
        struct sockaddr_in getClientIP();
        //thread getClientThread();

        Client(int PORT, int ID);
        ~Client();

        void connectToServer(Server* server);
        void connectToServerOnPort(const char* address, int port);
        void sendMessage(string messageStr);
        void sendHeartbeat(int intervalSeconds);
        void loadConfig();

        void getFileList(string& fileList);

    private :
        struct sockaddr_in _clientIP;
        int _clientSocket, _IPPROTOCOL, _PORT;

        std::filesystem::path _homeDirectory;
        
        thread _clientThread;
        thread _heartbeatThread;

        void _setupClient(int IPPROTOCOL, int STREAM, int PORT);
        void _sendMessage(Message msg);
        void _sendHeartbeat(int intervalSeconds);

};