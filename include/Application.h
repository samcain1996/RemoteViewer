#pragma once 
#include "Window.h"

class Application : public wxApp  {
public:
	Application();
	~Application();

private:
	wxFrame* _window = new StartUpWindow();

public:
	virtual bool OnInit();
};
