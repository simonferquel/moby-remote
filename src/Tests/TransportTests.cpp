#include <common.h>
#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <iostream>
using namespace mobyremote;
using namespace std;
using namespace std::chrono;
TEST(Transport, ConnectionShouldWork) {

	Listener l("127.0.0.1", 1011);
	bool acceptedConnection = false;
	std::thread([&l, &acceptedConnection]() {
		try {
			l.StartAcceptLoop([&acceptedConnection](std::unique_ptr<Connection> c) {
				acceptedConnection = true;
			});
		}
		catch (...)
		{

		}
	}).detach();
	this_thread::sleep_for(100ms);
	auto client = ConnectTo("127.0.0.1", 1011);
	this_thread::sleep_for(150ms);
	ASSERT_TRUE(acceptedConnection);

}


TEST(Transport, SendReceiveSmall) {

	Listener l("127.0.0.1", 1012);

	std::thread([&l]() {
		try {
			l.StartAcceptLoop([](std::unique_ptr<Connection> c) {
				int value = 42;
				auto buf = Bufferize(value);
				c->Send(buf);
			});
		}
		catch (...)
		{

		}
	}).detach();
	this_thread::sleep_for(100ms);
	auto client = ConnectTo("127.0.0.1", 1012);
	int result = 0;
	auto buf = Bufferize(result);
	client->Receive(buf);
	ASSERT_EQ(42, result);

}


TEST(Transport, SendReceiveLarge) {
	Listener l("127.0.0.1", 1013);
	std::unique_ptr<char[]> source(new char[1024 * 1024 * 5]);
	for (int i = 0; i < 1024 * 1024 * 5; ++i) {
		source[i] = (char)i;
	}
	std::unique_ptr<char[]> dest(new char[1024 * 1024 * 5]);
	std::thread([&l, &source]() {
		try {
			l.StartAcceptLoop([&source](std::unique_ptr<Connection> c) {
				c->Send(BufferView(source.get(), 1024 * 1024 * 5));
			});
		}
		catch (...)
		{

		}
	}).detach();
	this_thread::sleep_for(100ms);
	auto client = ConnectTo("127.0.0.1", 1013);
	BufferView buf(dest.get(), 1024 * 1024 * 5);
	client->Receive(buf);
	ASSERT_TRUE(std::equal(source.get(), source.get() + 1024 * 1024 * 5, dest.get(), dest.get() + 1024 * 1024 * 5));

}


int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}