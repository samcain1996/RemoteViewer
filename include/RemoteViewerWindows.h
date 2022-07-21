#pragma once
#include "Types.h"
#include <stack>
#include "wx/wx.h"
#include "wx/mstream.h"
#include "Client.h"
#include "Server.h"

enum class WindowNames {
	StartUp,
	ClientInit,
	ServerInit,
	ClientStream
};

using WindowStack = std::stack<WindowNames>;
using ElementList = std::vector<wxControl*>;

using Button = wxButton;

//--------------Abstract Base Window Class-----------------------------//

class BaseWindow : public wxFrame
{
public:
	virtual constexpr const WindowNames WindowName() = 0;
	// Handle events shared across all windows such as going back to the previous window
	virtual void HandleInput(wxKeyEvent& keyEvent);

	// Textbox validators used
	wxTextValidator IP_VALIDATOR = wxTextValidator(wxFILTER_INCLUDE_CHAR_LIST);
	wxTextValidator PORT_VALIDATOR = wxTextValidator(wxFILTER_DIGITS);
protected:

	// Previous Windows
	static WindowStack _prevWindows;

	BaseWindow(const std::string& name);
	virtual ~BaseWindow();

	// Delete copy and move constructors and assignment operators
	BaseWindow(const BaseWindow&) = delete;
	BaseWindow(BaseWindow&&) = delete;
	BaseWindow& operator=(const BaseWindow&) = delete;
	BaseWindow& operator=(BaseWindow&&) = delete;
	
	// All Window controls on the current window
	ElementList _windowElements;
	const int _windowId;

	// Flag indicating whether the program whould quit on window close
	bool _killProgramOnClose = true;



};


//--------------Start Up Window Class-----------------------------//

class StartUpWindow : public BaseWindow {
private:
	Button* _clientButton;
	Button* _serverButton;
	
public:
	StartUpWindow();
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

	const int PORT_TB_ID = 20003;
	const int IP_TB_ID = 20002;
	
	wxTextCtrl* _remotePortInput;
	wxTextCtrl* _localPortInput;
	wxTextCtrl* _ipInput;
	Button* _connectButton;

public:
	ClientInitWindow();
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

class ClientStreamWindow : public BaseWindow, public Messageable<PacketPriorityQueue*> {

	MessageReader<PacketPriorityQueue*>*& groupReader = Messageable<PacketPriorityQueue*>::msgReader;

private:
	
	wxImage _image;
	Client* client;
	std::thread clientThr;

	bool _connected = false;
	bool _skip = true;

public:
	ClientStreamWindow(const std::string& ip, int localPort, int remotePort);
	~ClientStreamWindow();

	// Delete copy and move constructors and assignment operators
	ClientStreamWindow(const ClientStreamWindow&) = delete;
	ClientStreamWindow(ClientStreamWindow&&) = delete;
	ClientStreamWindow& operator=(const ClientStreamWindow&) = delete;
	ClientStreamWindow& operator=(ClientStreamWindow&&) = delete;

	void OnPaint(wxPaintEvent& paintEvent);
	bool AssembleImage();
	void PaintNow();
	void OnIdle(wxIdleEvent& evt);

	constexpr const WindowNames WindowName() override { return WindowNames::ClientStream; }

	wxDECLARE_EVENT_TABLE();
};


//--------------Server Init Window Class-----------------------------//

class ServerInitWindow : public BaseWindow {

private:
	wxTextCtrl* _portTb;
	Button* _listenButton;

	//Constructor and destructor
public:
	ServerInitWindow();
	~ServerInitWindow();

	// Delete copy and move constructors and assignment operators
	ServerInitWindow(const ServerInitWindow&) = delete;
	ServerInitWindow(ServerInitWindow&&) = delete;
	ServerInitWindow& operator=(const ServerInitWindow&) = delete;
	ServerInitWindow& operator=(ServerInitWindow&&) = delete;
	
	void ListenButtonClick(wxCommandEvent& evt);
	
	constexpr const WindowNames WindowName() override { return WindowNames::ServerInit; }

	wxDECLARE_EVENT_TABLE();
};