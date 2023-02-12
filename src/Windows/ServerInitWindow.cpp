#include "Windows/RemoteViewerWindows.h"

/*----------------Server Initialization Window---------------------*/

wxBEGIN_EVENT_TABLE(ServerInitWindow, BaseWindow)
EVT_BUTTON(30001, ServerInitWindow::StartServer)
EVT_IDLE(ServerInitWindow::BackgroundTask)
EVT_KEY_UP(ServerInitWindow::HandleInput)
EVT_TIMER(9123, ServerInitWindow::OnTick)
wxEND_EVENT_TABLE()

ServerInitWindow::ServerInitWindow(const wxPoint& pos, const wxSize& size) :
	BaseWindow("Server Initialization", pos, size), _timer(this, 9123) {

	_startServerButton = new wxButton(this, 30001, "Listen", wxPoint(400, 400), wxSize(200, 50));

}

ServerInitWindow::~ServerInitWindow() {}

void ServerInitWindow::CleanUp() {
	_timer.Stop();
	SetSize(DEFAULT_SIZE);

	if (_init) {
		std::for_each(_server->connections.begin(), _server->connections.end(),
			[this](ConnectionPtr& pConnection) {

				if (pConnection->connected) {
					_server->Disconnect(pConnection);
				}
			});
	}

}

void ServerInitWindow::StartServer(wxCommandEvent& evt) {

	// Find a port to listen on
	int port = Connection::DEFUALT_PORT + Connection::SERVER_PORT_OFFSET;
	while (NetAgent::port_in_use(port)) { port++; }

	// Start server on port and launch pop-up
	_server = std::make_unique<Server>(port);
	_popup->SetText("Listening on port " + std::to_string(port));
	_popup->Popup();

	// Listen on separate thread so window is still responsive
	std::jthread([this] { 

		_init = false;
		_server->Listen(_server->connections[0]);
		_init = true;

	}).detach();  // Race condition if reinit modifies after object deletion?

}

void ServerInitWindow::BackgroundTask(wxIdleEvent& evt) {

	if (!_init || _timer.IsRunning()) { return; }

	if (_server->Connected(0)) {
		SetSize(MINIMIZED_SIZE);
		_timer.Start(1000 / _targetFPS);
	}
	else {
		_server.reset(nullptr);
		_init = false;
	}

}

void ServerInitWindow::OnTick(wxTimerEvent& timerEvent) {
	if (!_server->Serve()) { GoBack(); }
}