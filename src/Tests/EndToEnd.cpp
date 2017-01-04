#include <client.h>
#include <server.h>
#include <gtest/gtest.h>
#include <thread>
#include <future>
#include <string>
#include <iostream>
#include <chrono>
using namespace mobyremote;
using namespace std;
using namespace std::chrono;

class TestEnv {
private:
	std::shared_ptr<Client> _client;
	std::shared_ptr<Server> _server;
	std::unique_ptr<Connection> _clientConn, _serverConn;
public:
	TestEnv() {
		Listener l("127.0.0.1", 56987);
		std::thread([&]() {
			try {
				l.StartAcceptLoop([&](std::unique_ptr<Connection> c) {
					_serverConn = std::move(c);
				});
			}
			catch (...)
			{

			}
		}).detach();
		this_thread::sleep_for(100ms);
		_clientConn = ConnectTo("127.0.0.1", 56987);
		while (!_serverConn) {
			this_thread::sleep_for(10ms);
		}
	}

	void SetupClient(const std::shared_ptr<IClientHandler>& clientHandler) {
		_client = Client::make(std::move(_clientConn), clientHandler);
	}

	void SetupServer(const std::shared_ptr<IServerHandler>& serverHandler) {
		_server = Server::make(std::move(_serverConn), serverHandler);
	}


	void Stop() {
		_client->Stop();
		_server->Stop();
		this_thread::sleep_for(200ms);
	}

	Client* client() { return _client.get(); }
	Server* server() { return _server.get(); }
};


TEST(EndToEnd, ReplaceFile) {
	class ClientHandler : public IClientHandler {
	public:
		virtual PortForwardingResponse OnExposePortAction(PortForwardingRequest req)override {
			return PortForwardingResponse::Ok();
		}
	};
	class Serverhandler : public IServerHandler {
	private:
		ReplaceFileRequest _request;
	public:

		virtual bool OnReplaceFileRequest(ReplaceFileRequest request) override {
			_request = std::move(request);
			return true;
		}
		ReplaceFileRequest& Request() { return _request; }
	};

	auto serverHandler = std::make_shared<Serverhandler>();
	TestEnv env;
	env.SetupClient(std::make_shared<ClientHandler>());
	env.SetupServer(serverHandler);
	std::promise<bool> promise;
	std::string testData = "Hello World !";
	auto testBuffer = Buffer(Bufferize(testData));
	env.client()->ReplaceFileContent("/etc/test", testBuffer.Clone(), [&promise](bool result) {
		promise.set_value(result);
	});
	auto res = promise.get_future().get();
	ASSERT_TRUE(res);
	ASSERT_EQ(testBuffer.View().size(), serverHandler->Request().content().View().size());
	ASSERT_EQ(serverHandler->Request().fileName(), "/etc/test");
	ASSERT_TRUE(std::equal(testBuffer.View().begin(), testBuffer.View().end(), serverHandler->Request().content().View().begin(), serverHandler->Request().content().View().end()));
	env.Stop();

}


TEST(EndToEnd, ReplaceFile2) {
	class ClientHandler : public IClientHandler {
	public:
		virtual PortForwardingResponse OnExposePortAction(PortForwardingRequest req)override {
			return PortForwardingResponse::Ok();
		}
	};
	class Serverhandler : public IServerHandler {
	private:
		ReplaceFileRequest _request;
	public:

		virtual bool OnReplaceFileRequest(ReplaceFileRequest request) override {
			_request = std::move(request);
			return false;
		}
		ReplaceFileRequest& Request() { return _request; }
	};

	auto serverHandler = std::make_shared<Serverhandler>();
	TestEnv env;
	env.SetupClient(std::make_shared<ClientHandler>());
	env.SetupServer(serverHandler);
	std::promise<bool> promise;
	std::string testData = "Hello World !";
	auto testBuffer = Buffer(Bufferize(testData));
	env.client()->ReplaceFileContent("/etc/test2", testBuffer.Clone(), [&promise](bool result) {
		promise.set_value(result);
	});
	auto res = promise.get_future().get();
	ASSERT_FALSE(res);
	ASSERT_EQ(testBuffer.View().size(), serverHandler->Request().content().View().size());
	ASSERT_EQ(serverHandler->Request().fileName(), "/etc/test2");
	ASSERT_TRUE(std::equal(testBuffer.View().begin(), testBuffer.View().end(), serverHandler->Request().content().View().begin(), serverHandler->Request().content().View().end()));
	env.Stop();

}

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}