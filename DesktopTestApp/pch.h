#pragma once

#include <Unknwn.h>
#include <inspectable.h>

#include <wil/cppwinrt.h>

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Graphics.h>
#include <winrt/Windows.Graphics.Capture.h>
#include <winrt/Windows.Graphics.DirectX.h>
#include <winrt/Windows.Graphics.DirectX.Direct3D11.h>
#include <winrt/Windows.Graphics.Imaging.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.UI.h>
#include <winrt/Windows.UI.Composition.h>
#include <winrt/Windows.UI.Composition.Desktop.h>

// STL
#include <atomic>
#include <memory>
#include <algorithm>
#include <unordered_set>
#include <vector>
#include <filesystem>

// D3D
#include <d3d11_4.h>
#include <dxgi1_6.h>
#include <d2d1_3.h>
#include <wincodec.h>

// WIL
#include <wil/resource.h>

// Common
#include "robmikh.common/composition.interop.h"
#include "robmikh.common/DesktopWindow.h"
#include "robmikh.common/d3d11Helpers.h"
#include "robmikh.common/d2dHelpers.h"
#include "robmikh.common/direct3d11.interop.h"
#include "robmikh.common/capture.desktop.interop.h"
#include "robmikh.common/dispatcherqueue.desktop.interop.h"
#include "robmikh.common/stream.interop.h"
#include "robmikh.common/graphics.interop.h"
#include "robmikh.common/ControlsHelper.h"

namespace rutil
{
    using namespace robmikh::common::desktop;
    using namespace robmikh::common::uwp;
}