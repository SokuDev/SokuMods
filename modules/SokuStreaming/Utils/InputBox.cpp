//
// Created by PinkySmile on 09/12/2020.
//

#include "InputBox.hpp"

// Windows API
#include <windows.h>

// VBScript InputBox
#include <activscp.h>
#include <atlbase.h>
#include <comdef.h>

// UTF-8 Support
#include <string>
#include <vector>
#include <wchar.h>

using std::string;
using std::vector;

typedef std::basic_string<wchar_t> tstring;

static tstring StringWiden(string Str) {
	const size_t wchar_tCount = Str.size() + 1;

	vector<wchar_t> Buffer(wchar_tCount);

	return tstring{Buffer.data(), (size_t)MultiByteToWideChar(CP_UTF8, 0, Str.c_str(), -1, Buffer.data(), wchar_tCount)};
}

static string StringShorten(tstring Str) {
	int nBytes = (size_t)WideCharToMultiByte(CP_UTF8, 0, Str.c_str(), (int)Str.length(), NULL, 0, NULL, NULL);

	vector<char> Buffer((size_t)nBytes);

	return string{Buffer.data(), (size_t)WideCharToMultiByte(CP_UTF8, 0, Str.c_str(), (int)Str.length(), Buffer.data(), nBytes, NULL, NULL)};
}

static string StringReplaceAll(string Str, string SubStr, string NewStr) {
	size_t Position = 0;
	const size_t SubLen = SubStr.length(), NewLen = NewStr.length();

	while ((Position = Str.find(SubStr, Position)) != string::npos) {
		Str.replace(Position, SubLen, NewStr);
		Position += NewLen;
	}

	return Str;
}

static string CPPNewLineToVBSNewLine(string NewLine) {
	size_t Position = 0;

	while (Position < NewLine.length()) {
		if (NewLine[Position] == '\n' || NewLine[Position] == '\r')
			NewLine.replace(Position, 2, "\" + vbNewLine + \"");

		Position += 1;
	}

	return NewLine;
}

class CSimpleScriptSite: public IActiveScriptSite, public IActiveScriptSiteWindow {
public:
	CSimpleScriptSite(): m_cRefCount(1), m_hWnd(NULL) {}

	// IUnknown

	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();
	STDMETHOD(QueryInterface)(REFIID riid, void **ppvObject);

	// IActiveScriptSite

	STDMETHOD(GetLCID)(LCID *plcid) {
		*plcid = 0;
		return S_OK;
	}
	STDMETHOD(GetItemInfo)(LPCOLESTR pstrName, DWORD dwReturnMask, IUnknown **ppiunkItem, ITypeInfo **ppti) {
		return TYPE_E_ELEMENTNOTFOUND;
	}
	STDMETHOD(GetDocVersionString)(BSTR *pbstrVersion) {
		*pbstrVersion = SysAllocString(L"1.0");
		return S_OK;
	}
	STDMETHOD(OnScriptTerminate)(const VARIANT *pvarResult, const EXCEPINFO *pexcepinfo) {
		return S_OK;
	}
	STDMETHOD(OnStateChange)(SCRIPTSTATE ssScriptState) {
		return S_OK;
	}
	STDMETHOD(OnScriptError)(IActiveScriptError *pIActiveScriptError) {
		return S_OK;
	}
	STDMETHOD(OnEnterScript)(void) {
		return S_OK;
	}
	STDMETHOD(OnLeaveScript)(void) {
		return S_OK;
	}

	// IActiveScriptSiteWindow

	STDMETHOD(GetWindow)(HWND *phWnd) {
		*phWnd = m_hWnd;
		return S_OK;
	}
	STDMETHOD(EnableModeless)(BOOL fEnable) {
		return S_OK;
	}

	// Miscellaneous

	STDMETHOD(SetWindow)(HWND hWnd) {
		m_hWnd = hWnd;
		return S_OK;
	}

public:
	LONG m_cRefCount;
	HWND m_hWnd;
};

STDMETHODIMP_(ULONG) CSimpleScriptSite::AddRef() {
	return InterlockedIncrement(&m_cRefCount);
}

STDMETHODIMP_(ULONG) CSimpleScriptSite::Release() {
	if (!InterlockedDecrement(&m_cRefCount)) {
		delete this;
		return 0;
	}
	return m_cRefCount;
}

STDMETHODIMP CSimpleScriptSite::QueryInterface(REFIID riid, void **ppvObject) {
	if (riid == IID_IUnknown || riid == IID_IActiveScriptSiteWindow) {
		*ppvObject = (IActiveScriptSiteWindow *)this;
		AddRef();
		return NOERROR;
	}
	if (riid == IID_IActiveScriptSite) {
		*ppvObject = (IActiveScriptSite *)this;
		AddRef();
		return NOERROR;
	}
	return E_NOINTERFACE;
}

static HHOOK hHook = 0;
static bool HideInput = false;

static LRESULT CALLBACK InputBoxProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode < HC_ACTION)
		return CallNextHookEx(hHook, nCode, wParam, lParam);

	if (nCode = HCBT_ACTIVATE) {
		if (HideInput == true) {
			HWND TextBox = FindWindowExA((HWND)wParam, NULL, "Edit", NULL);
			SendDlgItemMessage((HWND)wParam, GetDlgCtrlID(TextBox), EM_SETPASSWORDCHAR, '*', 0);
		}
	}

	if (nCode = HCBT_CREATEWND) {
		if (!(GetWindowLongPtr((HWND)wParam, GWL_STYLE) & WS_CHILD))
			SetWindowLongPtr((HWND)wParam, GWL_EXSTYLE, GetWindowLongPtr((HWND)wParam, GWL_EXSTYLE) | WS_EX_DLGMODALFRAME);
	}

	return CallNextHookEx(hHook, nCode, wParam, lParam);
}

static std::string InputBoxHelper(const std::string &Prompt, const std::string &Title, const std::string &Default) {
	HRESULT hr = S_OK;
	hr = CoInitialize(NULL);

	// Initialize
	CSimpleScriptSite *pScriptSite = new CSimpleScriptSite();
	CComPtr<IActiveScript> spVBScript;
	CComPtr<IActiveScriptParse> spVBScriptParse;
	hr = spVBScript.CoCreateInstance(OLESTR("VBScript"));
	hr = spVBScript->SetScriptSite(pScriptSite);
	hr = spVBScript->QueryInterface(&spVBScriptParse);
	hr = spVBScriptParse->InitNew();

	// Replace quotes with double quotes
	string strPrompt = StringReplaceAll(Prompt, "\"", "\"\"");
	string strTitle = StringReplaceAll(Title, "\"", "\"\"");
	string strDefault = StringReplaceAll(Default, "\"", "\"\"");

	// Create evaluation string
	string Evaluation = "InputBox(\"" + strPrompt + "\", \"" + strTitle + "\", \"" + strDefault + "\")";
	Evaluation = CPPNewLineToVBSNewLine(Evaluation);
	tstring WideEval = StringWiden(Evaluation);

	// Run InpuBox
	CComVariant result;
	EXCEPINFO ei = {};

	DWORD ThreadID = GetCurrentThreadId();
	HINSTANCE ModHwnd = GetModuleHandle(NULL);
	hr = pScriptSite->SetWindow(GetAncestor(GetActiveWindow(), GA_ROOTOWNER));
	hHook = SetWindowsHookEx(WH_CBT, &InputBoxProc, ModHwnd, ThreadID);
	hr = spVBScriptParse->ParseScriptText(WideEval.c_str(), NULL, NULL, NULL, 0, 0, SCRIPTTEXT_ISEXPRESSION, &result, &ei);
	UnhookWindowsHookEx(hHook);

	// Cleanup
	spVBScriptParse = NULL;
	spVBScript = NULL;
	pScriptSite->Release();
	pScriptSite = NULL;

	CoUninitialize();
	static string strResult;
	_bstr_t bstrResult = (_bstr_t)result;
	strResult = StringShorten((wchar_t *)bstrResult);
	return strResult;
}

std::string InputBox(const std::string &Prompt, const std::string &Title, const std::string &Default) {
	HideInput = false;

	return InputBoxHelper(Prompt, Title, Default);
}

std::string PasswordBox(const std::string &Prompt, const std::string &Title, const std::string &Default) {
	HideInput = true;

	return InputBoxHelper(Prompt, Title, Default);
}