#include "Application.h"
bool Application::_init = false;

bool Application::_isClient = false;

std::atomic<bool> Application::_exit = false;

std::unique_ptr<NetAgent> Application::_netAgent = nullptr;

std::unique_ptr<GenericWindow> Application::_window = nullptr;

template <typename Message>
Messageable<Message> Application::_msgHandler;

bool Application::Init() {

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) { return false; }
	if (TTF_Init() != 0) { return false; }


	Button clientButton(500, 500, "Client Button", "Client");
	Button serverButton(1000, 500, "Server Button", "Server");

	TextBox localPortTb(500, 500, "Local Port Textbox", "Local Port");
	TextBox remotePortTb(1000, 500, "Remote Port TextBox", "Remote Port");
	TextBox ipTextbox(1500, 500, "IP Textbox", "IP to connect to");


	ElementList elements { clientButton, serverButton };


	EventHandler clientInitEventHandler = [&](const EventData& ed, const ElementView& elemView) {

		const SDL_Rect& mouseRect = ed.mouseRect;
		const WindowElement& elementInFocus = elemView.elemInFocus;

		if (ed.windowEvent.type == SDL_KEYDOWN) {
			if (elementInFocus == ipTextbox) {
				if (ed.windowEvent.key.keysym.sym == SDLK_RETURN) {
					_netAgent = std::unique_ptr<NetAgent>(new Client(std::stoi(remotePortTb.Text()), ipTextbox.Text()));
					return false;
				}
			}
		}

		return true;
	};


	EventHandler serverInitEventHandler = [&](const EventData& ed, const ElementView& elemView) {

		const SDL_Rect& mouseRect = ed.mouseRect;
		const WindowElement& elementInFocus = elemView.elemInFocus;

		if (ed.windowEvent.type == SDL_KEYDOWN) {

			if (elementInFocus == localPortTb) {
				if (ed.windowEvent.key.keysym.sym == SDLK_RETURN) {
					_netAgent = std::unique_ptr<NetAgent>(new Server(std::stoi(localPortTb.Text())));
					return false;
				}
				
			}
		}

		return true;

	};

	EventHandler eventHandler = [&](const EventData& ed, const ElementView& elemView) {

		const SDL_Rect& mouseRect = ed.mouseRect;

		// Check if either button has been hit
		if (SDL_HasIntersection(&mouseRect, &clientButton.Bounds())) {
			_isClient = true;

			elements.clear();
			elements.emplace_back(ipTextbox);
			elements.emplace_back(remotePortTb);

		}

		else if (SDL_HasIntersection(&mouseRect, &serverButton.Bounds())) {
			_isClient = false;

			elements.clear();

		}

		else {
			return true;
		}

		elements.emplace_back( localPortTb );

		return false;
	};


	_window = std::unique_ptr<GenericWindow>(new GenericWindow("Anonymous Window", eventHandler, elements));
	_window.reset();

	_window = _isClient ? std::unique_ptr<GenericWindow>(new GenericWindow("Client Initialization", clientInitEventHandler, elements)) :
		std::unique_ptr<GenericWindow>(new GenericWindow("Server Initialization", serverInitEventHandler, elements));
	_window->Update();
	_window.reset();


	elements.clear();
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

	EventHandler func = [&](const EventData& ed, const ElementView& elems) {

		const SDL_Event& windowEvent = ed.windowEvent;

		if (windowEvent.type == SDL_MOUSEBUTTONDOWN || windowEvent.type == SDL_QUIT) {
			_exit = true;
			_msgHandler<SDL_Event>.msgWriter->WriteMessage(windowEvent);
			return false;
		}
		return true;
	};

	_window = std::unique_ptr<GenericWindow>(new RenderWindow("192.168.50.160", func));
	RenderWindow& renderWindow = dynamic_cast<RenderWindow&>(*_window);

	ConnectMessageHandlers<PacketPriorityQueue*>(&client, &renderWindow);
	ConnectMessageHandlers<SDL_Event>(&client, &_msgHandler<SDL_Event>);

	std::thread networkThr(&Client::Receive, &client);
	
	renderWindow.Update();

	networkThr.join();
}

void Application::RunServer(Server& server) {

	server.Listen();

	ConnectMessageHandlers<ByteArray>(&server, &_msgHandler<ByteArray>);

	while (!_exit) {
		if (!_msgHandler<ByteArray>.msgReader->Empty()) {
			if (_msgHandler<ByteArray>.msgReader->ReadMessage()) {
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
