#include <protocol.h>
#include <cassert>
using namespace mobyremote;

mobyremote::ReplaceFileRequest::ReplaceFileRequest(const std::string & fileName, Buffer && content) : _fileName(fileName), _content(std::move(content))
{
}

mobyremote::ReplaceFileRequest::ReplaceFileRequest(const BufferView & flatData) : _fileName(flatData.begin())
{
	// content is at filenamesize +1 (for trailing \0)
	_content = Buffer(flatData.subBuffer((std::uint32_t)_fileName.size()+1));	
}

void mobyremote::ReplaceFileRequest::Send(Codec * codec, std::function<void(bool)> callback)
{
	codec->Request(MessageType::ReplaceFileRequest, { Bufferize(_fileName), _content.View() }, [callback](Codec* c, MessageType type, Buffer b) {
		if (type == MessageType::ConnectionInterrupted || type == MessageType::NotImplemented) {
			callback(false);
			return;
		}
		assert(type == MessageType::ReplaceFileResponse);
		assert(b.View().size() == sizeof(bool));
		callback(*reinterpret_cast<bool*>(b.View().begin()));
	});
}