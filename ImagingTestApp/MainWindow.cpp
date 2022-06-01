#include "pch.h"
#include "MainWindow.h"

const std::wstring MainWindow::ClassName = L"ImagingTestApp.MainWindow";
std::once_flag MainWindowClassRegistration;

void MainWindow::RegisterWindowClass()
{
    auto instance = winrt::check_pointer(GetModuleHandleW(nullptr));
    WNDCLASSEXW wcex = {};
    wcex.cbSize = sizeof(wcex);
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = instance;
    wcex.hIcon = LoadIconW(instance, IDI_APPLICATION);
    wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wcex.lpszClassName = ClassName.c_str();
    wcex.hIconSm = LoadIconW(instance, IDI_APPLICATION);
    winrt::check_bool(RegisterClassExW(&wcex));
}

MainWindow::MainWindow(std::wstring const& titleString, int width, int height)
{
    auto instance = winrt::check_pointer(GetModuleHandleW(nullptr));

    std::call_once(MainWindowClassRegistration, []() { RegisterWindowClass(); });

    auto exStyle = WS_EX_NOREDIRECTIONBITMAP;
    auto style = WS_OVERLAPPEDWINDOW;

    RECT rect = { 0, 0, width, height};
    winrt::check_bool(AdjustWindowRectEx(&rect, style, false, exStyle));
    auto adjustedWidth = rect.right - rect.left;
    auto adjustedHeight = rect.bottom - rect.top;

    winrt::check_bool(CreateWindowExW(exStyle, ClassName.c_str(), titleString.c_str(), style,
        CW_USEDEFAULT, CW_USEDEFAULT, adjustedWidth, adjustedHeight, nullptr, nullptr, instance, this));
    WINRT_ASSERT(m_window);

    ShowWindow(m_window, SW_SHOW);
    UpdateWindow(m_window);
}

LRESULT MainWindow::MessageHandler(UINT const message, WPARAM const wparam, LPARAM const lparam)
{
    return base_type::MessageHandler(message, wparam, lparam);
}
