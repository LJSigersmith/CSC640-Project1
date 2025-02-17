#include <thread>
#include <vector>
#include <string>
#include "Message.h"

using namespace std;
class Client;

class Server {

    public :
        int getServerSocket();
        int getPort();
        int getAddr();
        int getipProtocol();

        Server(int PORT);
        ~Server();

    private :
        int _serverSocket;
        int _PORT, _ADDR, _IPPROTOCOL;
        thread _serverThread;
        unordered_map<string, vector<string>> _clientData;

        void _processMessage(Message msg);
        void _processPulse(Message pulse);

        void _setupServer(int IPPROTOCOL, int STREAM, int PORT);
        void _startServer();

};