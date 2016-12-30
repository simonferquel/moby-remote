#include <protocol.h>
#include <mutex>
using namespace mobyremote;
struct mobyremote::Codec::MutexWrapper {
	std::mutex mut;
};
mobyremote::Codec::Codec(std::unique_ptr<Connection>&& conn, const EventHandler& eventHandler, const RequestHandler& requestHandler) : _mutex(std::make_unique<MutexWrapper>()), _connection(std::move(conn)), _lastCorrelationId(0), _eventHandler(eventHandler), _requestHandler(requestHandler)
{
}
mobyremote::Codec::~Codec()
{
	Close();
}
void mobyremote::Codec::Event(MessageType type, const BufferView & body)
{
	std::lock_guard<std::mutex> lg(_mutex->mut);
	MessageHeader h;
	h.family = MessageFamily::Event;
	h.type = type;
	h.correlationId = 0;
	h.bodySize = body.size();
	_connection->Send(Bufferize(h));
	_connection->Send(body);
}

void mobyremote::Codec::Event(MessageType type, const std::initializer_list<BufferView>& bodyParts)
{
	std::lock_guard<std::mutex> lg(_mutex->mut);
	MessageHeader h;
	h.family = MessageFamily::Event;
	h.type = type;
	h.correlationId = 0;
	h.bodySize = 0;
	for (auto& part : bodyParts) {
		h.bodySize += part.size();
	}
	_connection->Send(Bufferize(h));
	for (auto& part : bodyParts) {
		_connection->Send(part);
	}
}

void mobyremote::Codec::ReceiveMessagesUntilClosed()
{
	try {
		while (_connection->Valid()) {
			MessageHeader h;
			auto hbuf = Bufferize(h);
			_connection->Receive(hbuf);
			Buffer b(h.bodySize);
			auto bodyBufView = b.View();
			_connection->Receive(bodyBufView);

			switch (h.family)
			{
			case MessageFamily::Event:
				_eventHandler(this, h.type, std::move(b));
				break;
			case MessageFamily::Request:
				_requestHandler(this, h.type, h.correlationId, std::move(b));
				break;
			case MessageFamily::Response:
			{
				std::lock_guard<std::mutex> lg(_mutex->mut);
				auto found = _pendingRequests.find(h.correlationId);
				if (found != _pendingRequests.end()) {
					found->second(this, h.type, std::move(b));
					_pendingRequests.erase(found);
				}
			}
				break;
			default:
				break;
			}
		}
	}
	catch (TransportErrorException&) {}
	Close();
}

void mobyremote::Codec::Response(std::uint32_t correlationId, MessageType messageType, const BufferView & body)
{
	std::lock_guard<std::mutex> lg(_mutex->mut);
	MessageHeader h;
	h.family = MessageFamily::Response;
	h.type = messageType;
	h.correlationId = correlationId;
	h.bodySize = body.size();
	_connection->Send(Bufferize(h));
	_connection->Send(body);
}

void mobyremote::Codec::Response(std::uint32_t correlationId, MessageType messageType, const std::initializer_list<BufferView>& bodyParts)
{
	std::lock_guard<std::mutex> lg(_mutex->mut);
	MessageHeader h;
	h.family = MessageFamily::Response;
	h.type = messageType;
	h.correlationId = correlationId;
	h.bodySize = 0;
	for (auto& part : bodyParts) {
		h.bodySize += part.size();
	}
	_connection->Send(Bufferize(h));
	for (auto& part : bodyParts) {
		_connection->Send(part);
	}
}

void mobyremote::Codec::Request(MessageType type, const BufferView & body, EventHandler callback)
{
	std::lock_guard<std::mutex> lg(_mutex->mut);
	MessageHeader h;
	h.family = MessageFamily::Request;
	h.type = type;
	h.correlationId = ++_lastCorrelationId;
	_pendingRequests[h.correlationId] = callback;
	h.bodySize = body.size();
	_connection->Send(Bufferize(h));
	_connection->Send(body);
}

void mobyremote::Codec::Request(MessageType type, const std::initializer_list<BufferView>& bodyParts, EventHandler callback)
{
	std::lock_guard<std::mutex> lg(_mutex->mut);
	MessageHeader h;
	h.family = MessageFamily::Request;
	h.type = type;
	h.correlationId = ++_lastCorrelationId;
	_pendingRequests[h.correlationId] = callback;
	h.bodySize = 0;
	for (auto& part : bodyParts) {
		h.bodySize += part.size();
	}
	_connection->Send(Bufferize(h));
	for (auto& part : bodyParts) {
		_connection->Send(part);
	}
}