#pragma once

#include "Windows/BaseWindow.h"
#include "Messageable.h"

#include "Networking/Client.h"
#include "Networking/Server.h"


//--------------Start Up Window Class-----------------------------//

class StartUpWindow : public BaseWindow {
private:
	wxButton* _clientButton;
	wxButton* _serverButton;

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

	wxTextCtrl* _ipInput;
	wxButton* _connectButton;

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

	PixelData _imageData {};
	Resolution _resolution = ScreenCapture::DefaultResolution;

	bool doneConnecting = false;
	
	std::shared_ptr<Client> _client;
	std::jthread _clientThr;

	const int TARGET_FRAME_TIME = 1000 / 60;

	wxTimer _timer;


public:
	ClientStreamWindow(const std::string& ip, 
		const wxPoint& pos = DEFAULT_POS, const wxSize& size = DEFAULT_SIZE);
	~ClientStreamWindow();

	ClientStreamWindow(const ClientStreamWindow&) = delete;
	ClientStreamWindow(ClientStreamWindow&&) = delete;
	ClientStreamWindow& operator=(const ClientStreamWindow&) = delete;
	ClientStreamWindow& operator=(ClientStreamWindow&&) = delete;

	void ImageBuilder();  // Assemble image from packet queue
	
	void OnPaint(wxPaintEvent& evt);
	void Resize(const Resolution& resolution);
	void OnConnect(ConnectionPtr& pConnection);
	void CleanUp() override;
	
	void PaintNow();
	void BackgroundTask(wxIdleEvent& evt);

	void OnTick(wxTimerEvent& timerEvent);

	constexpr const WindowNames WindowName() override { return WindowNames::ClientStream; }

	wxDECLARE_EVENT_TABLE();
};


//--------------Server Init Window Class-----------------------------//

class ServerInitWindow : public BaseWindow {

private:

	wxButton* _startServerButton;
	wxTimer _timer;

	std::unique_ptr<Server> _server;
	
	const int TARGET_FRAME_TIME = 1000 / 60;

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

	void CleanUp() override;

	void OnTick(wxTimerEvent& timerEvent);
	
	constexpr const WindowNames WindowName() override { return WindowNames::ServerInit; }

	wxDECLARE_EVENT_TABLE();
};


