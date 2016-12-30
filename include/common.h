#pragma once
#include <memory>
#include <utility>
#include <functional>
#include <cstdint>
#include <stdexcept>
#include <iostream>
#ifdef _WIN32
#include <WinSock2.h>
#include <chrono>
#else
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
using SOCKET = int;
#define INVALID_SOCKET -1
#define closesocket close
#define SD_BOTH SHUT_RDWR
#endif
namespace mobyremote{
	enum class TransportError {
		InvalidSocket,
		BindFailed,
		ListenFailed,
		SendReceiveFailed,
		NameResolutionFailed,
		ConnectFailed
	};
	struct TransportErrorException{
		TransportError Error;
		TransportErrorException(TransportError e);
	};
	class SafeSocket {
	private:
		SOCKET _socket;
	public:
		SafeSocket(SOCKET socket) :_socket(socket){
			if (socket == INVALID_SOCKET) {
				throw TransportErrorException{ TransportError::InvalidSocket };
			}
		}

		SafeSocket() :_socket(INVALID_SOCKET) {
		}

		SafeSocket(const SafeSocket&) = delete;
		SafeSocket& operator =(const SafeSocket&) = delete;

		SafeSocket(SafeSocket&& moved) : _socket(moved._socket){
			moved._socket = INVALID_SOCKET;
		}

		SafeSocket& operator =(SafeSocket&& moved) {
			if (this != &moved) {
				std::swap(_socket, moved._socket);
			}
			return *this;			
		}
		void Close() {
			if (_socket != INVALID_SOCKET) {
				shutdown(_socket, SD_BOTH);
				if (0 != closesocket(_socket))
				{
					std::cerr << "close socket failed errno: " << errno << std::endl;
				}
				_socket = INVALID_SOCKET;
			
			}
		}

		~SafeSocket() {
			Close();
		}

		SOCKET Get() const {
			return _socket;
		}
	};

	class BufferView{
	private:
		char* _begin;
		std::uint32_t _size;
	public:
		BufferView(char* data, std::uint32_t size) : _begin(data), _size(size){}

		char* begin() { return _begin; }
		const char* begin() const { return _begin; }

		char* end() { return _begin+_size; }
		const char* end() const { return _begin + _size; }

		std::uint32_t size() const { return _size; }

		BufferView subBuffer(std::uint32_t offset) const {
			if (offset > _size) {
				throw std::out_of_range("offset out of range");
			}
			return BufferView(_begin + offset, _size - offset);
		}
		BufferView subBuffer(std::uint32_t offset, std::uint32_t length) const {
			if (offset+length > _size) {
				throw std::out_of_range("subBuffer out of range");
			}
			return BufferView(_begin + offset, length);
		}

	};

	template<typename T>
	BufferView Bufferize(T& value) {
		return BufferView(reinterpret_cast<char*>(&value), sizeof(T));
	}

	template<>
	inline BufferView Bufferize(std::string& value) {
		return BufferView(&value[0], (std::uint32_t)value.size() + 1);
	}

	class Connection {
	private:
		SafeSocket _socket;
	public:
		Connection(SafeSocket&& s);
		void Send(const BufferView& buf);
		void Receive(BufferView& buf);
		void Close() {
			_socket.Close();
		}
		bool Valid() const {
			return _socket.Get() != INVALID_SOCKET;
		}
	};

    class Listener{
	private:
		SafeSocket _listeningSocket;
    public:
		Listener(int port);
		Listener(const char* address, int port);
		void StartAcceptLoop(const std::function<void(std::unique_ptr<Connection>)>& callback);
    };

	std::unique_ptr<Connection> ConnectTo(const char* hostname, int port);
#ifdef _WIN32
	std::unique_ptr<Connection> ConnectTo(const char* hostname, int port, std::chrono::milliseconds timeout);
#endif

	void startThread(const std::function<void(void)>& callback);
	void overrideStartThread(std::function<void(const std::function<void(void)>&)> threadStarter);
}