#include <thread>
#include <vector>
#include <string>

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
        vector<Client*> _activeClients;

        void _rcvData(string data);
        void _setupServer(int IPPROTOCOL, int STREAM, int PORT);
        void _startServer();

};