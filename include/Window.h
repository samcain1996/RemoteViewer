#pragma once
#include "Types.h"
#include "wx/wx.h"

class GenericWindow : public wxFrame
{

protected:
	std::vector<wxControl*> _windowElements;
	const int _windowId;
	GenericWindow();

	virtual ~GenericWindow();

};


class StartUpWindow : public GenericWindow {
public:
	StartUpWindow();
	~StartUpWindow();

	void ClientButtonClick(wxCommandEvent& evt);
	void ServerButtonClick(wxCommandEvent& evt);

	wxDECLARE_EVENT_TABLE();
};