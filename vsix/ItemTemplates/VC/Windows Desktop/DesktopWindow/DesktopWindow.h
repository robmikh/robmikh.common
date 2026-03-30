#pragma once
#include <robmikh.common/DesktopWindow.h>

struct $safeitemname$ : robmikh::common::desktop::DesktopWindow<$safeitemname$>
{
	static const std::wstring ClassName;
	$safeitemname$(std::wstring const& titleString, int width, int height);
	LRESULT MessageHandler(UINT const message, WPARAM const wparam, LPARAM const lparam);

private:
	static void RegisterWindowClass();
    void CreateControls(HINSTANCE instance);

private:
    HWND m_demoLabel = nullptr;
    HWND m_demoButton = nullptr;
};
