#pragma once
#include "Types.h"
#include <stack>
#include "wx/wx.h"
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

using TextBox = wxTextCtrl;
using Button = wxButton;

//--------------Abstract Base Window Class-----------------------------//

class BaseWindow : public wxFrame
{
public:
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
	
	// All Window controls on the current window
	ElementList _windowElements;
	const int _windowId;

	// Flag indicating whether the program whould quit on window close
	bool _killProgramOnClose = true;

	virtual constexpr const WindowNames WindowName() = 0;

};


//--------------Start Up Window Class-----------------------------//

class StartUpWindow : public BaseWindow {
private:
	Button* _clientButton;
	Button* _serverButton;
	
public:
	StartUpWindow();
	~StartUpWindow();

	void ClientButtonClick(wxCommandEvent& evt);
	void ServerButtonClick(wxCommandEvent& evt);

	constexpr const WindowNames WindowName() override { return WindowNames::StartUp; }

	wxDECLARE_EVENT_TABLE();
};


//--------------Client Init Window Class-----------------------------//

class ClientInitWindow : public BaseWindow {

private:

	const int PORT_TB_ID = 20003;
	const int IP_TB_IP = 20002;
	
	TextBox* _remotePortInput;
	TextBox* _localPortInput;
	TextBox* _ipInput;
	Button* _connectButton;

	//Constructor and destructor
	public:
		ClientInitWindow();
		~ClientInitWindow();

		void ConnectButtonClick(wxCommandEvent& evt);

		void HandleInput(wxKeyEvent& kehgyEvent) override;

		constexpr const WindowNames WindowName() override { return WindowNames::ClientInit; }

		wxDECLARE_EVENT_TABLE();
};


//--------------Client Stream Window Class-----------------------------//
// Receives a video stream from the server and sends 
// back a stream of keyboard and mouse events

class ClientStreamWindow : public BaseWindow {
public:
	ClientStreamWindow(const std::string& ip, int localPort, int remotePort);
	~ClientStreamWindow();

	constexpr const WindowNames WindowName() override { return WindowNames::ClientStream; }
};


//--------------Server Init Window Class-----------------------------//

class ServerInitWindow : public BaseWindow {

private:
	TextBox* _portTb;
	Button* _listenButton;

	//Constructor and destructor
	public:
		ServerInitWindow();
		~ServerInitWindow();
		
		void ListenButtonClick(wxCommandEvent& evt);
		void HandleInput(wxKeyEvent& keyEvent) override;
		
		wxDECLARE_EVENT_TABLE();

		constexpr const WindowNames WindowName() override { return WindowNames::ServerInit; };
};