#pragma once

#include <stack>

#include "wx/wx.h"
#include "wx/filesys.h"
#include "wx/mstream.h"
#include "wx/popupwin.h"

#include "QuickShot/Capture.h"

enum class WindowNames {
	StartUp,
	ClientInit,
	ServerInit,
	ClientStream,
	UNDEFINED
};

using std::ifstream;
using std::string;
using std::ios;

using WindowStack = std::stack<WindowNames>;
using ElementList = std::vector<wxControl*>;
using SizeAndPos = std::pair<wxPoint, wxSize>;

class PopUp;
using PopUpPtr = std::unique_ptr<PopUp>;

class BaseWindow : public wxFrame
{

	BaseWindow* SpawnWindow(const WindowNames windowName);

protected:

	bool _init = false;

	enum class Asset {
		ICON
	};

	static string GetAssetPath(Asset asset) {

		static const string AssetDirectory = "assets";

		switch (asset) {
		case Asset::ICON:
		default:
			return AssetDirectory + "/logo.png";
		}
	}

	static wxIcon FetchIcon() {

		static const auto GetIcon = []() {

			wxFSInputStream inputStream(GetAssetPath(Asset::ICON));
			wxImage image(inputStream);
			wxBitmap bitmap(image);


			wxIcon icon;
			icon.CopyFromBitmap(bitmap);
			return icon;
		};

		return GetIcon();
	}

	static void CenterElements(ElementList& elements) {

		if (elements.empty()) { return; }

		const wxSize parentSize = elements[0]->GetParent()->GetSize();

		const int minX = (*std::min_element(elements.begin(), elements.end(), [](const auto& control, const auto& otherControl) {
			return control->GetPosition().x < otherControl->GetPosition().x;
			}))->GetPosition().x;
		const int minY = (*std::min_element(elements.begin(), elements.end(), [](const auto& control, const auto& otherControl) {
			return control->GetPosition().y < otherControl->GetPosition().y;
			}))->GetPosition().y;

		const int maxX = (*std::max_element(elements.begin(), elements.end(), [](const auto& control, const auto& otherControl) {
			return control->GetPosition().x + control->GetSize().GetWidth() > otherControl->GetPosition().x + otherControl->GetSize().GetWidth();
			}))->GetPosition().x;
		const int maxY = (*std::max_element(elements.begin(), elements.end(), [](const auto& control, const auto& otherControl) {
			return control->GetPosition().y + control->GetSize().GetHeight() > otherControl->GetPosition().y + otherControl->GetSize().GetHeight();
			}))->GetPosition().y;

		const int targetX = 0.5 * parentSize.GetWidth() - 0.5 * (maxX - minX);
		const int targetY = 0.5 * parentSize.GetHeight() - 0.5 * (maxY - minY);

		std::for_each(elements.begin(), elements.end(), [=](auto& control) {
			int diffX = control->GetPosition().x - minX;
			int diffY = control->GetPosition().y - minY;

			control->Move(targetX - diffX, targetY - diffY);
			});

	}
	
public:

	// Handle events shared across all windows such as going back to the previous window
	virtual void HandleInput(wxKeyEvent& keyEvent);

	// Textbox validators used
	wxTextValidator IP_VALIDATOR = wxTextValidator(wxFILTER_INCLUDE_CHAR_LIST);
	wxTextValidator PORT_VALIDATOR = wxTextValidator(wxFILTER_DIGITS);
	virtual ~BaseWindow();

protected:
	
	// Previous Windows
	static inline WindowStack _prevWindows {};
	static inline const wxSize DEFAULT_SIZE = wxSize(ScreenCapture::DefaultResolution.width, ScreenCapture::DefaultResolution.height);
	static inline const wxPoint DEFAULT_POS = wxPoint(ScreenCapture::NativeResolution().width / 2 - DEFAULT_SIZE.GetWidth() / 2,
		ScreenCapture::NativeResolution().height / 2 - DEFAULT_SIZE.GetHeight() / 2);

	BaseWindow(const std::string& name, const wxPoint& pos = DEFAULT_POS,
		const wxSize& size = DEFAULT_SIZE, const bool show = true);

	// Delete copy and move constructors and assignment operators
	BaseWindow(const BaseWindow&) = delete;
	BaseWindow(BaseWindow&&) = delete;
	BaseWindow& operator=(const BaseWindow&) = delete;
	BaseWindow& operator=(BaseWindow&&) = delete;

	// All Window controls on the current window
	ElementList _windowElements;
	const int _windowId = -1;

	PopUpPtr _popup;

	void GoBack();
	void OpenWindow(const WindowNames windowName);

	virtual void CleanUp() {};
	virtual constexpr const WindowNames WindowName() = 0;

};

class PopUp : public wxPopupTransientWindow {

public:

	PopUp(BaseWindow* parent, const std::string& message, Action&& OnClose = []() {});
	~PopUp();

	// Delete copy and move constructors and assignment operators
	PopUp(const PopUp&) = delete;
	PopUp(PopUp&&) = delete;
	PopUp& operator=(const PopUp&) = delete;
	PopUp& operator=(PopUp&&) = delete;

	void OnButton(wxCommandEvent& evt);
	void OnDismiss() override;

private:
	static const inline wxSize POPUP_SIZE = wxSize(300, 200);
	
	Action onClose;

	wxStaticText* _text;
	wxButton* _dismissButton;

	wxDECLARE_ABSTRACT_CLASS(PopUp);
	wxDECLARE_EVENT_TABLE();

};