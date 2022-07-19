#pragma once
#include "Types.h"
#include <stack>
#include "wx/wx.h"

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

class BaseWindow : public wxFrame
{
public:
	virtual void HandleInput(wxKeyEvent& keyEvent);

	wxTextValidator IP_VALIDATOR = wxTextValidator(wxFILTER_INCLUDE_CHAR_LIST);
	wxTextValidator PORT_VALIDATOR = wxTextValidator(wxFILTER_DIGITS);
protected:

	static WindowStack _prevWindows;

	BaseWindow(const std::string& name);
	virtual ~BaseWindow();
	
	ElementList _windowElements;
	const int _windowId;

	bool _killProgramOnClose = true;

	virtual constexpr const WindowNames WindowName() = 0;

};


class StartUpWindow : public BaseWindow {
private:
	Button* clientButton;
	Button* serverButton;
	
public:
	StartUpWindow();
	~StartUpWindow();

	void ClientButtonClick(wxCommandEvent& evt);
	void ServerButtonClick(wxCommandEvent& evt);

	constexpr const WindowNames WindowName() override;

	wxDECLARE_EVENT_TABLE();
};

class ClientInitWindow : public BaseWindow {

private:

	const int PORT_TB_ID = 20003;
	const int IP_TB_IP = 20002;
	

	TextBox* _portInput;
	TextBox* _ipInput;
	Button* _connectButton;

	//Constructor and destructor
	public:
		ClientInitWindow();
		~ClientInitWindow();

		void ConnectButtonClick(wxCommandEvent& evt);

		void HandleInput(wxKeyEvent& keyEvent) override;

		constexpr const WindowNames WindowName() override;

		wxDECLARE_EVENT_TABLE();
};
//
//class ServerInitWindow : public BaseWindow {
//
//	//Constructor and destructor
//	public:
//		ServerInitWindow();
//		~ServerInitWindow();
//
//		constexpr const WindowNames WindowName() override;
//};