#include "pch.h"
#include "CompositionWindow.h"
#include "ControlsWindow.h"

namespace winrt
{
    using namespace Windows::Foundation;
    using namespace Windows::Foundation::Numerics;
    using namespace Windows::Graphics::Capture;
    using namespace Windows::Graphics::DirectX;
    using namespace Windows::Graphics::DirectX::Direct3D11;
    using namespace Windows::Graphics::Imaging;
    using namespace Windows::Storage;
    using namespace Windows::UI;
    using namespace Windows::UI::Composition;
}

int __stdcall WinMain(HINSTANCE, HINSTANCE, PSTR, int)
{
    winrt::init_apartment();
    
    CompositionWindow::RegisterWindowClass();
    ControlsWindow::RegisterWindowClass();

    auto controller = rutil::CreateDispatcherQueueControllerForCurrentThread();

    auto compositor = winrt::Compositor();
    auto root = compositor.CreateSpriteVisual();
    root.RelativeSizeAdjustment({ 1.0f, 1.0f });
    root.Brush(compositor.CreateColorBrush(winrt::Colors::White()));

    auto window = CompositionWindow(compositor, L"Hello, World!");
    window.Root(root);

    auto picker = winrt::GraphicsCapturePicker();
    window.InitializeObjectWithWindowHandle(picker);

    auto d3dDevice = rutil::CreateD3DDevice();
    auto d3dDevice2 = rutil::CreateD3DDevice(D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_VIDEO_SUPPORT);

    // https://docs.microsoft.com/en-us/windows/win32/direct2d/path-geometries-overview
    auto d2dFactory = rutil::CreateD2DFactory();
    /*
    winrt::com_ptr<ID2D1RoundedRectangleGeometry> rectGeometry;
    winrt::check_hresult(d2dFactory->CreateRoundedRectangleGeometry(
        D2D1_ROUNDED_RECT{ D2D1_RECT_F { 0, 0, 167, 243 }, 9.5f, 9.5f },
        rectGeometry.put()));
    auto compositionPath = winrt::CompositionPath(winrt::make<rutil::GeometrySource>(rectGeometry));
    auto compositionPathGeometry = compositor.CreatePathGeometry(compositionPath);
    auto shape = compositor.CreateSpriteShape(compositionPathGeometry);
    shape.StrokeThickness(1);
    shape.StrokeBrush(compositor.CreateColorBrush(winrt::Colors::Black()));
    */

    winrt::com_ptr<ID2D1PathGeometry> pathGeometry;
    winrt::check_hresult(d2dFactory->CreatePathGeometry(pathGeometry.put()));
    winrt::com_ptr<ID2D1GeometrySink> sink;
    winrt::check_hresult(pathGeometry->Open(sink.put()));
    sink->SetFillMode(D2D1_FILL_MODE_WINDING);
    sink->BeginFigure(
        D2D1::Point2F(183, 392),
        D2D1_FIGURE_BEGIN_FILLED
    );
    sink->AddBezier(
        D2D1::BezierSegment(
            D2D1::Point2F(238, 284),
            D2D1::Point2F(472, 345),
            D2D1::Point2F(356, 303)
        ));
    sink->AddBezier(
        D2D1::BezierSegment(
            D2D1::Point2F(237, 261),
            D2D1::Point2F(333, 256),
            D2D1::Point2F(333, 256)
        ));
    sink->AddBezier(
        D2D1::BezierSegment(
            D2D1::Point2F(335, 257),
            D2D1::Point2F(241, 261),
            D2D1::Point2F(411, 306)
        ));
    sink->AddBezier(
        D2D1::BezierSegment(
            D2D1::Point2F(574, 350),
            D2D1::Point2F(288, 324),
            D2D1::Point2F(296, 392)
        ));
    sink->EndFigure(D2D1_FIGURE_END_OPEN);
    winrt::check_hresult(sink->Close());
    auto compositionPath = winrt::CompositionPath(winrt::make<rutil::GeometrySource>(pathGeometry));
    auto compositionPathGeometry = compositor.CreatePathGeometry(compositionPath);
    auto shape = compositor.CreateSpriteShape(compositionPathGeometry);
    shape.FillBrush(compositor.CreateColorBrush(winrt::Colors::LightBlue()));

    auto shapeVisual = compositor.CreateShapeVisual();
    shapeVisual.RelativeSizeAdjustment({ 1, 1 });
    shapeVisual.Shapes().Append(shape);
    root.Children().InsertAtTop(shapeVisual);

    // Seperate window for controls
    auto controlsWindow = ControlsWindow(L"Controls Window", 800, 600);

    // Get the primary monitor
    auto monitor = MonitorFromWindow(GetDesktopWindow(), MONITOR_DEFAULTTOPRIMARY);
    auto item = rutil::CreateCaptureItemForMonitor(monitor);
    auto itemSize = item.Size();

    // Get a file to save the screenshot
    auto currentPath = std::filesystem::current_path();
    auto folder = winrt::StorageFolder::GetFolderFromPathAsync(currentPath.wstring()).get();
    auto file = folder.CreateFileAsync(L"screenshot.png", winrt::CreationCollisionOption::ReplaceExisting).get();

    auto dxgiDevice = d3dDevice.as<IDXGIDevice>();
    auto device = CreateDirect3DDevice(dxgiDevice.get());
    auto framePool = winrt::Direct3D11CaptureFramePool::CreateFreeThreaded(
        device,
        winrt::DirectXPixelFormat::B8G8R8A8UIntNormalized,
        1,
        itemSize);
    auto session = framePool.CreateCaptureSession(item);
    session.IsCursorCaptureEnabled(false);

    winrt::com_ptr<ID3D11Texture2D> texture;
    winrt::com_ptr<ID3D11DeviceContext> d3dContext;
    d3dDevice->GetImmediateContext(d3dContext.put());
    wil::shared_event captureEvent(wil::EventOptions::None);
    framePool.FrameArrived([&texture, d3dDevice, d3dContext, captureEvent, session](auto& pool, auto&)
        {
            auto frame = pool.TryGetNextFrame();

            auto sourceTexture = GetDXGIInterfaceFromObject<ID3D11Texture2D>(frame.Surface());

            texture = rutil::CopyD3DTexture(d3dDevice, sourceTexture, false);

            pool.Close();
            session.Close();
            frame.Close();

            captureEvent.SetEvent();
        });

    session.StartCapture();
    captureEvent.wait();

    {
        D3D11_TEXTURE2D_DESC desc = {};
        texture->GetDesc(&desc);

        auto bytes = rutil::CopyBytesFromTexture(texture);
        auto stream = file.OpenAsync(winrt::FileAccessMode::ReadWrite).get();
        auto encoder = winrt::BitmapEncoder::CreateAsync(winrt::BitmapEncoder::PngEncoderId(), stream).get();
        encoder.SetPixelData(
            winrt::BitmapPixelFormat::Bgra8,
            winrt::BitmapAlphaMode::Premultiplied,
            desc.Width,
            desc.Height,
            1.0,
            1.0,
            bytes);
        encoder.FlushAsync().get();
    }

    // Message pump
    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return static_cast<int>(msg.wParam);
}
