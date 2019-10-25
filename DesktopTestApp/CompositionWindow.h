#pragma once

struct CompositionWindow : rutil::DesktopWindow<CompositionWindow>
{
    static const std::wstring ClassName;
    static void RegisterWindowClass();

    CompositionWindow(winrt::Windows::UI::Composition::Compositor const& compositor, std::wstring const& windowTitle);
    ~CompositionWindow();

    LRESULT MessageHandler(UINT const message, WPARAM const wparam, LPARAM const lparam);

    winrt::Windows::UI::Composition::Visual Root() { return m_target.Root(); }
    void Root(winrt::Windows::UI::Composition::Visual const& visual) { m_target.Root(visual); }

private:
    winrt::Windows::UI::Composition::CompositionTarget m_target{ nullptr };
};