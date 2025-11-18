#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

// COM/Win32 headers
#include <shobjidl.h>
#include <microsoft.ui.xaml.window.h>

// WinRT headers
#include <winrt/Microsoft.UI.Xaml.Media.Imaging.h>
#include <winrt/Windows.Storage.Pickers.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.Foundation.h>


#include <winrt/Microsoft.UI.Xaml.Media.Animation.h>
#include <winrt/Microsoft.UI.Xaml.Input.h>


#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.h>

namespace winrt::App3::implementation
{
    MainWindow::MainWindow()
    {
        InitializeComponent();
    }

    winrt::Windows::Foundation::IAsyncAction MainWindow::ChangeIcon_Click(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&)
    {
        winrt::Windows::Storage::Pickers::FileOpenPicker picker;

        // Get the window handle (HWND)
        auto windowNative{ this->try_as<::IWindowNative>() };
        HWND hWnd{ 0 };
        windowNative->get_WindowHandle(&hWnd);
        
        // Initialize the file picker with the window handle
        auto initializeWithWindow{ picker.as<::IInitializeWithWindow>() };
        initializeWithWindow->Initialize(hWnd);

        picker.ViewMode(winrt::Windows::Storage::Pickers::PickerViewMode::Thumbnail);
        picker.SuggestedStartLocation(winrt::Windows::Storage::Pickers::PickerLocationId::PicturesLibrary);
        picker.FileTypeFilter().Append(L".jpg");
        picker.FileTypeFilter().Append(L".jpeg");
        picker.FileTypeFilter().Append(L".png");

        winrt::Windows::Storage::StorageFile file{ co_await picker.PickSingleFileAsync() };
        if (file)
        {
            auto stream = co_await file.OpenAsync(winrt::Windows::Storage::FileAccessMode::Read);
            winrt::Microsoft::UI::Xaml::Media::Imaging::BitmapImage bitmapImage;
            bitmapImage.SetSource(stream);
            IconImageBrush().ImageSource(bitmapImage);
        }
    }

    void MainWindow::IconGrid_PointerEntered(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e)
    {
        auto grid = sender.as<winrt::Microsoft::UI::Xaml::Controls::Grid>();
        auto storyboard = grid.Resources().TryLookup(winrt::box_value(L"FadeInStoryboard")).as<winrt::Microsoft::UI::Xaml::Media::Animation::Storyboard>();
        storyboard.Begin();
    }

    void MainWindow::IconGrid_PointerExited(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e)
    {
        auto grid = sender.as<winrt::Microsoft::UI::Xaml::Controls::Grid>();
        auto storyboard = grid.Resources().TryLookup(winrt::box_value(L"FadeOutStoryboard")).as<winrt::Microsoft::UI::Xaml::Media::Animation::Storyboard>();
        storyboard.Begin();
    }

    void MainWindow::IconImageBrush_ImageFailed(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::ExceptionRoutedEventArgs const& e)
    {
        IconEllipse().Fill(winrt::Microsoft::UI::Xaml::Media::SolidColorBrush(winrt::Microsoft::UI::Colors::Gray()));
    }
}