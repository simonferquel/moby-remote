#include <server.h>
#include <fstream>
using namespace mobyremote;
void DispatchMobyRemoteServerEvent(std::weak_ptr<IServerHandler> serverHandler, Codec* codec, MessageType messageType, Buffer& b) {
	auto handler = serverHandler.lock();
	if (!handler) {
		return;
	}
}
void DispatchMobyRemoteServerRequest(std::weak_ptr<IServerHandler> serverHandler, Codec* codec, MessageType messageType,std::uint32_t correlationId, Buffer& b) {
	auto handler = serverHandler.lock();
	if (!handler) {
		codec->Response(correlationId, MessageType::NotImplemented, BufferView(nullptr, 0));
		return;
	}
	switch (messageType)
	{
	case MessageType::ReplaceFileRequest:
	{
		bool result = handler->OnReplaceFileRequest(ReplaceFileRequest(b.View()));
		codec->Response(correlationId, MessageType::ReplaceFileResponse, Bufferize(result));
	}
	break;
	default:
		codec->Response(correlationId, MessageType::NotImplemented, BufferView(nullptr, 0));
		break;
	}
}
std::shared_ptr<Server> mobyremote::Server::make(std::unique_ptr<Connection>&& conn, const std::shared_ptr<IServerHandler>& handler)
{
	std::weak_ptr<IServerHandler> weakHandler = handler;
	std::shared_ptr<Server> server (new Server (std::make_unique<Codec>(std::move(conn), [weakHandler](Codec* c, MessageType type, Buffer buffer) {
		DispatchMobyRemoteServerEvent(weakHandler, c, type, buffer);
	}, [weakHandler](Codec* c, MessageType type, std::uint32_t correlationId, Buffer b) {
		DispatchMobyRemoteServerRequest(weakHandler, c, type, correlationId, b);
	}), handler));
	startThread([server]() {
		server->GetCodec()->ReceiveMessagesUntilClosed();
	});
	return server;
}




bool mobyremote::MobyServerHandler::OnReplaceFileRequest(ReplaceFileRequest request)
{
	try {
		std::ofstream o(request.fileName(), std::ofstream::binary | std::ofstream::trunc);
		auto bufView = request.content().View();	
		o.write(bufView.begin(), bufView.size());
		o.flush();
		o.close();
		return true;
	}
	catch (...) {
		return false;
	}
}