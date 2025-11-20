#pragma once

#include "MainWindow.g.h"

#include <vector>

#include <winrt/Windows.UI.h>                      
#include <winrt/Windows.Foundation.h>              
#include <winrt/Microsoft.UI.Xaml.Shapes.h>
#include <winrt/Microsoft.UI.Xaml.Input.h>
#include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h>    
#include <winrt/Microsoft.UI.Xaml.Media.h>

#include <winrt/Windows.Data.Json.h>

// Required headers for File Picker (Win32 HWND initialization)
#include <windows.h>
#include <Shobjidl_core.h>            // IInitializeWithWindow
#include <microsoft.ui.xaml.window.h> // IWindowNative (To get HWND from WinUI 3 Window)

namespace winrt::Q2_2::implementation
{
    // Data structure to hold information for a single drawn stroke
    struct StrokeData
    {
        winrt::Windows::UI::Color color{};
        double thickness{ 2.0 };
        std::vector<winrt::Windows::Foundation::Point> points{};
    };

    struct MainWindow : MainWindowT<MainWindow>
    {
        MainWindow();
        int32_t MyProperty();
        void MyProperty(int32_t value);

        // Drawing (Canvas) Events
        void DrawCanvas_PointerPressed(winrt::Windows::Foundation::IInspectable const& sender,
            winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);
        void DrawCanvas_PointerMoved(winrt::Windows::Foundation::IInspectable const& sender,
            winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);
        void DrawCanvas_PointerReleased(winrt::Windows::Foundation::IInspectable const& sender,
            winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);

        // Control Events
        void PenColorPicker_ColorChanged(winrt::Windows::Foundation::IInspectable const& sender,
            winrt::Microsoft::UI::Xaml::Controls::ColorChangedEventArgs const& e);
        void ApplyColorButton_Click(winrt::Windows::Foundation::IInspectable const& sender,
            winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void CloseColorFlyout_Click(winrt::Windows::Foundation::IInspectable const& sender,
            winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void ThicknessSlider_ValueChanged(winrt::Windows::Foundation::IInspectable const& sender,
            winrt::Microsoft::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs const& e);

        // Save/Load Operations
        winrt::fire_and_forget SaveButton_Click(winrt::Windows::Foundation::IInspectable const& sender,
            winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        winrt::fire_and_forget LoadButton_Click(winrt::Windows::Foundation::IInspectable const& sender,
            winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void ClearButton_Click(winrt::Windows::Foundation::IInspectable const& sender,
            winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);

    private:
        // Current stroke handle and data being collected
        winrt::Microsoft::UI::Xaml::Shapes::Polyline m_currentStroke{ nullptr };
        StrokeData m_currentData{};
        bool m_isDrawing{ false };

        // Current pen settings (applied to new strokes)
        winrt::Windows::UI::Color m_currentColor{ 255, 0, 0, 0 };
        winrt::Windows::UI::Color m_pendingColor{ 255, 0, 0, 0 }; // Color selected but not yet applied
        double m_currentThickness{ 2.0 };

        // Completed strokes (the data model)
        std::vector<StrokeData> m_strokes;

        // Utility Methods
        void UpdateColorPreview();
        void RedrawFromModel();
        // Helper to initialize FilePickers with the current window handle
        void InitializePickerWithWindow(winrt::Windows::Foundation::IInspectable const& picker);

        // Error message dialog
        winrt::Windows::Foundation::IAsyncAction ShowErrorDialog(winrt::hstring const& message);
    };
}

namespace winrt::Q2_2::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}