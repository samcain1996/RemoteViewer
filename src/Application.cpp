#include "Application.h"
bool Application::_init = false;

bool Application::_isClient = true;

std::atomic<bool> Application::_exit = false;

GenericWindow* Application::_window = nullptr;

NetAgent* Application::_netAgent = nullptr;

template <typename T>
MsgWriterPtr<T> Application::_writer = nullptr;

template <typename T>
MsgReaderPtr<T> Application::_reader = nullptr;

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
	
	ConnectMsgHandlers<SDL_Event>(_writer<SDL_Event>, client.eventReader);

	std::thread networkThr(&Client::Receive, &client);
	
	Update(renderWindow);

	networkThr.join();

	delete _writer<SDL_Event>;
}

void Application::RunServer(Server& server) {

	server.Listen();

	ConnectMsgHandlers<ByteArray>(server.eventWriter, _reader<ByteArray>);

	while (!_exit) {
		if (!_reader<ByteArray>->Empty()) {
			if (_reader<ByteArray>->ReadMessage()) {
				_exit = true;
				break;
			}
		}
		server.Serve();
	}

}

void Application::Cleanup() {

	delete _netAgent;

	if (_isClient)
		delete _window;

	SDL_Quit();
}

void Update(RenderWindow& window) {

	Uint32 ticks;

	while (!Application::_exit) {

		// Get events
		while (SDL_PollEvent(&(window._event))) {

			if (window._event.type == SDL_MOUSEBUTTONDOWN) {
				Application::_exit = true;
				Application::_writer<SDL_Event>->WriteMessage(window._event);
			}

		}

		ticks = SDL_GetTicks();

		if (window.Draw()) {
			//SDL_FreeSurface(window->_surface);
		}  // Draw content to window

		window.CapFPS(ticks);

	}

}
