#pragma once

#include "Windows/BaseWindow.h"
#include "Messageable.h"
#include "Networking/Client.h"
#include "Networking/Server.h"
#include "Timer.h"

//--------------Start Up Window Class-----------------------------//

class StartUpWindow : public BaseWindow {

private:

	wxButton _clientButton;
	wxButton _serverButton;

public:

	StartUpWindow(const wxPoint& pos = DEFAULT_POS, const wxSize& size = DEFAULT_SIZE);
	~StartUpWindow();

	// Delete copy and move constructors and assignment operators
	StartUpWindow(const StartUpWindow&) = delete;
	StartUpWindow(StartUpWindow&&) = delete;
	StartUpWindow& operator=(const StartUpWindow&) = delete;
	StartUpWindow& operator=(StartUpWindow&&) = delete;

	void ClientButtonClick(wxCommandEvent& evt);
	void ServerButtonClick(wxCommandEvent& evt);

	constexpr const WindowNames WindowName() override { return WindowNames::StartUp; }

	wxDECLARE_EVENT_TABLE();
};

//--------------Client Init Window Class-----------------------------//

class ClientInitWindow : public BaseWindow {

private:

	wxTextCtrl _ipInput;
	wxButton _connectButton;

public:

	ClientInitWindow(const wxPoint& pos = DEFAULT_POS, const wxSize& size = DEFAULT_SIZE);
	~ClientInitWindow();

	// Delete copy and move constructors and assignment operators
	ClientInitWindow(const ClientInitWindow&) = delete;
	ClientInitWindow(ClientInitWindow&&) = delete;
	ClientInitWindow& operator=(const ClientInitWindow&) = delete;
	ClientInitWindow& operator=(ClientInitWindow&&) = delete;

	void ConnectButtonClick(wxCommandEvent& evt);

	constexpr const WindowNames WindowName() override { return WindowNames::ClientInit; }

	wxDECLARE_EVENT_TABLE();
};




//--------------Client Stream Window Class-----------------------------//
// Receives a video stream from the server and sends 
// back a stream of keyboard and mouse events

class ClientStreamWindow : public BaseWindow, public Messageable<PacketPtr> {

	MessageReader<PacketPtr>*& packetReader = msgReader;

private:
	steady_clock::time_point prev;
	PixelData _imageData {};
	Resolution _resolution = ScreenCapture::DefaultResolution;
	
	unique_ptr<Client> _client;

	const int TARGET_FRAME_TIME = 1000 / Configs::TARGET_FPS;

	unique_ptr<Timer> pTimer;


public:
	ClientStreamWindow(const string& ip, 
		const wxPoint& pos = DEFAULT_POS, const wxSize& size = DEFAULT_SIZE);
	~ClientStreamWindow();

	ClientStreamWindow(const ClientStreamWindow&) = delete;
	ClientStreamWindow(ClientStreamWindow&&) = delete;
	ClientStreamWindow& operator=(const ClientStreamWindow&) = delete;
	ClientStreamWindow& operator=(ClientStreamWindow&&) = delete;

	void OnClick(wxMouseEvent& mouseEvent);

	void ImageBuilder();  // Assemble image from packet queue
	
	void Resize(const Resolution& resolution);
	void CleanUp() override;
	
	bool Connect();
	void PaintNow();
	void BackgroundTask(wxIdleEvent& evt);

	constexpr const WindowNames WindowName() override { return WindowNames::ClientStream; }

	wxDECLARE_EVENT_TABLE();
};


//--------------Server Init Window Class-----------------------------//

class ServerInitWindow : public BaseWindow {

private:

	wxButton _startServerButton;
	wxTimer _timer;
	unique_ptr<Timer> pTimer;

	future<bool> listenRes;
	unique_ptr<Server> _server;
	thread listenThr;
	
	const int TARGET_FRAME_TIME = 1000 / Configs::TARGET_FPS;

public:
	ServerInitWindow(const wxPoint& pos, const wxSize& size);
	~ServerInitWindow();

	// Delete copy and move constructors and assignment operators
	ServerInitWindow(const ServerInitWindow&) = delete;
	ServerInitWindow(ServerInitWindow&&) = delete;
	ServerInitWindow& operator=(const ServerInitWindow&) = delete;
	ServerInitWindow& operator=(ServerInitWindow&&) = delete;
	
	void StartServer(wxCommandEvent& evt);
	void BackgroundTask(wxIdleEvent& evt);	
	bool Listen();

	void CleanUp() override;

	void OnTick(wxTimerEvent& timerEvent);
	
	constexpr const WindowNames WindowName() override { return WindowNames::ServerInit; }

	wxDECLARE_EVENT_TABLE();
};


