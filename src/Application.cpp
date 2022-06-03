#include "Application.h"
bool Application::_init = false;

bool Application::_isClient = false;

std::atomic<bool> Application::_exit = false;

GenericWindow* Application::_window = nullptr;

std::unique_ptr<NetAgent> Application::_netAgent = nullptr;

template <typename T>
MsgWriterPtr<T> Application::_writer = nullptr;

template <typename T>
MsgReaderPtr<T> Application::_reader = nullptr;

bool Application::Init() {

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) { return false; }
	if (TTF_Init() != 0) { return false; }

	std::string ipToConnectTo("");

	EventHandler func = [&](const SDL_Event& ev, const ElementView& elems) {
		
		const auto& clientButton = elems.GetElementByName("Client");
		const auto& serverButton = elems.GetElementByName("Server");

		TextBox& tBox = (TextBox&)elems.GetElementByName("TEST");
	
		if (ev.type == SDL_MOUSEBUTTONDOWN) {

			SDL_Rect mouse;
			mouse.w = 32;
			mouse.h = 32;

			SDL_GetMouseState(&mouse.x, &mouse.y);

			ipToConnectTo = tBox.Text();

			_isClient = SDL_HasIntersection(&mouse, &clientButton.Bounds());

			return !(_isClient || SDL_HasIntersection(&mouse, &serverButton.Bounds()));
		}


		return true;
		
	};

	_window = new InitWindow("Remote Viewer", func);
	InitWindow& initWin = dynamic_cast<InitWindow&>(*_window);

	_window->Update();

	if (_isClient) {
		_netAgent = std::unique_ptr<NetAgent>(new Client(10008, ipToConnectTo));
	}
	else {
		_netAgent = std::unique_ptr<NetAgent>(new Server(10009));
	}

	delete _window;
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

	TTF_Font* font = TTF_OpenFont("tahoma.ttf", 54);

	SDL_Rect tbBounds;
	tbBounds.x = 500;
	tbBounds.y = 500;
	tbBounds.h = 300;
	tbBounds.w = 100;
	TextBox portTb(font, "Port TextBox", tbBounds);
	std::vector<std::reference_wrapper<WindowElement>> elements{ portTb };

	EventHandler ev = [&](const SDL_Event& ev, const ElementView& elems) {

		if (ev.type == SDL_KEYDOWN) {
			if (ev.key.keysym.sym == SDLK_RETURN) {
				return !client.Connect(portTb.Text());
			}
		}

		return true;

	};

	_window = new GenericWindow("Connect To Port", ev, elements);
	_window->Update();

	delete _window;

	EventHandler func = [&](const SDL_Event& ev, const ElementView& elems) {
		if (ev.type == SDL_MOUSEBUTTONDOWN) {
			_exit = true;
			_writer<SDL_Event>->WriteMessage(ev);
			return false;
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

	delete _window;
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

	TTF_Quit();
	SDL_Quit();
}
