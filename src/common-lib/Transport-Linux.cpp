#ifndef _WIN32
#include <common.h>
#include "compat.h"
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
using namespace mobyremote;

void mobyremote::init_transport() {
}

class WindowsResolvedAddress : public ResolvedAddress {
private:
	addrinfo* _info;
public:
	WindowsResolvedAddress(addrinfo* info) :_info(info) {}
	virtual ~WindowsResolvedAddress() {
		freeaddrinfo(_info);
	}

	virtual const sockaddr* SockAddr()const override {
		return _info->ai_addr;
	}
	virtual int SockAddrLen() const  override {
		return (int)_info->ai_addrlen;
	}
};

std::unique_ptr<ResolvedAddress> mobyremote::Resolve(const char * hostName, int port)
{
	auto sPort = std::to_string(port);
	addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	addrinfo* results = nullptr;

	if (getaddrinfo(hostName, sPort.c_str(), &hints, &results) != 0) {
		throw TransportErrorException{ TransportError::NameResolutionFailed };
	}
	if (results == nullptr) {
		throw TransportErrorException{ TransportError::NameResolutionFailed };
	}
	return std::make_unique<WindowsResolvedAddress>(results);
}
#endif