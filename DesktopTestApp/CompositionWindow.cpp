#include "pch.h"
#include "CompositionWindow.h"

namespace winrt
{
    using namespace Windows::UI::Composition;
}

const std::wstring CompositionWindow::ClassName = L"CompositionWindow";

void CompositionWindow::RegisterWindowClass()
{
    auto instance = winrt::check_pointer(GetModuleHandleW(nullptr));
    WNDCLASSEX wcex = { sizeof(wcex) };
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

CompositionWindow::CompositionWindow(winrt::Compositor const& compositor, std::wstring const& windowTitle)
{
    auto instance = winrt::check_pointer(GetModuleHandleW(nullptr));
    WINRT_ASSERT(!m_window);
    WINRT_VERIFY(CreateWindowExW(WS_EX_NOREDIRECTIONBITMAP, ClassName.c_str(), windowTitle.c_str(), WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, nullptr, nullptr, instance, this));
    WINRT_ASSERT(m_window);

    ShowWindow(m_window, SW_SHOWDEFAULT);
    UpdateWindow(m_window);

    m_target = rutil::CreateDesktopWindowTarget(compositor, m_window, false);
}

CompositionWindow::~CompositionWindow()
{
    
}

LRESULT CompositionWindow::MessageHandler(UINT const message, WPARAM const wparam, LPARAM const lparam)
{
    return base_type::MessageHandler(message, wparam, lparam);
}