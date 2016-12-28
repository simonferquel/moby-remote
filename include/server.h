#pragma once
#include "protocol.h"
namespace mobyremote {
	class IServerHandler {
	public:
		virtual ~IServerHandler() {}

		virtual bool OnReplaceFileRequest(ReplaceFileRequest request) = 0;
	};

	class MobyServerHandler : public IServerHandler {
	public:
		virtual bool OnReplaceFileRequest(ReplaceFileRequest request) override;
	};

	class Server {
	private:
		std::unique_ptr<Codec> _codec;
		std::shared_ptr<IServerHandler> _handler;
		Server(std::unique_ptr<Codec>&& codec, const std::shared_ptr<IServerHandler>& handler) :_codec(std::move(codec)), _handler(handler) {}
	public:

		Codec* GetCodec() { return _codec.get(); }
		static std::shared_ptr<Server> make(std::unique_ptr<Connection>&& conn, const std::shared_ptr<IServerHandler>& handler);
		void Stop() {
			_codec->Close();
		}
	};
}