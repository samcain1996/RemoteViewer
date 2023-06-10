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
	_startServerButton(this, 30001, "Listen", wxPoint(400, 400), wxSize(200, 50)) {

	//pTimer = make_unique<Timer>(TARGET_FRAME_TIME, [this]() {
	//	
	//	_server->Serve();
	//	
	//	});
}

ServerInitWindow::~ServerInitWindow() {}

void ServerInitWindow::CleanUp() {

	_timer.Stop();
	//pTimer->stop();
	BaseWindow::CleanUp();

}

void ServerInitWindow::StartServer(wxCommandEvent& evt) {

	// Find a port to listen on
	Ushort port = Connection::SERVER_BASE_PORT;
	while (!NetAgent::portAvailable(port)) { port++; }

	// Start server on port and launch pop-up
	_server = make_unique<Server>(port);
	_popup->SetText("Listening on port " + std::to_string(port));
	_popup->Popup();

	std::packaged_task<bool()> listen(bind(&ServerInitWindow::Listen, this));
	listenRes = listen.get_future();
	// Listen on separate thread so window is still responsive
	listenThr = thread(move(listen));
	// Race condition if reinit modifies after object deletion?

}

bool ServerInitWindow::Listen() {

	// Listen for connection
	for (ConnectionPtr& connection : _server->connections) {
		if (!_server->Listen(connection)) { return false; }
	}

	if (_server->Connected()) {

		_server->Listen(_server->dataCon);

		// Start separate thread to receive data
		thread([this]() { 
			_server->Receive(_server->dataCon); 
			_server->dataCon->pIO_cont->run(); }).detach();

	}

	return _server->Connected();
}

void ServerInitWindow::BackgroundTask(wxIdleEvent& evt) {

	if (listenThr.joinable()) {
		if (listenRes.wait_for(seconds(0)) == std::future_status::ready) {
			listenThr.join();
			if (listenRes.get()) { /*pTimer->start(); */ _timer.Start(TARGET_FRAME_TIME); }
			else { _server.reset(); }
		}
	}

}

void ServerInitWindow::OnTick(wxTimerEvent& timerEvent) {
	if (!_server->Serve()) { GoBack(); }
}