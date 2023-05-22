#pragma once

#include <stack>
#include <tuple>

#include "wx/wx.h"
#include "wx/filesys.h"
#include "wx/mstream.h"
#include "wx/popupwin.h"

#include "Config.h"

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

using Element = wxControl;
using ElementList = std::vector<Element*>;
using Bounds = std::tuple<int, int, int, int>;

class PopUp;
using PopUpPtr = std::unique_ptr<PopUp>;

static Bounds GetRelativeBounds(const ElementList& elements) {

	// Element furthest left
	const auto minX = std::min_element(elements.begin(), elements.end(), [](const auto& element, const auto& other) {
		return element->GetPosition().x < other->GetPosition().x;
	});

	// Element furthest up
	const auto minY = std::min_element(elements.begin(), elements.end(), [](const auto& element, const auto& other) {
		return element->GetPosition().y < other->GetPosition().y;
	});

	// Element furthest right
	const auto maxX = std::max_element(elements.begin(), elements.end(), [](const auto& element, const auto& other) {
		return element->GetPosition().x + element->GetSize().GetWidth() < other->GetPosition().x + other->GetSize().GetWidth();
	});

	// Element furthest down
	const auto maxY = std::max_element(elements.begin(), elements.end(), [](const auto& element, const auto& other) {
		return element->GetPosition().y + element->GetSize().GetHeight() < other->GetPosition().y + other->GetSize().GetHeight();
	});

	return Bounds
	{
		(*minX)->GetPosition().x,								   // min X
		(*maxX)->GetPosition().x + (*maxX)->GetSize().GetWidth(),  // max X
		(*minY)->GetPosition().y,								   // min Y
		(*maxY)->GetPosition().y + (*maxY)->GetSize().GetHeight()  // max Y
	};
}

static void CenterElements(ElementList& elements, const wxSize& padding = wxSize(0, 0)) {

	if (elements.empty()) { return; }

	const wxSize PARENT_SIZE = elements.front()->GetParent()->GetSize();

	// Amount of screen covered by elements
	const auto [left, right, top, bottom] = GetRelativeBounds(elements);

	// Margin between window edges and position of elements
	const wxPoint OFFSET 
	{ 
		( PARENT_SIZE.GetWidth()  - (right - left) ) / 2,
		( PARENT_SIZE.GetHeight() - (bottom - top) ) / 2
	};

	for_each(elements.begin(), elements.end(), [x_1 = left, y_1 = top, OFFSET](auto& element) {
		
		const wxPoint RELATIVE_OFFSET
		{
			// Distance this element is to the one that is furthest left
			element->GetPosition().x - x_1,

			// Distance this element is to the one that is furthest up
			element->GetPosition().y - y_1
		};

		// Top left corner of this element should be OFFSET + RELATIVE_OFFSET
		element->SetPosition(OFFSET + RELATIVE_OFFSET);

	});
}

class BaseWindow : public wxFrame
{

protected:

	bool _initialized = false;

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

		wxFSInputStream inputStream(GetAssetPath(Asset::ICON));
		wxImage image(inputStream);
		wxBitmap bitmap(image);


		wxIcon icon;
		icon.CopyFromBitmap(bitmap);
		return icon;

	}

	
	
public:

	void OpenWindow(const WindowNames windowName, const string& ip = "", const bool close = true);

	// Handle events shared across all windows such as going back to the previous window
	virtual void HandleInput(wxKeyEvent& keyEvent);

	// Textbox validators used
	wxTextValidator IP_VALIDATOR = wxTextValidator(wxFILTER_INCLUDE_CHAR_LIST);
	virtual ~BaseWindow();

protected:
	
	// Previous Windows
	static inline WindowStack _prevWindows {};
	
	static inline const wxSize DEFAULT_SIZE = wxSize(ScreenCapture::DefaultResolution.width, ScreenCapture::DefaultResolution.height);
	static inline const wxSize MINIMIZED_SIZE = DEFAULT_SIZE * 0.25;
	static inline const wxPoint DEFAULT_POS = wxPoint(ScreenCapture::NativeResolution().width / 2 - DEFAULT_SIZE.GetWidth() / 2,
		ScreenCapture::NativeResolution().height / 2 - DEFAULT_SIZE.GetHeight() / 2);

	BaseWindow(const string& name, const wxPoint& pos = DEFAULT_POS,
		const wxSize& size = DEFAULT_SIZE, const bool show = true);

	// Delete copy and move constructors and assignment operators
	BaseWindow(const BaseWindow&) = delete;
	BaseWindow(BaseWindow&&) = delete;
	BaseWindow& operator=(const BaseWindow&) = delete;
	BaseWindow& operator=(BaseWindow&&) = delete;

	// All Window elements on the current window
	ElementList _windowElements;

	PopUpPtr _popup;

	void GoBack();

	virtual constexpr const WindowNames WindowName() = 0;
	virtual void CleanUp();

};

class PopUp : public wxPopupTransientWindow {

public:

	PopUp(BaseWindow* parent, const string& message = string(), const Action& OnClose = []() {});
	~PopUp();

	// Delete copy and move constructors and assignment operators
	PopUp(const PopUp&) = delete;
	PopUp(PopUp&&) = delete;
	PopUp& operator=(const PopUp&) = delete;
	PopUp& operator=(PopUp&&) = delete;

	void OnButton(wxCommandEvent& evt);
	void OnDismiss() override;
	void SetText(const std::string& text) { _text->Clear(); _text->AppendText(text); };

private:

	static const inline wxSize DEF_POPUP_SIZE = wxSize(RES_480.width, RES_480.height);
	
	Action onClose;

	wxTextCtrl* _text;
	wxButton* _dismissButton;

	wxDECLARE_ABSTRACT_CLASS(PopUp);
	wxDECLARE_EVENT_TABLE();

};