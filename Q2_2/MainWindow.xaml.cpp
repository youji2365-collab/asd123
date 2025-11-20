#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.Shapes.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Input.h>
#include <winrt/Microsoft.UI.Input.h>                // PointerPoint
#include <winrt/Microsoft.UI.Windowing.h>            // AppWindow

#include <winrt/Windows.Data.Json.h>                 // JSON
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Storage.Pickers.h>

#include <windows.h>
#include <Shobjidl_core.h>                           // IInitializeWithWindow
#include <microsoft.ui.xaml.window.h> // IWindowNative

#include <sstream>

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Microsoft::UI::Xaml::Input;
using namespace Microsoft::UI::Xaml::Media;

using Windows::Data::Json::JsonArray;
using Windows::Data::Json::JsonObject;
using Windows::Data::Json::JsonValue;
using Windows::Foundation::IAsyncAction;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::Q2_2::implementation
{
    MainWindow::MainWindow()
    {
        InitializeComponent();

        // Set the default window size to 1024x800
        if (auto aw = this->AppWindow())
        {
            aw.Resize({ 1024, 800 });
        }

        // Initialize UI state
        if (auto cp = this->PenColorPicker())
        {
            cp.Color(m_currentColor);
        }
        UpdateColorPreview();

        if (auto tb = this->ThicknessValueText())
        {
            tb.Text(L"2.0 px");
        }
    }
    int32_t MainWindow::MyProperty() { throw hresult_not_implemented(); }
    void MainWindow::MyProperty(int32_t) { throw hresult_not_implemented(); }
    // ---- Initialize FilePicker with Win32 HWND (Necessary for WinApp SDK) ----
    void MainWindow::InitializePickerWithWindow(Windows::Foundation::IInspectable const& picker)
    {
        // Get the HWND from the current Window (this) via IWindowNative
        if (auto native = this->try_as<::IWindowNative>())
        {
            HWND hwnd{};
            if (SUCCEEDED(native->get_WindowHandle(&hwnd)) && hwnd)
            {
                // Initialize the file picker with the window handle
                if (auto init = picker.try_as<::IInitializeWithWindow>())
                {
                    (void)init->Initialize(hwnd);
                }
            }
        }
    }

    // ==== Error Dialog ====
    IAsyncAction MainWindow::ShowErrorDialog(hstring const& message)
    {
        ContentDialog dlg{};
        dlg.XamlRoot(this->Content().XamlRoot());  // WinUI 3: XamlRoot must be set
        dlg.Title(box_value(L"Error"));
        dlg.Content(box_value(message));
        dlg.CloseButtonText(L"OK");
        co_await dlg.ShowAsync();
    }

    // ==== ColorPicker: Update pending color and preview only during interaction ====
    void MainWindow::PenColorPicker_ColorChanged(IInspectable const&,
        Microsoft::UI::Xaml::Controls::ColorChangedEventArgs const& e)
    {
        m_pendingColor = e.NewColor();
        UpdateColorPreview();
    }

    // ==== [Apply] Button: Apply the pending color and hide the flyout ====
    void MainWindow::ApplyColorButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        m_currentColor = m_pendingColor;

        if (auto cp = this->PenColorPicker())
            cp.Color(m_currentColor);
        UpdateColorPreview();

        if (auto f = this->ColorFlyout())
            f.Hide();
    }

    void MainWindow::CloseColorFlyout_Click(IInspectable const&, RoutedEventArgs const&)
    {
        if (auto f = this->ColorFlyout())
            f.Hide();
    }

    // ==== Thickness Slider Value Change ====
    void MainWindow::ThicknessSlider_ValueChanged(IInspectable const&,
        Microsoft::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs const& e)
    {
        m_currentThickness = e.NewValue();

        // Update the thickness display text
        if (auto tb = this->ThicknessValueText())
        {
            wchar_t buf[32]{};
            swprintf_s(buf, L"%.1f px", m_currentThickness);
            tb.Text(buf);
        }

        // Apply the new thickness to the currently drawing stroke
        if (m_currentStroke)
            m_currentStroke.StrokeThickness(m_currentThickness);
    }

    // ==== Drawing Start (Pointer Pressed) ====
    void MainWindow::DrawCanvas_PointerPressed(IInspectable const&, PointerRoutedEventArgs const& e)
    {
        auto canvas = this->DrawCanvas();
        if (!canvas) return;

        auto point = e.GetCurrentPoint(canvas);
        // Only start drawing on Left Button press
        if (!point.Properties().IsLeftButtonPressed()) return;

        m_isDrawing = true;

        // Create a new Polyline for the stroke
        Microsoft::UI::Xaml::Shapes::Polyline pl;
        pl.Stroke(Microsoft::UI::Xaml::Media::SolidColorBrush(m_currentColor));
        pl.StrokeThickness(m_currentThickness);
        pl.StrokeLineJoin(Microsoft::UI::Xaml::Media::PenLineJoin::Round);
        pl.StrokeStartLineCap(Microsoft::UI::Xaml::Media::PenLineCap::Round);
        pl.StrokeEndLineCap(Microsoft::UI::Xaml::Media::PenLineCap::Round);

        // Add the first point
        auto pos = point.Position();
        Windows::Foundation::Point p{ static_cast<float>(pos.X), static_cast<float>(pos.Y) };
        pl.Points().Append(p);

        // Add the polyline to the canvas
        canvas.Children().Append(pl);
        canvas.CapturePointer(e.Pointer());

        m_currentStroke = pl;

        // Initialize StrokeData for saving
        m_currentData = {};
        m_currentData.color = m_currentColor;
        m_currentData.thickness = m_currentThickness;
        m_currentData.points.push_back(p);

        e.Handled(true);
    }

    // ==== Add point on pointer move ====
    void MainWindow::DrawCanvas_PointerMoved(IInspectable const&, PointerRoutedEventArgs const& e)
    {
        if (!m_isDrawing || !m_currentStroke) return;

        auto canvas = this->DrawCanvas();
        if (!canvas) return;

        auto point = e.GetCurrentPoint(canvas);
        // Check if the pointer is still in contact (i.e., button is down)
        if (!point.IsInContact()) return;

        // Add point to the current Polyline
        auto pos = point.Position();
        Windows::Foundation::Point p{ static_cast<float>(pos.X), static_cast<float>(pos.Y) };
        m_currentStroke.Points().Append(p);
        m_currentData.points.push_back(p);

        e.Handled(true);
    }

    // ==== Drawing End (Pointer Released) ====
    void MainWindow::DrawCanvas_PointerReleased(IInspectable const&, PointerRoutedEventArgs const& e)
    {
        if (!m_isDrawing) return;

        // Release pointer capture from the canvas
        if (auto canvas = this->DrawCanvas())
            canvas.ReleasePointerCaptures();

        m_isDrawing = false;

        // Save the completed stroke data to the main collection
        if (!m_currentData.points.empty())
            m_strokes.push_back(std::move(m_currentData));

        m_currentStroke = nullptr;
        e.Handled(true);
    }

    // ==== Save (Serialize strokes to JSON and write to file) ====
    winrt::fire_and_forget MainWindow::SaveButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        hstring err;    // Used to store error message for the dialog (since co_await is not allowed in catch)
        try
        {
            // 1. Create JSON structure
            JsonArray rootArray;
            for (auto const& s : m_strokes)
            {
                JsonObject o;

                // Store Color (A, R, G, B)
                JsonArray c;
                c.Append(JsonValue::CreateNumberValue(s.color.A));
                c.Append(JsonValue::CreateNumberValue(s.color.R));
                c.Append(JsonValue::CreateNumberValue(s.color.G));
                c.Append(JsonValue::CreateNumberValue(s.color.B));
                o.Insert(L"color", c);

                // Store Thickness
                o.Insert(L"thickness", JsonValue::CreateNumberValue(s.thickness));

                // Store Points (array of {x, y} objects)
                JsonArray pts;
                for (auto const& pt : s.points)
                {
                    JsonObject jp;
                    jp.Insert(L"x", JsonValue::CreateNumberValue(pt.X));
                    jp.Insert(L"y", JsonValue::CreateNumberValue(pt.Y));
                    pts.Append(jp);
                }
                o.Insert(L"points", pts);

                rootArray.Append(o);
            }

            // Create the main document object (including version for future compatibility)
            JsonObject doc;
            doc.Insert(L"version", JsonValue::CreateNumberValue(1));
            doc.Insert(L"strokes", rootArray);

            // 2. Show File Save Picker
            Windows::Storage::Pickers::FileSavePicker picker;
            InitializePickerWithWindow(picker);
            picker.SuggestedStartLocation(Windows::Storage::Pickers::PickerLocationId::DocumentsLibrary);
            picker.SuggestedFileName(L"strokes");
            picker.FileTypeChoices().Insert(L"JSON", single_threaded_vector<hstring>({ L".json" }));

            Windows::Storage::StorageFile file = co_await picker.PickSaveFileAsync();
            if (!file) co_return; // User canceled

            // 3. Write the JSON string to the file
            co_await Windows::Storage::FileIO::WriteTextAsync(file, doc.Stringify());
        }
        catch (winrt::hresult_error const& ex)
        {
            err = L"An error occurred during save operation.\n\n" + ex.message();
        }
        catch (...)
        {
            err = L"An unknown error occurred during the save operation.";
        }

        if (!err.empty())
        {
            co_await ShowErrorDialog(err);
        }
        co_return;
    }

    // ==== Load (Open file, parse JSON, and redraw canvas) ====
    winrt::fire_and_forget MainWindow::LoadButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        hstring err;    // Used to store error message for the dialog
        try
        {
            // 1. Show File Open Picker
            Windows::Storage::Pickers::FileOpenPicker picker;
            InitializePickerWithWindow(picker);
            picker.SuggestedStartLocation(Windows::Storage::Pickers::PickerLocationId::DocumentsLibrary);
            picker.FileTypeFilter().Append(L".json");

            Windows::Storage::StorageFile file = co_await picker.PickSingleFileAsync();
            if (!file) co_return; // User canceled

            // 2. Read file content
            auto text = co_await Windows::Storage::FileIO::ReadTextAsync(file);

            // 3. Parse JSON content
            JsonObject doc;
            if (!JsonObject::TryParse(text, doc))
            {
                err = L"JSON parsing failed: Please check if the file content is valid JSON.";
            }
            else
            {
                // Check for 'strokes' key
                if (!doc.HasKey(L"strokes"))
                {
                    err = L"JSON is missing the 'strokes' key.";
                }
                else if (doc.Lookup(L"strokes").ValueType() != Windows::Data::Json::JsonValueType::Array)
                {
                    err = L"The 'strokes' value is not a JSON array.";
                }
                else
                {
                    auto arr = doc.Lookup(L"strokes").GetArray();

                    // Clear existing strokes
                    m_strokes.clear();

                    // Iterate and reconstruct stroke data
                    for (uint32_t i = 0; i < arr.Size(); ++i)
                    {
                        auto itemVal = arr.GetAt(i);
                        if (itemVal.ValueType() != Windows::Data::Json::JsonValueType::Object)
                            continue;

                        auto o = itemVal.GetObject();
                        StrokeData s{};

                        // Parse color (A, R, G, B)
                        if (o.HasKey(L"color") && o.Lookup(L"color").ValueType() == Windows::Data::Json::JsonValueType::Array)
                        {
                            auto c = o.Lookup(L"color").GetArray();
                            if (c.Size() == 4)
                            {
                                s.color = {
                                    static_cast<uint8_t>(c.GetAt(0).GetNumber()),
                                    static_cast<uint8_t>(c.GetAt(1).GetNumber()),
                                    static_cast<uint8_t>(c.GetAt(2).GetNumber()),
                                    static_cast<uint8_t>(c.GetAt(3).GetNumber())
                                };
                            }
                        }

                        // Parse thickness
                        if (o.HasKey(L"thickness") && o.Lookup(L"thickness").ValueType() == Windows::Data::Json::JsonValueType::Number)
                        {
                            s.thickness = o.Lookup(L"thickness").GetNumber();
                        }

                        // Parse points
                        if (o.HasKey(L"points") && o.Lookup(L"points").ValueType() == Windows::Data::Json::JsonValueType::Array)
                        {
                            auto pts = o.Lookup(L"points").GetArray();
                            s.points.reserve(pts.Size());
                            for (uint32_t k = 0; k < pts.Size(); ++k)
                            {
                                auto pv = pts.GetAt(k);
                                if (pv.ValueType() != Windows::Data::Json::JsonValueType::Object)
                                    continue;

                                auto jp = pv.GetObject();
                                // Check for valid 'x' and 'y' number keys
                                if (jp.HasKey(L"x") && jp.HasKey(L"y") &&
                                    jp.Lookup(L"x").ValueType() == Windows::Data::Json::JsonValueType::Number &&
                                    jp.Lookup(L"y").ValueType() == Windows::Data::Json::JsonValueType::Number)
                                {
                                    float x = static_cast<float>(jp.Lookup(L"x").GetNumber());
                                    float y = static_cast<float>(jp.Lookup(L"y").GetNumber());
                                    s.points.push_back({ x, y });
                                }
                            }
                        }

                        if (!s.points.empty())
                            m_strokes.push_back(std::move(s));
                    }

                    // Redraw the UI
                    RedrawFromModel();
                }
            }
        }
        catch (winrt::hresult_error const& ex)
        {
            err = L"An error occurred while reading the file.\n\n" + ex.message();
        }
        catch (...)
        {
            err = L"An unknown error occurred while reading the file.";
        }

        if (!err.empty())
        {
            co_await ShowErrorDialog(err);
        }
        co_return;
    }

    // ==== Update Color Preview Chip ====
    void MainWindow::UpdateColorPreview()
    {
        if (auto chip = this->ColorPreview())
        {
            chip.Fill(Microsoft::UI::Xaml::Media::SolidColorBrush(m_pendingColor));
        }
    }

    // ==== Redraw all strokes on canvas from model data ====
    void MainWindow::RedrawFromModel()
    {
        auto canvas = this->DrawCanvas();
        if (!canvas) return;

        canvas.Children().Clear();

        for (auto const& s : m_strokes)
        {
            if (s.points.empty()) continue;

            // Create Polyline object for the stroke
            Microsoft::UI::Xaml::Shapes::Polyline pl;
            pl.Stroke(Microsoft::UI::Xaml::Media::SolidColorBrush(s.color));
            pl.StrokeThickness(s.thickness);
            pl.StrokeLineJoin(Microsoft::UI::Xaml::Media::PenLineJoin::Round);
            pl.StrokeStartLineCap(Microsoft::UI::Xaml::Media::PenLineCap::Round);
            pl.StrokeEndLineCap(Microsoft::UI::Xaml::Media::PenLineCap::Round);

            // Add all points to the Polyline
            for (auto const& pt : s.points)
            {
                pl.Points().Append(pt);
            }

            canvas.Children().Append(pl);
        }
    }

    void MainWindow::ClearButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        // Clear both the data model and the UI
        m_strokes.clear();
        RedrawFromModel();
    }
}