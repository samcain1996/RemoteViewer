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

class BaseWindow : public wxFrame
{
public:
	virtual void BackSpace(wxKeyEvent& keyEvent);
protected:
	BaseWindow(const std::string& name);
	virtual ~BaseWindow();
	
	std::vector<wxControl*> _windowElements;
	const int _windowId;

	static std::stack<WindowNames> _prevWindows;

	virtual constexpr const WindowNames WindowName() = 0;

};


class StartUpWindow : public BaseWindow {
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
	wxTextCtrl* _portInput;
	wxTextCtrl* _ipInput;
	wxButton* _connectButton;

	//Constructor and destructor
	public:
		ClientInitWindow();
		~ClientInitWindow();

		void ConnectButtonClick(wxCommandEvent& evt);

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