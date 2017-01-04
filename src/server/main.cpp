#include <server.h>
#include <string>
#include <thread>
using namespace mobyremote;
std::shared_ptr<Server> _currentServer;

class PortForwarderServer : public IPortForwarder {
private:
public:
	virtual void Handle(PortForwardingRequest request, std::function<void(PortForwardingResponse)> callback) override {
		auto server = _currentServer;
		if (!server) {
			PortForwardingResponse resp{ 1 };
			callback(resp);
			return;
		}
		server->GetCodec()->Request(MessageType::ExposePortRequest, Bufferize(request), [callback](Codec* codec, MessageType type, Buffer b) {
			auto resp = Unbufferize<PortForwardingResponse>(b.View());
			callback(*resp);
		});
	}
};

int main(int argc, char** argv) {
	std::string address = "0.0.0.0";
	int port = 56987;
	if (argc != 4) {
		std::cout << "usage: server <ip> <port> <port_forwarding_unix_socket>" << std::endl;
		return 1;
	}
	address = argv[1];
	port = atoi(argv[2]);
	auto unixSocketPath = argv[3];
	PortForwardingRequestListener ul(unixSocketPath, std::make_shared<PortForwarderServer>());
	auto pfT = std::thread([&ul]() {ul.Loop(); });
	Listener l(address.c_str(), port);
	l.StartAcceptLoop([](std::unique_ptr<Connection> c) {
		if (_currentServer) {
			_currentServer->Stop();
		}
		_currentServer = Server::make(std::move(c), std::make_shared<MobyServerHandler>());
	});
	ul.Stop();
	pfT.join();
	return 0;
}