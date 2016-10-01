#include "stdafx.h"
#include "WinToastDllImporter.h"
#define DEFAULT_SHELL_LINKS_PATH	L"\\Microsoft\\Windows\\Start Menu\\Programs\\"
#define DEFAULT_LINK_FORMAT			L".lnk"

namespace WinToastUtil {

	extern WINTOASTLIB_API inline HRESULT defaultExecutablePath(_In_ WCHAR* path, _In_ DWORD nSize = MAX_PATH) {
		DWORD written = GetModuleFileNameEx(GetCurrentProcess(), nullptr, path, nSize);
		return (written > 0) ? S_OK : E_FAIL;
	}


	extern WINTOASTLIB_API inline HRESULT defaultShellLinksDirectory(_In_ WCHAR* path, _In_ DWORD nSize = MAX_PATH) {
		DWORD written = GetEnvironmentVariable(L"APPDATA", path, nSize);
		HRESULT hr = written > 0 ? S_OK : E_INVALIDARG;
		if (SUCCEEDED(hr)) {
			errno_t result = wcscat_s(path, nSize, DEFAULT_SHELL_LINKS_PATH);
			hr = (result == 0) ? S_OK : E_INVALIDARG;
		}
		return hr;
	}

	extern WINTOASTLIB_API inline HRESULT defaultShellLinkPath(const std::wstring& appname, _In_ WCHAR* path, _In_ DWORD nSize = MAX_PATH) {
		HRESULT hr = defaultShellLinksDirectory(path, nSize);
		if (SUCCEEDED(hr)) {
			const wstring appLink(appname + DEFAULT_LINK_FORMAT);
			errno_t result = wcscat_s(path, nSize, appLink.c_str());
			hr = (result == 0) ? S_OK : E_INVALIDARG;
		}
		return hr;
	}


	extern WINTOASTLIB_API inline HSTRING	loadStringReference(_In_ std::wstring data) {
		HSTRING_HEADER hstringHeader;
		HSTRING string;
		HRESULT hr = WinToastDllImporter::WindowsCreateStringReference(data.c_str(), static_cast<UINT32>(data.length()), &hstringHeader, &string);
		if (FAILED(hr)) {
			RaiseException(static_cast<DWORD>(STATUS_INVALID_PARAMETER), EXCEPTION_NONCONTINUABLE, 0, nullptr);
		}
		return string;
	}

	extern WINTOASTLIB_API inline HSTRING loadStringReference(_In_reads_(length) PCWSTR stringRef, _In_ UINT32 length) throw()
	{
	
		HSTRING_HEADER hstringHeader;
		HSTRING string; 
		HRESULT hr = WinToastDllImporter::WindowsCreateStringReference(stringRef, length, &hstringHeader, &string);
		if (FAILED(hr)) {
			RaiseException(static_cast<DWORD>(STATUS_INVALID_PARAMETER), EXCEPTION_NONCONTINUABLE, 0, nullptr);
		}
		return string;
	}


	extern WINTOASTLIB_API inline HRESULT getNodeListByTag(const std::wstring tag, ComPtr<IXmlNodeList>& nodeList, IXmlDocument *xml) {
		return xml->GetElementsByTagName(loadStringReference(tag), &nodeList);
	}

	extern WINTOASTLIB_API inline HRESULT getNodeFromNodeList(ComPtr<IXmlNodeList>& nodeList, ComPtr<IXmlNode>& node, int pos) {
		ComPtr<IXmlNode> textNode;
		return nodeList->Item(pos, &node);
	}

	extern WINTOASTLIB_API inline HRESULT setNodeStringValue(const HSTRING &string, IXmlNode *node, IXmlDocument *xml) {
		ComPtr<IXmlText> textNode;
		HRESULT hr = xml->CreateTextNode(string, &textNode);
		if (SUCCEEDED(hr)) {
			ComPtr<IXmlNode> stringNode;
			hr = textNode.As(&stringNode);
			if (SUCCEEDED(hr)) {
				ComPtr<IXmlNode> appendedChild;
				hr = node->AppendChild(stringNode.Get(), &appendedChild);
			}
		}
		return hr;
	}

	extern WINTOASTLIB_API inline HRESULT setEventHandlers(_In_ IToastNotification* notification, _In_ WinToastHandler* eventHandler) {
		EventRegistrationToken activatedToken, dismissedToken, failedToken;
		HRESULT hr = notification->add_Activated(eventHandler, &activatedToken);
		if (SUCCEEDED(hr)) {
			hr = notification->add_Dismissed(eventHandler, &dismissedToken);
			if (SUCCEEDED(hr)) {
				hr = notification->add_Failed(eventHandler, &failedToken);
			}
		}
		return hr;
	}

	extern WINTOASTLIB_API inline HRESULT createShellLinkInPath(_In_ PCWSTR path) {
		WCHAR exePath[MAX_PATH];
		HRESULT hr = defaultExecutablePath(exePath);
		if (SUCCEEDED(hr)) {
			ComPtr<IShellLink> shellLink;
			hr = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&shellLink));
			if (SUCCEEDED(hr)) {
				hr = shellLink->SetPath(exePath);
				if (SUCCEEDED(hr)) {
					hr = shellLink->SetArguments(L"");
					if (SUCCEEDED(hr)) {
						ComPtr<IPersistFile> persistFile;
						hr = shellLink.As(&persistFile);
						if (SUCCEEDED(hr)) {
							hr = persistFile->Save(path, TRUE);
							CoTaskMemFree(exePath);
						}
					}
				}
			}
		}
		return hr;
	}
}