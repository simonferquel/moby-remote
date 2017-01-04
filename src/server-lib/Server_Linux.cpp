
#include <server.h>

using namespace mobyremote;
using namespace std;
#ifdef _WIN32
void bindUnixSocket(SOCKET s, const char* path) {
	throw TransportErrorException{ TransportError::BindFailed };
}
#else
#include <sys/un.h>
#include <cstring>

void bindUnixSocket(SOCKET s, const char* path) {
	sockaddr_un addr;
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, path);
	unlink(addr.sun_path);
	auto len = strlen(addr.sun_path) + sizeof(addr.sun_family);
	if (0 != bind(s, (sockaddr *)&addr, len)) {
		throw TransportErrorException{ TransportError::BindFailed };
	}
}
#endif
mobyremote::PortForwardingRequestListener::PortForwardingRequestListener(const char * path, const std::shared_ptr<IPortForwarder>& forwarder) : _s(socket(AF_UNIX, SOCK_STREAM, 0)), _forwarder(forwarder)
{
	bindUnixSocket(_s.Get(), path);
	if (0 != listen(_s.Get(), SOMAXCONN)) {
		throw TransportErrorException{ TransportError::ListenFailed };
	}
}

void mobyremote::PortForwardingRequestListener::Loop()
{
	while(_s.Get() != INVALID_SOCKET){
		auto rawSock = accept(_s.Get(), nullptr, nullptr);
		if (INVALID_SOCKET == rawSock) {
			continue;
		}
		auto clientSock = make_shared<SafeSocket>(rawSock);
		PortForwardingRequest req;
		if (sizeof(req) != recv(clientSock->Get(), reinterpret_cast<char*>(&req), sizeof(req), MSG_WAITALL)) {
			continue;
		}
		_forwarder->Handle(req, [cliSock = std::move(clientSock)](PortForwardingResponse resp){
			send(cliSock->Get(), reinterpret_cast<char*>(&resp), sizeof(resp), 0);
		});
	}
}

void mobyremote::PortForwardingRequestListener::Stop()
{
	_s.Close();
}