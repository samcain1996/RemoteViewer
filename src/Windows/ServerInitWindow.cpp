#include "Windows/RemoteViewerWindows.h"

/*----------------Server Initialization Window---------------------*/

wxBEGIN_EVENT_TABLE(ServerInitWindow, BaseWindow)
EVT_BUTTON(30001, ServerInitWindow::StartServer)
EVT_IDLE(ServerInitWindow::BackgroundTask)
EVT_KEY_UP(ServerInitWindow::HandleInput)
EVT_TIMER(9123, ServerInitWindow::OnTick)
wxEND_EVENT_TABLE()

ServerInitWindow::ServerInitWindow(const wxPoint& pos, const wxSize& size) :
	BaseWindow("Server Initialization", pos, size), _timer(this, 9123), 
	_startServerButton(this, 30001, "Listen", wxPoint(400, 400), wxSize(200, 50)) {}

ServerInitWindow::~ServerInitWindow() {}

void ServerInitWindow::CleanUp() {

	_timer.Stop();

	if (_initialized) {
		_server->Disconnect();
	}

	BaseWindow::CleanUp();

}

void ServerInitWindow::StartServer(wxCommandEvent& evt) {

	_initialized = false;

	// Find a port to listen on
	Ushort port = Connection::SERVER_BASE_PORT;
	while (!NetAgent::portAvailable(port)) { port++; }

	// Start server on port and launch pop-up
	_server = make_unique<Server>(port);
	_popup->SetText("Listening on port " + std::to_string(port));
	_popup->Popup();

	// Listen on separate thread so window is still responsive
	std::thread([this] { 

		_server->Listen(_server->connections[0]);
		_initialized = true;

	}).detach();  // Race condition if reinit modifies after object deletion?

}

void ServerInitWindow::BackgroundTask(wxIdleEvent& evt) {

	if (!_initialized || _timer.IsRunning()) { return; }

	if (_server->Connected()) {
		_timer.Start(TARGET_FRAME_TIME);
	}
	else {
		_server.reset(nullptr);
		_initialized = false;
	}

}

void ServerInitWindow::OnTick(wxTimerEvent& timerEvent) {
	if (!_server->Serve()) { GoBack(); }
}