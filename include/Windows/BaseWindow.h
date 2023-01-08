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

class PopUp;
using PopUpPtr = std::unique_ptr<PopUp>;

class BaseWindow : public wxFrame
{

protected:

	bool _init = false;

	enum class Asset {
		ICON
	};

	static string GetAssetPath(Asset asset) {

		static const auto GetAssetDirectory = []() {
			ifstream asset_file("asset_dir.txt");

			string directory = "";
			asset_file >> directory;

			return directory;
		};

		static const string AssetDirectory = GetAssetDirectory();

		switch (asset) {
		case Asset::ICON:
		default:
			return AssetDirectory + "/logo.png";
		}
	}

	static wxIcon inline FetchIcon() {

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
	static inline const wxPoint DEFAULT_POS = wxPoint(100, 100);
	static inline const wxSize DEFAULT_SIZE = wxSize(ScreenCapture::DefaultResolution.width, ScreenCapture::DefaultResolution.height);

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
	Action onClose;
	static const inline wxSize POPUP_SIZE = wxSize(300, 200);

	wxStaticText* _text;
	wxButton* _dismissButton;

	wxDECLARE_ABSTRACT_CLASS(PopUp);
	wxDECLARE_EVENT_TABLE();

};