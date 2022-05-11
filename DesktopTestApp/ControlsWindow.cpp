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

    winrt::check_bool(CreateWindowExW(0, ClassName.c_str(), titleString.c_str(), WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, width, height, nullptr, nullptr, instance, this));
    WINRT_ASSERT(m_window);

    ShowWindow(m_window, SW_SHOWDEFAULT);
    UpdateWindow(m_window);

    CreateControls(instance);
}

LRESULT ControlsWindow::MessageHandler(UINT const message, WPARAM const wparam, LPARAM const lparam)
{
    switch (message)
    {
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
    return base_type::MessageHandler(message, wparam, lparam);
}

void ControlsWindow::CreateControls(HINSTANCE instance)
{
    auto controls = util::StackPanel(m_window, instance, 10, 10, 40, 200, 30);

    m_someLabel = controls.CreateControl(util::ControlType::Label, L"Some Label");
    m_someButton = controls.CreateControl(util::ControlType::Button, L"Some Button");
    m_someEdit = controls.CreateControl(util::ControlType::Edit, L"Woop");
}
