#pragma once 
#include "RemoteViewerWindows.h"

class Application : public wxApp  {
public:
	Application();
	~Application();

private:
	BaseWindow* _window = new StartUpWindow();

public:
	virtual bool OnInit();
};
