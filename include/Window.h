#pragma once
#include "Types.h"
#include "wx/wx.h"

class GenericWindow : public wxFrame
{

protected:
	std::vector<wxControl*> _windowElements;
	GenericWindow() : wxFrame(nullptr, wxID_ANY, "Remote Viewer", wxPoint(50, 50), wxSize(1270, 720)) {}

	virtual ~GenericWindow();

};


class StartUpWindow : public GenericWindow {
public:
	StartUpWindow();
	~StartUpWindow();
};