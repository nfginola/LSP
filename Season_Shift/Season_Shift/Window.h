#pragma once

#include <windows.h>
#include <string>
#include <assert.h>
#include <functional>
#include <imgui.h>
#include <imgui_impl_win32.h>


class Window
{
private:
	HINSTANCE m_hInst;
	HWND m_hwnd;

	UINT m_clientWidth, m_clientHeight;
	DWORD m_style, m_exStyle;
	std::wstring m_title;
	bool m_isClosed;
	bool m_firstActivate;

	void resizeToWindowToFitClient();
	
public:
	std::function<void(UINT width, UINT height)> m_onResize;
	std::function<void(bool fullScreen)> m_setFullScreen;
	std::function<bool()> m_getFullScreenState;

	Window(HINSTANCE hInst, const std::wstring title, UINT clientWidth, UINT clientHeight);
	~Window();

	HWND& getHWND();
	float getAspectRatio() const;
	UINT getClientWidth() const;
	UINT getClientHeight() const;
	
	bool isClosed() const;

	void quit() const;

	void setOnResizeCallback(std::function<void(UINT width, UINT height)> func);
	void setFullScreenCallback(std::function<void(bool fullScreen)> set, std::function<bool()> get);

	LRESULT handleProc(UINT uMsg, WPARAM wParam, LPARAM lParam);


	static LRESULT WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

