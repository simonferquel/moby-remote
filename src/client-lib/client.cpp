#include <client.h>
using namespace mobyremote;

void DispatchMobyRemoteClientEvent(std::weak_ptr<IClientHandler> serverHandler, Codec* codec, MessageType messageType, Buffer& b) {
	auto handler = serverHandler.lock();
	if (!handler) {
		return;
	}
}
void DispatchMobyRemoteClientRequest(std::weak_ptr<IClientHandler> serverHandler, Codec* codec, MessageType messageType, std::uint32_t correlationId, Buffer& b) {
	auto handler = serverHandler.lock();
	if (!handler) {
		return;
	}
}
std::shared_ptr<Client> mobyremote::Client::make(std::unique_ptr<Connection>&& conn, const std::shared_ptr<IClientHandler>& handler)
{
	std::weak_ptr<IClientHandler> weakHandler = handler;
	std::shared_ptr<Client> client(new Client(std::make_unique<Codec>(std::move(conn), [weakHandler](Codec* c, MessageType type, Buffer buffer) {
		DispatchMobyRemoteClientEvent(weakHandler, c, type, buffer);
	}, [weakHandler](Codec* c, MessageType type, std::uint32_t correlationId, Buffer b) {
		DispatchMobyRemoteClientRequest(weakHandler, c, type, correlationId, b);
	}), handler));
	startThread([client]() {
		client->GetCodec()->ReceiveMessagesUntilClosed();
	});
	return client;
}
void mobyremote::Client::ReplaceFileContent(const std::string& fileName, Buffer&& b, std::function<void(bool)> callback)
{
	ReplaceFileRequest request(fileName, std::move(b));
	request.Send(_codec.get(), callback);
}