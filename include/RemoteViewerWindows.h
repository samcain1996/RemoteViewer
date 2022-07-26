#pragma once
#include "Types.h"
#include <stack>
#include "wx/wx.h"
#include "wx/mstream.h"
#include "Client.h"
#include "Server.h"
#include "wx/popupwin.h"

class PopUp;

enum class WindowNames {
	StartUp,
	ClientInit,
	ServerInit,
	ClientStream,
	UNDEFINED
};

using WindowStack = std::stack<WindowNames>;
using ElementList = std::vector<wxControl*>;

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
	static inline const wxPoint DEFAULT_POS = wxPoint(100, 100);
	static inline const wxSize DEFAULT_SIZE = wxSize(1280, 720);

	BaseWindow(const std::string& name, const wxPoint& pos = DEFAULT_POS,
		const wxSize& size = DEFAULT_SIZE);
	virtual ~BaseWindow();

	// Delete copy and move constructors and assignment operators
	BaseWindow(const BaseWindow&) = delete;
	BaseWindow(BaseWindow&&) = delete;
	BaseWindow& operator=(const BaseWindow&) = delete;
	BaseWindow& operator=(BaseWindow&&) = delete;
	
	// All Window controls on the current window
	ElementList _windowElements;
	const int _windowId = -1;

	// Flag indicating whether the program whould quit on window close
	bool _killProgramOnClose = true;

	virtual void BackgroundTask(wxIdleEvent& evt) {}
	virtual constexpr const WindowNames WindowName() = 0;

};


//--------------Start Up Window Class-----------------------------//

class StartUpWindow : public BaseWindow {
private:
	wxButton* _clientButton;
	wxButton* _serverButton;
	
	const int _windowId = 1;
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
	
	wxTextCtrl* _remotePortInput;
	wxTextCtrl* _localPortInput;
	wxTextCtrl* _ipInput;
	wxButton* _connectButton;

	PopUp* _popUp = nullptr;

	Client* _client = nullptr;
	bool _connected = false;

	const int _windowId = 2;

public:
	ClientInitWindow(const wxPoint& pos = DEFAULT_POS, const wxSize& size = DEFAULT_SIZE);
	~ClientInitWindow();

	// Delete copy and move constructors and assignment operators
	ClientInitWindow(const ClientInitWindow&) = delete;
	ClientInitWindow(ClientInitWindow&&) = delete;
	ClientInitWindow& operator=(const ClientInitWindow&) = delete;
	ClientInitWindow& operator=(ClientInitWindow&&) = delete;

	void ConnectButtonClick(wxCommandEvent& evt);
	void BackgroundTask(wxIdleEvent& evt) override;

	constexpr const WindowNames WindowName() override { return WindowNames::ClientInit; }

	wxDECLARE_EVENT_TABLE();
};


//--------------Client Stream Window Class-----------------------------//
// Receives a video stream from the server and sends 
// back a stream of keyboard and mouse events

class ClientStreamWindow : public BaseWindow, public Messageable<PacketPriorityQueue*> {

	MessageReader<PacketPriorityQueue*>*& groupReader = msgReader;  // Queue of packets that can create a complete image

private:
	
	ByteArray _imgData = nullptr;
	int _imgSize = 0;
	
	Client* _client;
	std::thread _clientThr;

	const int _windowId = 3;

public:
	ClientStreamWindow(Client* client, 
		const wxPoint& pos = DEFAULT_POS, const wxSize& size = DEFAULT_SIZE);
	~ClientStreamWindow();

	// Delete copy and move constructors and assignment operators
	ClientStreamWindow(const ClientStreamWindow&) = delete;
	ClientStreamWindow(ClientStreamWindow&&) = delete;
	ClientStreamWindow& operator=(const ClientStreamWindow&) = delete;
	ClientStreamWindow& operator=(ClientStreamWindow&&) = delete;

	const bool AssembleImage();  // Assemble image from packet queue, return true if successful
	
	void OnPaint(wxPaintEvent& evt);
	
	void PaintNow();
	void OnIdle(wxIdleEvent& evt);

	constexpr const WindowNames WindowName() override { return WindowNames::ClientStream; }

	wxDECLARE_EVENT_TABLE();
};


//--------------Server Init Window Class-----------------------------//

class ServerInitWindow : public BaseWindow {

private:
	wxTextCtrl* _portTb;
	wxButton* _startServerButton;

	const int _windowId = 4;

	// Constructor and destructor
public:
	ServerInitWindow(const wxPoint& pos, const wxSize& size);
	~ServerInitWindow();

	// Delete copy and move constructors and assignment operators
	ServerInitWindow(const ServerInitWindow&) = delete;
	ServerInitWindow(ServerInitWindow&&) = delete;
	ServerInitWindow& operator=(const ServerInitWindow&) = delete;
	ServerInitWindow& operator=(ServerInitWindow&&) = delete;
	
	void StartServer(wxCommandEvent& evt);
	
	constexpr const WindowNames WindowName() override { return WindowNames::ServerInit; }

	wxDECLARE_EVENT_TABLE();
};


class PopUp : public wxPopupTransientWindow {
public:
	PopUp(BaseWindow* parent, const std::string& message, 
		std::function<void(wxIdleEvent&)> onIdle = [](wxIdleEvent&) {});
	~PopUp();

	// Delete copy and move constructors and assignment operators
	PopUp(const PopUp&) = delete;
	PopUp(PopUp&&) = delete;
	PopUp& operator=(const PopUp&) = delete;
	PopUp& operator=(PopUp&&) = delete;

	void OnButton(wxCommandEvent& evt);
	//void BackgroundTask(wxIdleEvent& evt);

private:

	const inline static wxSize POPUP_SIZE = wxSize(300, 200);
	wxStaticText* _text;
	wxButton* _dismissButton;

	//std::function<void(wxIdleEvent& evt)> _backgroundTask;

	wxDECLARE_ABSTRACT_CLASS(PopUp);
	wxDECLARE_EVENT_TABLE();
};