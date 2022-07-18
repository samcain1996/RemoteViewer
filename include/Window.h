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

class GenericWindow : public wxFrame
{
public:
	virtual void OnChar(wxKeyEvent& keyEvent);
protected:
	GenericWindow(const std::string& name);
	virtual ~GenericWindow();
	
	std::vector<wxControl*> _windowElements;
	const int _windowId;

	static std::stack<WindowNames> _prevWindows;

	virtual constexpr const WindowNames WindowName() = 0;

};


class StartUpWindow : public GenericWindow {
public:
	StartUpWindow();
	~StartUpWindow();

	void ClientButtonClick(wxCommandEvent& evt);
	void ServerButtonClick(wxCommandEvent& evt);

	constexpr const WindowNames WindowName() override;

	wxDECLARE_EVENT_TABLE();
};

class ClientInitWindow : public GenericWindow {

	//Constructor and destructor
	public:
		ClientInitWindow();
		~ClientInitWindow();

		wxDECLARE_EVENT_TABLE();

		constexpr const WindowNames WindowName() override;
};
//
//class ServerInitWindow : public GenericWindow {
//
//	//Constructor and destructor
//	public:
//		ServerInitWindow();
//		~ServerInitWindow();
//
//		constexpr const WindowNames WindowName() override;
//};