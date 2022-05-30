#include "Application.h"

bool Application::_init = false;

bool Application::_isClient = true;

std::atomic<bool> Application::_exit = false;

GenericWindow* Application::_window = nullptr;

NetAgent* Application::_netAgent = nullptr;

std::thread* Application::_networkThr = nullptr;

void Application::Init(const bool isClient) {

	_isClient = isClient;

	if (_isClient) {
		_netAgent = new Client(10008, "192.168.50.160");
	}
	else {
		_netAgent = new Server(10009);
	}

	_init = true;

}

void Application::Run() {

	if (!_init) { return; }

	if (_isClient) { RunClient(); }
	else { RunServer(); }

	Cleanup();

}

void Application::RunClient() {

	Client& client = dynamic_cast<Client&>(*_netAgent);
	if (!client.Connect("10009")) { return; }

	_window = new RenderWindow("192.168.50.160", &_exit, client._incompletePackets);
	RenderWindow& renderWindow = dynamic_cast<RenderWindow&>(*_window);

	ConnectMsgHandlers(client.writer, renderWindow.completeGroups);

	_networkThr = new std::thread(&Client::Receive, &client);

	renderWindow.Update();

	_networkThr->join();
	delete _networkThr;

}

void Application::RunServer() {

	Server& server = dynamic_cast<Server&>(*_netAgent);

	server.Listen();

	while (!_exit) {
		server.Serve();
	}

}

void Application::Cleanup() {

	delete _netAgent;
	delete _window;
}
