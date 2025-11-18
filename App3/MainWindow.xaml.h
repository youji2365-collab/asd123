#pragma once

#include "MainWindow.g.h"
#include "winrt/Windows.Storage.Pickers.h"
#include "winrt/Windows.Storage.Streams.h"
#include "winrt/Microsoft.UI.Xaml.Media.Imaging.h"
#include "winrt/Windows.Foundation.h"

#include "winrt/Microsoft.UI.Xaml.Input.h"

namespace winrt::App3::implementation
{
    struct MainWindow : MainWindowT<MainWindow>
    {
        MainWindow();

        Windows::Foundation::IAsyncAction ChangeIcon_Click(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& args);
        void IconGrid_PointerEntered(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);
        void IconGrid_PointerExited(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);
        void IconImageBrush_ImageFailed(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::ExceptionRoutedEventArgs const& e);
    };
}

namespace winrt::App3::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}
