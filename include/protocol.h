#pragma once
#include "common.h"
#include <functional>
#include <map>

namespace mobyremote {
	enum class MessageType : std::uint32_t{
		ReplaceFileRequest,
		ReplaceFileResponse,
		ConnectionInterrupted,
		NotImplemented,
		ExposePortRequest,
		ExposePortResponse
	};
	enum class MessageFamily : std::uint32_t {
		Request,
		Response,
		Event
	};
#pragma pack(push,1)
	struct MessageHeader {
		MessageType type;
		MessageFamily family;
		std::uint32_t correlationId;
		std::uint32_t bodySize;
	};
	struct PortForwardingRequest {
		std::int8_t action; // 1:on, 2:off
		std::int8_t protocols; // 0x1:tcp, 0x2:udp
		std::uint16_t port;
	};
	struct PortForwardingResponse {
		std::uint32_t status; // 1:ok, 2:ko
		static PortForwardingResponse Ok() {
			return PortForwardingResponse{ 1 };
		}
		static PortForwardingResponse Fail() {
			return PortForwardingResponse{ 2 };
		}
	};
#pragma pack(pop)
	class Buffer {
	private:
		std::unique_ptr<char[]> _data;
		std::uint32_t _size;
	public:
		Buffer() :_data(nullptr), _size(0) {}
		Buffer(std::uint32_t size) :_data(new char[size]), _size(size) {}
		Buffer(BufferView copy) : _data(new char[copy.size()]), _size(copy.size()) {
			memcpy(_data.get(), copy.begin(), copy.size());
		}
		BufferView View() { return BufferView(_data.get(), _size); }
		Buffer Clone() const {
			Buffer other(_size);
			memcpy(other._data.get(), _data.get(), _size);
			return other;
		}
		Buffer(Buffer&& moved) : _data(std::move(moved._data)), _size(moved._size) {
			moved._size = 0;
		}
		Buffer& operator =(Buffer&& moved) {
			if (&moved == this) {
				return *this;
			}
			_data = std::move(moved._data);
			_size = moved._size;
			moved._size = 0;
			return *this;
		}
		~Buffer() {
			_data.reset();
		}
	};
	
	class Codec;
	using EventHandler = std::function<void(Codec*, MessageType, Buffer)>;
	using RequestHandler = std::function<void(Codec*, MessageType, std::uint32_t, Buffer)>;
	class Codec {
		struct MutexWrapper;
	private:
		std::unique_ptr<MutexWrapper> _mutex;
		std::unique_ptr<Connection> _connection;
		std::uint32_t _lastCorrelationId;
		EventHandler _eventHandler;
		RequestHandler _requestHandler;
		std::map<std::uint32_t, EventHandler> _pendingRequests;
	public:
		Codec(std::unique_ptr<Connection>&& conn, const EventHandler& eventHandler, const RequestHandler& requestHandler);
		~Codec();
		void ReceiveMessagesUntilClosed();

		void Request(MessageType type, const BufferView& body, EventHandler callback);
		void Request(MessageType type, const std::initializer_list<BufferView>& bodyParts, EventHandler callback);
		void Event(MessageType type, const BufferView& body);
		void Event(MessageType type, const std::initializer_list<BufferView>& bodyParts);
		void Response(std::uint32_t correlationId, MessageType messageType, const BufferView& body);
		void Response(std::uint32_t correlationId, MessageType messageType, const std::initializer_list<BufferView>& bodyParts);
		void Close() {
			if (_connection) {
				_connection->Close();
			}
			for (auto& pending : _pendingRequests) {
				pending.second(this, MessageType::ConnectionInterrupted, Buffer());
			}
			_pendingRequests.clear();
		}
	};

	class ReplaceFileRequest;

	class ReplaceFileRequest {
	private:
		std::string _fileName;
		Buffer _content;
	public:
		ReplaceFileRequest() = default;
		ReplaceFileRequest(const std::string& fileName, Buffer&& content);
		ReplaceFileRequest(const BufferView& flatData);
		std::string& fileName() { return _fileName; }
		Buffer& content() { return _content; }

		void Send(Codec* codec, std::function<void(bool)> callback);
	};
}