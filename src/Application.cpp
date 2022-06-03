#include "Application.h"
bool Application::_init = false;

bool Application::_isClient = false;

std::atomic<bool> Application::_exit = false;

GenericWindow* Application::_window = nullptr;

NetAgent* Application::_netAgent = nullptr;

template <typename T>
MsgWriterPtr<T> Application::_writer = nullptr;

template <typename T>
MsgReaderPtr<T> Application::_reader = nullptr;

bool Application::Init() {

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) { return false; }

	EventHandler func = [&](const SDL_Event& ev, const ElementManager& elems) {
		
		const auto& clientButton = elems.GetElementByName("Client");
		const auto& serverButton = elems.GetElementByName("Server");
	
		if (ev.type == SDL_MOUSEBUTTONDOWN) {

			SDL_Rect mouse;
			mouse.w = 32;
			mouse.h = 32;

			SDL_GetMouseState(&mouse.x, &mouse.y);

			_isClient = SDL_HasIntersection(&mouse, &clientButton.Bounds());

			return !(_isClient || SDL_HasIntersection(&mouse, &serverButton.Bounds()));
		}

		return true;
		
	};

	_window = new InitWindow("192.168.50.160", func);
	InitWindow& initWin = dynamic_cast<InitWindow&>(*_window);

	initWin.Update();

	if (_isClient) {
		_netAgent = new Client(10008, "192.168.50.160");
	}
	else {
		_netAgent = new Server(10009);
	}

	_init = true;
	delete _window;

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

	EventHandler func = [&](const SDL_Event& ev, const ElementManager& elems) {
		if (ev.type == SDL_MOUSEBUTTONDOWN) {
			_exit = true;
			_writer<SDL_Event>->WriteMessage(ev);
		}
		return true;
	};

	_window = new RenderWindow("192.168.50.160", func);
	RenderWindow& renderWindow = dynamic_cast<RenderWindow&>(*_window);

	ConnectMsgHandlers(client.writer, renderWindow.completeGroups);
	ConnectMsgHandlers<SDL_Event>(_writer<SDL_Event>, client.eventReader);

	std::thread networkThr(&Client::Receive, &client);
	
	renderWindow.Update();

	networkThr.join();

	delete _writer<SDL_Event>;
	delete _window;
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

	SDL_Quit();
}
