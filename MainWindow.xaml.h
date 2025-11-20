#pragma once

#include "MainWindow.g.h"
#include <winrt/Microsoft.UI.Windowing.h>

namespace winrt::Q2_1::implementation
{
    struct MainWindow : MainWindowT<MainWindow>
    {
        MainWindow()
        {
            // Xaml objects should not call InitializeComponent during construction.
            // See https://github.com/microsoft/cppwinrt/tree/master/nuget#initializecomponent
            if (auto aw = this->AppWindow())
            {
                aw.Resize({ 320, 240 });  // Ã¢ Å©±â 320x240
            }
        }

        int32_t MyProperty();
        void MyProperty(int32_t value);

        void myButton_Click(IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& args);
    };
}

namespace winrt::Q2_1::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}
