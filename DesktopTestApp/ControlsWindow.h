#pragma once
#include "robmikh.common/DesktopWindow.h"

struct ControlsWindow : robmikh::common::desktop::DesktopWindow<ControlsWindow>
{
	static const std::wstring ClassName;
	static void RegisterWindowClass();
	ControlsWindow(std::wstring const& titleString, int width, int height);
	LRESULT MessageHandler(UINT const message, WPARAM const wparam, LPARAM const lparam);
private:
	void CreateControls(HINSTANCE instance);

private:
	std::unique_ptr<robmikh::common::desktop::controls::StackPanel> m_stackPanel;
	wil::shared_hfont m_font;
	HWND m_someLabel = nullptr;
	HWND m_someButton = nullptr;
	HWND m_someEdit = nullptr;
};