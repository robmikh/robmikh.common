#include "pch.h"
#include "$safeitemname$.h"

namespace util
{
    using namespace robmikh::common::desktop::controls;
}

const std::wstring $safeitemname$::ClassName = L"$safeitemname$";
std::once_flag $safeitemname$ClassRegistration;

void $safeitemname$::RegisterWindowClass()
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
    wcex.hIconSm = LoadIconW(wcex.hInstance, IDI_APPLICATION);
    winrt::check_bool(RegisterClassExW(&wcex));
}

$safeitemname$::$safeitemname$(std::wstring const& titleString, int width, int height)
{
    auto instance = winrt::check_pointer(GetModuleHandleW(nullptr));

    std::call_once($safeitemname$ClassRegistration, []() { RegisterWindowClass(); });

    auto exStyle = 0;
    auto style = WS_OVERLAPPEDWINDOW;

    RECT rect = { 0, 0, width, height};
    winrt::check_bool(AdjustWindowRectEx(&rect, style, false, exStyle));
    auto adjustedWidth = rect.right - rect.left;
    auto adjustedHeight = rect.bottom - rect.top;

    winrt::check_bool(CreateWindowExW(exStyle, ClassName.c_str(), titleString.c_str(), style,
        CW_USEDEFAULT, CW_USEDEFAULT, adjustedWidth, adjustedHeight, nullptr, nullptr, instance, this));
    WINRT_ASSERT(m_window);

    CreateControls(instance);

    ShowWindow(m_window, SW_SHOW);
    UpdateWindow(m_window);
}

LRESULT $safeitemname$::MessageHandler(UINT const message, WPARAM const wparam, LPARAM const lparam)
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
            if (hwnd == m_demoButton)
            {
                SetWindowTextW(m_demoLabel, L"That tickles!");
            }
        }
        break;
        default:
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

void $safeitemname$::CreateControls(HINSTANCE instance)
{
    auto controls = util::StackPanel(m_window, instance, 10, 10, 40, 200, 30);

    m_demoLabel = controls.CreateControl(util::ControlType::Label, L"Hello, World!");
    m_demoButton = controls.CreateControl(util::ControlType::Button, L"Click Me!");
}
