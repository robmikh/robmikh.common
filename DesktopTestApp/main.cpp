#include "pch.h"
#include "CompositionWindow.h"

namespace winrt
{
    using namespace Windows::Foundation;
    using namespace Windows::Foundation::Numerics;
    using namespace Windows::Graphics::Capture;
    using namespace Windows::UI;
    using namespace Windows::UI::Composition;
}

int __stdcall WinMain(HINSTANCE, HINSTANCE, PSTR, int)
{
    winrt::init_apartment();
    
    CompositionWindow::RegisterWindowClass();

    auto controller = rutil::CreateDispatcherQueueControllerForCurrentThread();

    auto compositor = winrt::Compositor();
    auto root = compositor.CreateSpriteVisual();
    root.RelativeSizeAdjustment({ 1.0f, 1.0f });
    root.Brush(compositor.CreateColorBrush(winrt::Colors::White()));

    auto window = CompositionWindow(compositor, L"Hello, World!");
    window.Root(root);

    auto picker = winrt::GraphicsCapturePicker();
    window.InitializeObjectWithWindowHandle(picker);

    // Message pump
    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return static_cast<int>(msg.wParam);
}
