#include "Application.h"
bool Application::_init = false;

bool Application::_isClient = true;

std::atomic<bool> Application::_exit = false;

GenericWindow* Application::_window = nullptr;

NetAgent* Application::_netAgent = nullptr;

MessageReader<ByteArray>* Application::eventReader = nullptr;
MessageReader<SDL_Event>* Application::sigh = nullptr;

bool Application::Init(const bool isClient) {

	_isClient = isClient;

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) { return false; }

	if (_isClient) {
		_netAgent = new Client(10008, "192.168.50.160");
	}
	else {
		_netAgent = new Server(10009);
	}

	_init = true;

	return true;

}

void Application::Run() {

	if (!_init) { return; }

	if (_isClient) { RunClient(dynamic_cast<Client&>(*_netAgent)); }
	else { RunServer(dynamic_cast<Server&>(*_netAgent)); }

	Cleanup();

}

void Application::RunClient(Client& client) {

	if (!client.Connect("10009")) { return; }

	_window = new RenderWindow("192.168.50.160", _exit);
	RenderWindow& renderWindow = dynamic_cast<RenderWindow&>(*_window);

	ConnectMsgHandlers(client.writer, renderWindow.completeGroups);
	ConnectMsgHandlers(renderWindow.eventWriter, client.eventReader);
	ConnectMsgHandlers(renderWindow.eventWriter2, sigh);

	std::thread msgThr([&](){
		while (!_exit) {
			if (!sigh->Empty()) {
				
				if (sigh->ReadMessage().type == SDL_MOUSEBUTTONDOWN) {
					_exit = true;
					
				}

			}
		}
		});

	std::thread networkThr(&Client::Receive, &client);

	renderWindow.Update();

	msgThr.join();
	networkThr.join();


}

void Application::RunServer(Server& server) {

	server.Listen();
	ConnectMsgHandlers(server.eventWriter, eventReader);

	while (!_exit) {
		if (!eventReader->Empty()) {
			if (eventReader->ReadMessage()) {
				_exit = true;
			}
		}
		server.Serve();
	}

	server.Disconnect();

}

void Application::Cleanup() {

	delete _netAgent;
	delete _window;

	SDL_Quit();
}
