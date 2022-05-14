#include "pch.h"
#include "ControlsWindow.h"

const std::wstring ControlsWindow::ClassName = L"CaptureVideoSample.ControlsWindow";

namespace util
{
    using namespace robmikh::common::desktop::controls;
}

void ControlsWindow::RegisterWindowClass()
{
    auto instance = winrt::check_pointer(GetModuleHandleW(nullptr));
    WNDCLASSEXW wcex = {};
    wcex.cbSize = sizeof(wcex);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = instance;
    wcex.hIcon = LoadIconW(instance, IDI_APPLICATION);
    wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszClassName = ClassName.c_str();
    wcex.hIconSm = LoadIconW(instance, IDI_APPLICATION);
    winrt::check_bool(RegisterClassExW(&wcex));
}

ControlsWindow::ControlsWindow(std::wstring const& titleString, int width, int height)
{
    auto instance = winrt::check_pointer(GetModuleHandleW(nullptr));

    auto exStyle = 0;
    auto style = WS_OVERLAPPEDWINDOW;

    // Create our window
    winrt::check_bool(CreateWindowExW(exStyle, ClassName.c_str(), titleString.c_str(), style,
        CW_USEDEFAULT, CW_USEDEFAULT, width, height, nullptr, nullptr, instance, this));
    WINRT_ASSERT(m_window);

    // Get the dpi
    auto dpi = GetDpiForWindow(m_window);

    // Adjust our window to the desired width/height for the current dpi
    RECT rect = { 0, 0, width, height };
    winrt::check_bool(AdjustWindowRectExForDpi(&rect, style, false, exStyle, dpi));
    auto adjustedWidth = rect.right - rect.left;
    auto adjustedHeight = rect.bottom - rect.top;
    winrt::check_bool(SetWindowPos(m_window, nullptr, 0, 0, adjustedWidth, adjustedHeight, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER));

    // Set a DPI-scaled font
    m_font = util::GetFontForDpi(dpi);
    SendMessageW(m_window, WM_SETFONT, reinterpret_cast<WPARAM>(m_font.get()), true);

    CreateControls(instance);

    ShowWindow(m_window, SW_SHOW);
    UpdateWindow(m_window);
}

LRESULT ControlsWindow::MessageHandler(UINT const message, WPARAM const wparam, LPARAM const lparam)
{
    switch (message)
    {
    case WM_DPICHANGED:
    {
        auto dpi = GetDpiForWindow(m_window);
        m_font = util::GetFontForDpi(dpi);
        SendMessageW(m_window, WM_SETFONT, reinterpret_cast<WPARAM>(m_font.get()), true);

        m_stackPanel->OnDpiChanged(m_font);
    }
        return base_type::MessageHandler(message, wparam, lparam);
    case WM_COMMAND:
    {
        auto command = HIWORD(wparam);
        auto hwnd = (HWND)lparam;
        switch (command)
        {
        case BN_CLICKED:
        {
            if (hwnd == m_someButton)
            {
                winrt::check_bool(SetWindowTextW(m_someButton, L"Hey!"));
            }
        }
        break;
        }
    }
    break;
    case WM_CTLCOLORSTATIC:
        return util::StaticControlColorMessageHandler(wparam, lparam);
    default:
        return base_type::MessageHandler(message, wparam, lparam);
    }
    return 0;
}

void ControlsWindow::CreateControls(HINSTANCE instance)
{
    m_stackPanel = std::make_unique<util::StackPanel>(m_window, instance, m_font, 10, 10, 40, 200, 30);

    m_someLabel = m_stackPanel->CreateControl(util::ControlType::Label, L"Some Label");
    m_someButton = m_stackPanel->CreateControl(util::ControlType::Button, L"Some Button");
    m_someEdit = m_stackPanel->CreateControl(util::ControlType::Edit, L"Woop");
}
