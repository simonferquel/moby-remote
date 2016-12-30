#include <server.h>
#include <string>
using namespace mobyremote;
std::shared_ptr<Server> _currentServer;
int main(int argc, char** argv) {
	std::string address = "0.0.0.0";
	int port = 56987;
	if (argc > 2) {
		address = argv[1];
		port = atoi(argv[2]);
	}
	Listener l(address.c_str(), port);
	l.StartAcceptLoop([](std::unique_ptr<Connection> c) {
		if (_currentServer) {
			_currentServer->Stop();
		}
		_currentServer = Server::make(std::move(c), std::make_shared<MobyServerHandler>());
	});
	return 0;
}