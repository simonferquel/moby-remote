#pragma once
#include "protocol.h"
namespace mobyremote {
	class IClientHandler {
	public:
		virtual ~IClientHandler(){}
		virtual PortForwardingResponse OnExposePortAction(PortForwardingRequest req) = 0;
	};
	class Client {
	private:
		std::unique_ptr<Codec> _codec;
		std::shared_ptr<IClientHandler> _handler;
		Client(std::unique_ptr<Codec>&& codec, const std::shared_ptr<IClientHandler>& handler) :_codec(std::move(codec)), _handler(handler) {}
	public:

		Codec* GetCodec() { return _codec.get(); }
		static std::shared_ptr<Client> make(std::unique_ptr<Connection>&& conn, const std::shared_ptr<IClientHandler>& handler);
		void Stop() {
			_codec->Close();
		}

		void ReplaceFileContent(const std::string& fileName, Buffer&& b, std::function<void(bool)> callback);
	};
}