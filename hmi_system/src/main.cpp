#include <atomic>
#include <mutex>
#include <string>
#include <Windows.h>
#include <graphics/graphics_system.h>
#include <graphics/graphics_element.h>
#include <wrl/client.h>

class ColorButton;
class HmiSystemWindow
{
public:
    HmiSystemWindow(const std::wstring &title, int width, int height);

    ~HmiSystemWindow();

    void SpinOnce();

private:
    static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
    HWND m_hWnd;
    hmi_graphics::System* m_graphics;
    ColorButton* m_redButton;
    ColorButton* m_greenButton;
};

class ColorButton: public hmi_graphics::GraphicsElement
{
public:
    explicit ColorButton(const D2D1_COLOR_F& color, const std::wstring &title)
    {
        m_color = color;
        m_label = title;
    }

    bool Initialize(Pimpl* pimpl, hmi_graphics::System* parent) override;

    void Render(hmi_graphics::System* parent) override;
private:
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_brush;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_blackBrush;
    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_textFormat;
    Microsoft::WRL::ComPtr<IDWriteTextLayout> m_textLayout;
    D2D1_COLOR_F m_color;
    std::wstring m_label;
};

bool ColorButton::Initialize(Pimpl* pimpl, hmi_graphics::System* parent)
{
    GraphicsElement::Initialize(pimpl, parent);
    parent->GetCachedColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &m_brush);
    parent->GetCachedColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &m_blackBrush);
    Microsoft::WRL::ComPtr<IDWriteFactory> dwriteFactory;
    parent->GetDirectWriteFactory(&dwriteFactory);
    dwriteFactory->CreateTextFormat(L"arial", nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 11.f, L"", &m_textFormat);
    m_textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    m_textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    auto size = GetSize();
    dwriteFactory->CreateTextLayout(m_label.c_str(), m_label.size(), m_textFormat.Get(), std::get<0>(size), std::get<1>(size), &m_textLayout);
    return true;
}

void ColorButton::Render(hmi_graphics::System* parent)
{
    Microsoft::WRL::ComPtr<ID2D1DeviceContext> context;
    parent->GetDirect2dDeviceContext(&context);
    context->SetTarget(GetTarget());
    context->BeginDraw();
    context->Clear(m_color);
    context->DrawTextLayout(D2D1::Point2(0.f, 0.f), m_textLayout.Get(), m_blackBrush.Get());
    context->EndDraw();
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    HmiSystemWindow window(L"Hello World", 800, 600);

    MSG message{};
    while(message.message != WM_QUIT)
    {
        if(PeekMessage(&message, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }

        window.SpinOnce();
    }

    return 0;
}


static std::atomic_bool init_flag;
static std::mutex init_mutex;

HmiSystemWindow::HmiSystemWindow(const std::wstring& title, int width, int height)
    : m_hWnd(nullptr)
    , m_graphics()
    , m_greenButton()
    ,m_redButton()
{
    HINSTANCE hInstance = GetModuleHandleW(nullptr);
    if(!init_flag.load())
    {
        std::lock_guard<std::mutex> lock(init_mutex);
        if(!init_flag.load())
        {
            WNDCLASSW wndClass{};
            wndClass.style = CS_OWNDC;
            wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
            wndClass.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
            wndClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
            wndClass.hInstance = hInstance;
            wndClass.cbClsExtra = 0;
            wndClass.cbWndExtra = sizeof(intptr_t);
            wndClass.lpfnWndProc = &HmiSystemWindow::WndProc;
            wndClass.lpszClassName = L"KUMA_HMI_WINDOW";
            RegisterClassW(&wndClass);
            init_flag.store(true);
        }
    }

    RECT rc{0, 0, width, height};
    constexpr DWORD WINDOW_STYLE = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    AdjustWindowRectEx(&rc, WINDOW_STYLE, false, false);
    rc.right -= rc.left;
    rc.bottom -= rc.top;
    m_hWnd = CreateWindowExW(0, L"KUMA_HMI_WINDOW", title.c_str(), WINDOW_STYLE, CW_USEDEFAULT, CW_USEDEFAULT, rc.right, rc.bottom, nullptr, nullptr, hInstance, this);
    ShowWindow(m_hWnd, SW_SHOW);
}

HmiSystemWindow::~HmiSystemWindow()
{
    delete m_redButton;
    delete m_greenButton;
    delete m_graphics;
}

void HmiSystemWindow::SpinOnce()
{
    m_graphics->Render();
}

LRESULT HmiSystemWindow::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if(uMsg != WM_CREATE)
    {
        switch(uMsg)
        {
        case WM_PAINT:
            ValidateRect(hWnd, nullptr);
            return 0;

        case WM_CLOSE:
            PostQuitMessage(0);
            return 0;
        }
    }
    else
    {
        RECT rc{};
        GetClientRect(hWnd, &rc);
        auto graphics = hmi_graphics::System::CreateInstance(hWnd, rc.right - rc.left, rc.bottom - rc.top);
        if(graphics == nullptr)
            return DefWindowProcW(hWnd, uMsg, wParam, lParam);

        auto s = (CREATESTRUCT*)lParam;
        auto instance = (HmiSystemWindow*)s->lpCreateParams;
        SetWindowLongPtrW(hWnd, GWLP_USERDATA, (LONG_PTR)instance);
        instance->m_graphics = graphics;
        instance->m_redButton = graphics->AddElement<ColorButton>(100, 100, D2D1::ColorF(D2D1::ColorF::Red, 0.5f), L"Red Button");
        instance->m_greenButton = graphics->AddElement<ColorButton>(100, 100, D2D1::ColorF(D2D1::ColorF::Green, 1.f), L"Green Button");
        instance->m_redButton->SetPosition(50, 50);
        instance->m_redButton->SetZIndex(1);
        return 0;
    }

    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}
