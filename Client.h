#include <thread>
#include "Message.cpp"

using namespace std;
class Server;

class Client {

    public :
        int getClientSocket();
        int getClientID();
        //thread getClientThread();

        Client(int PORT, int ID);
        ~Client();

        void connectToServer(Server* server);
        void connectToServerOnPort(const char* address, int port);
        void sendMessage(string messageStr);
        void sendHeartbeat(int intervalSeconds);

    private :
        int _clientID;
        int _clientSocket, _IPPROTOCOL, _PORT;
        
        thread _clientThread;
        thread _heartbeatThread;

        void _setupClient(int IPPROTOCOL, int STREAM, int PORT);
        void _sendMessage(Message msg);
        void _sendHeartbeat(int intervalSeconds);

};