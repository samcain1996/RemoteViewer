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
	if (_listenThr.joinable()) { _listenThr.join(); }
	if (_server.get() != nullptr) { _server->Disconnect(); _server.reset(nullptr); }
}

void ServerInitWindow::StartServer(wxCommandEvent& evt) {

	while (NetAgent::port_in_use(portToTry)) { portToTry++; }

	_server = std::make_unique<Server>(portToTry, std::chrono::seconds(10));
	_popup = std::make_unique<PopUp>(this, "Listening on port " + std::to_string(portToTry));
	_popup->Popup();

	_listenThr = std::thread([this]() { _server->Listen(); reinit = true; });  // Race condition if reinit modifies after object deletion?
	_listenThr.detach();

	_init = true;
}

void ServerInitWindow::BackgroundTask(wxIdleEvent& evt) {

	if (_init && reinit) {
		reinit = false;
		_popup.reset(nullptr);

		if (_server->Connected()) {
			SetSize({ 100, 100 });
			_timer.Start(1000 / _targetFPS);
		}
	}
}

void ServerInitWindow::OnTick(wxTimerEvent& timerEvent) {
	wxWakeUpIdle();
	if (!_server->Serve()) { GoBack(); }
}