#pragma once
#include "RemoteViewerWindows.h"

/*----------------Server Initialization Window---------------------*/

wxBEGIN_EVENT_TABLE(ServerInitWindow, BaseWindow)
EVT_BUTTON(30001, ServerInitWindow::StartServer)
EVT_IDLE(ServerInitWindow::BackgroundTask)
EVT_KEY_UP(ServerInitWindow::HandleInput)
EVT_TIMER(2345, ServerInitWindow::OnTick)
wxEND_EVENT_TABLE()

ServerInitWindow::ServerInitWindow(const wxPoint& pos, const wxSize& size) :
	BaseWindow("Server Initialization", pos, size), iconData(CalculateBMPFileSize({ 16, 16 }, 32)), _timer(this, 2345) {
	_portTb = new wxTextCtrl(this, 30002, "20009", wxPoint(100, 200), wxSize(500, 50), 0L, IP_VALIDATOR);

	_startServerButton = new wxButton(this, 30001, "Listen for connections", wxPoint(400, 400), wxSize(200, 50));

}

ServerInitWindow::~ServerInitWindow() {
	delete _server;
}

void ServerInitWindow::StartServer(wxCommandEvent& evt) {
	const int listenPort = std::stoi(_portTb->GetValue().ToStdString());

	_server = new Server(listenPort, std::chrono::seconds(10));

	std::string message("Listening on port " + std::to_string(listenPort));
	_popup = new PopUp(this, message);
	_popup->Popup();

	_init = true;

}

void ServerInitWindow::BackgroundTask(wxIdleEvent& evt) {

	if (!_init) { return; }

	if (!_server->Connected()) {

		_server->Listen();

		if (!_server->Connected()) {
			GoBack();
		}

		else {
			delete _popup;
			Hide();

			_timer.Start(1000 / _targetFPS);
		}

	}

	else {

		if (!_server->Serve()) {
			GoBack();
		}

	}
}

void ServerInitWindow::OnTick(wxTimerEvent& timerEvent) {
	wxWakeUpIdle();
	_timer.Start(1000 / _targetFPS);
}
