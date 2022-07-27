#pragma once
#include "wx/wx.h"

class BaseControl : public wxControl {
protected:
	virtual void HandleInput(wxKeyEvent& keyEvent);
};

class TextBox : public BaseControl {
	
};