#include "pch.h"
#include "MainWindow.h"

namespace winrt
{
    using namespace Windows::Foundation;
    using namespace Windows::Foundation::Numerics;
    using namespace Windows::Foundation::Collections;
    using namespace Windows::UI;
    using namespace Windows::UI::Composition;
    using namespace Windows::Storage::Streams;
    using namespace Windows::Graphics::Imaging;
    using namespace Windows::Graphics;
    using namespace Windows::Graphics::DirectX;
    using namespace Windows::Graphics::DirectX::Direct3D11;
}

namespace util
{
    using namespace robmikh::common::uwp;
    using namespace robmikh::common::desktop;
}

winrt::IAsyncAction EncodeBytesToPngStreamAsync(
    winrt::array_view<uint8_t const> const& bytes,
    uint32_t width, 
    uint32_t height, 
    winrt::BitmapPixelFormat const& pixelFormat, 
    winrt::BitmapAlphaMode const& alphaMode, 
    winrt::IRandomAccessStream const& stream);

int __stdcall WinMain(HINSTANCE, HINSTANCE, PSTR, int)
{
    // Initialize COM
    // Usually your UI thread should be a STA, but this test
    // is playing fast and loose with the rules. Don't copy this part!
    winrt::init_apartment(winrt::apartment_type::multi_threaded);

    // Init D3D and D2D
    auto d3dDevice = util::CreateD3D11Device();
    auto d2dFactory = util::CreateD2DFactory();
    auto d2dDevice = util::CreateD2DDevice(d2dFactory, d3dDevice);
    winrt::com_ptr<ID2D1DeviceContext> d2dContext;
    winrt::check_hresult(d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, d2dContext.put()));
    winrt::com_ptr<ID3D11DeviceContext> d3dContext;
    d3dDevice->GetImmediateContext(d3dContext.put());

    // Create the DispatcherQueue that the compositor needs to run
    auto controller = util::CreateDispatcherQueueControllerForCurrentThread();

    // Create our window and visual tree
    auto window = MainWindow(L"ImagingTestApp", 800, 600);
    auto compositor = winrt::Compositor();
    auto target = window.CreateWindowTarget(compositor);
    auto root = compositor.CreateSpriteVisual();
    root.RelativeSizeAdjustment({ 1.0f, 1.0f });
    root.Brush(compositor.CreateColorBrush(winrt::Colors::White()));
    target.Root(root);

    // Create the images we'll load
    uint32_t width = 150;
    uint32_t height = 150;

    // First create the render target
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    desc.SampleDesc.Count = 1;
    winrt::com_ptr<ID3D11Texture2D> texture;
    winrt::check_hresult(d3dDevice->CreateTexture2D(&desc, nullptr, texture.put()));

    auto dxgiSurface = texture.as<IDXGISurface>();
    winrt::com_ptr<ID2D1Bitmap1> bitmap;
    winrt::check_hresult(d2dContext->CreateBitmapFromDxgiSurface(dxgiSurface.get(), nullptr, bitmap.put()));
    d2dContext->SetTarget(bitmap.get());

    // Create our red brush
    winrt::com_ptr<ID2D1SolidColorBrush> redBrush;
    winrt::check_hresult(d2dContext->CreateSolidColorBrush(D2D1::ColorF(1.0f, 0.0f, 0.0f), redBrush.put()));
    
    // Draw our image
    d2dContext->BeginDraw();
    d2dContext->Clear(D2D1::ColorF(0, 0));
    D2D1_RECT_F redRect = {};
    redRect.left = 50;
    redRect.top = 50;
    redRect.right = 100;
    redRect.bottom = 100;
    d2dContext->FillRectangle(&redRect, redBrush.get());
    winrt::check_hresult(d2dContext->EndDraw());

    // Encode the bitmap to a png within a memory stream
    auto bgraPremultipliedStream = winrt::InMemoryRandomAccessStream();
    SaveTextureToStreamAsync(texture, bgraPremultipliedStream, util::BitmapEncoding::Png).get();

    // Next, let's create the equivalent RGBA bitmap by swizzling the bytes
    auto bytes = util::CopyBytesFromTexture(texture);
    for (uint32_t i = 0; i < width * height; i++)
    {
        auto byteIndex = i * 4;
        auto blue = bytes[byteIndex + 0];
        //auto green = bytes[byteIndex + 1];
        auto red = bytes[byteIndex + 2];
        //auto alpha = bytes[byteIndex + 3];

        bytes[byteIndex + 0] = red;
        bytes[byteIndex + 2] = blue;
    }
    auto rgbaPremultipliedStream = winrt::InMemoryRandomAccessStream();
    EncodeBytesToPngStreamAsync(
        bytes,
        width,
        height,
        winrt::BitmapPixelFormat::Rgba8,
        winrt::BitmapAlphaMode::Premultiplied,
        rgbaPremultipliedStream).get();

    // Finally, take any transparent pixel and make it transparent green
    for (uint32_t i = 0; i < width * height; i++)
    {
        auto byteIndex = i * 4;
        //auto blue = bytes[byteIndex + 0];
        //auto green = bytes[byteIndex + 1];
        //auto red = bytes[byteIndex + 2];
        auto alpha = bytes[byteIndex + 3];

        if (alpha == 0)
        {
            bytes[byteIndex + 0] = 0;
            bytes[byteIndex + 1] = 255;
            bytes[byteIndex + 2] = 0;
        }
    }
    auto rgbaStraightStream = winrt::InMemoryRandomAccessStream();
    EncodeBytesToPngStreamAsync(
        bytes,
        width,
        height,
        winrt::BitmapPixelFormat::Rgba8,
        winrt::BitmapAlphaMode::Straight,
        rgbaStraightStream).get();

    // Load each of these images back into textures
    auto bgraPremultipliedTexture = util::LoadTextureFromStreamAsync(bgraPremultipliedStream, d3dDevice).get();
    auto rgbaPremultipliedTexture = util::LoadTextureFromStreamAsync(rgbaPremultipliedStream, d3dDevice).get();
    auto rgbaStraightTexture = util::LoadTextureFromStreamAsync(rgbaStraightStream, d3dDevice).get();

    // Display these in a surface
    auto compGraphics = util::CreateCompositionGraphicsDevice(compositor, d3dDevice.get());
    auto drawingSurface = compGraphics.CreateDrawingSurface2(
        winrt::SizeInt32{ static_cast<int32_t>(width * 3), static_cast<int32_t>(height) }, 
        winrt::DirectXPixelFormat::B8G8R8A8UIntNormalized, 
        winrt::DirectXAlphaMode::Premultiplied);
    POINT offset = {};
    auto drawingSurfaceDxgiSurface = util::SurfaceBeginDraw(drawingSurface, &offset);
    auto drawingSurfaceTexture = drawingSurfaceDxgiSurface.as<ID3D11Texture2D>();
    d3dContext->CopySubresourceRegion(drawingSurfaceTexture.get(), 0, 0, 0, 0, bgraPremultipliedTexture.get(), 0, nullptr);
    d3dContext->CopySubresourceRegion(drawingSurfaceTexture.get(), 0, width, 0, 0, rgbaPremultipliedTexture.get(), 0, nullptr);
    d3dContext->CopySubresourceRegion(drawingSurfaceTexture.get(), 0, width * 2, 0, 0, rgbaStraightTexture.get(), 0, nullptr);
    util::SurfaceEndDraw(drawingSurface);

    auto visual = compositor.CreateSpriteVisual();
    visual.Size({ width * 3.0f, static_cast<float>(height) });
    visual.AnchorPoint({ 0.5f, 0.5f });
    visual.RelativeOffsetAdjustment({ 0.5f, 0.5f, 0.0f });
    visual.Brush(compositor.CreateSurfaceBrush(drawingSurface));
    root.Children().InsertAtTop(visual);

    // Message pump
    MSG msg = {};
    while (GetMessageW(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return util::ShutdownDispatcherQueueControllerAndWait(controller, static_cast<int>(msg.wParam));
}

winrt::IAsyncAction EncodeBytesToPngStreamAsync(
    winrt::array_view<uint8_t const> const& bytes,
    uint32_t width,
    uint32_t height,
    winrt::BitmapPixelFormat const& pixelFormat,
    winrt::BitmapAlphaMode const& alphaMode,
    winrt::IRandomAccessStream const& stream)
{
    auto encoder = co_await winrt::BitmapEncoder::CreateAsync(
        winrt::BitmapEncoder::PngEncoderId(),
        stream);
    encoder.SetPixelData(
        pixelFormat,
        alphaMode,
        width,
        height,
        1.0,
        1.0,
        bytes);
    co_await encoder.FlushAsync();

    co_return;
}