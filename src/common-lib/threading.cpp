#include <common.h>
#include <thread>
using namespace mobyremote;
std::unique_ptr<std::function<void(const std::function<void(void)>&)>> _mobyStartThread;
void mobyremote::overrideStartThread(std::function<void(const std::function<void(void)>&)> threadStarter)
{
	_mobyStartThread = std::make_unique<std::function<void(const std::function<void(void)>&)>>(threadStarter);
}

void mobyremote::startThread(const std::function<void(void)>& callback)
{
	if (_mobyStartThread) {
		(*_mobyStartThread)(callback);
	}
	else {
		std::thread(callback).detach();
	}
}
void mobyremote::resetStartThread()
{
	_mobyStartThread.reset();
}