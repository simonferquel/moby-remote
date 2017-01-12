#include <common.h>
#include <iostream>
#include <string>
#include <future>
#include <thread>
#include <vector>

using namespace std;

std::unique_ptr<mobyremote::Connection> GetConnectionConnMode(const string& address, int port) {
	auto addr = mobyremote::Resolve(address.c_str(), port);
	return mobyremote::ConnectTo(*addr);
}

std::unique_ptr<mobyremote::Connection> GetConnectionListenMode(const string& address, int port) {
	std::promise<std::unique_ptr<mobyremote::Connection>> p;
	mobyremote::Listener l(address.c_str(), port);
	std::thread([&l, &p]() {
		try {
			l.StartAcceptLoop([&p](std::unique_ptr<mobyremote::Connection> c) {
				p.set_value(std::move(c));
			});
		}
		catch (...) {}
	}).detach();
	return p.get_future().get();
}

int main(int argc, char** argv) {
	const char* usage = "usage: randomrecv <conn|listen> <ip address> <port> <packet size in bytes> <packet count>";
	if (argc != 6) {
		cerr << usage << endl;
		return -1;
	}
	string connMode = argv[1];
	string ip = argv[2];
	auto port = atoi(argv[3]);
	auto packetSize = atoi(argv[4]);
	auto packetCount = atoi(argv[5]);
	std::unique_ptr<mobyremote::Connection> conn;
	if (connMode == "conn") {
		conn = GetConnectionConnMode(ip, port);
	}
	else if (connMode == "listen") {
		conn = GetConnectionListenMode(ip, port);
	}
	else {
		cerr << "unknown connection mode " << connMode << endl;
		return -2;
	}

	std::vector<char> payload;
	for (auto ix = 0; ix < packetSize; ++ix) {
		payload.push_back((char)ix);
	}

	mobyremote::BufferView bv(&payload[0], packetSize);
	for (auto ix = 0; ix < packetCount; ++ix) {
		conn->Receive(bv);
		cout << (ix + 1) << " / " << packetCount << " packets received" << endl;
	}
	return 0;
}