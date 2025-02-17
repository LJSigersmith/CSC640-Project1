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
        thread _clientMonitoringThread;

        unordered_map<string, vector<string>> _clientFilemaps;
        unordered_map<string, bool> _clientStatus;
        unordered_map<string, chrono::steady_clock::time_point> _clientLastPulse;
        unordered_map<string, int> _clientSockets;

        void _sendAcknowledgement(Message acknowledgeMsg);

        void _processMessage(Message msg);
        void _processPulse(Message pulse);

        void _monitorClients();

        void _setupServer(int IPPROTOCOL, int STREAM, int PORT);
        void _startServer();

};