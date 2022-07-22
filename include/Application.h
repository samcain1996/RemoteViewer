#pragma once 
#include "RemoteViewerWindows.h"

class Application : public wxApp  {
public:
	Application();
	~Application();

private:
	BaseWindow* _window = nullptr;

public:
	virtual bool OnInit() wxOVERRIDE;

};
