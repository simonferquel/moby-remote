#pragma once
#include <memory>
namespace mobyremote {
	void init_transport();
	class ResolvedAddress {
	public:
		virtual ~ResolvedAddress() {}

		virtual const sockaddr* SockAddr()const = 0;
		virtual int SockAddrLen() const = 0;
	};
	std::unique_ptr<ResolvedAddress> Resolve(const char* hostName, int port);
}