#include <atomic>
#include <mutex>
#include <string>
#include <array>
#include <Windows.h>
#include <strsafe.h>
#include <graphics/graphics_system.h>
#include <graphics/graphics_element.h>
#include <wrl/client.h>

class ColorButton;

__interface IHmiApplication;
__interface IHmiRenderer;
__interface IHmiRenderManager;
__interface IHmiSession;
__interface IHmiApplicationView;
__interface IHmiSystem: IUnknown
{
    hmi_graphics::System* GetGraphicsSystem();

    bool BindElement(hmi_graphics::GraphicsElement* element, const UUID* appUuid);
};

__interface IHmiModule: IUnknown
{
    STDMETHOD(OnLoaded)(IHmiSystem*);

    STDMETHOD(OnShutdown)();

    STDMETHOD(OnSpin)();

    STDMETHOD(GetApplication)(IHmiApplication** application);

    STDMETHOD(GetRenderer)(IHmiRenderer** renderer);
};

__interface IHmiApplication: IUnknown
{
    HRESULT OnHit(int32_t x, int32_t y);

    HRESULT GetUuid(UUID* guid);
};

__interface IHmiRenderer: IUnknown
{
    HRESULT GetUuid(UUID* guid);
};

__interface IHmiRenderManager: IUnknown
{

};

class BazelLabel : public hmi_graphics::GraphicsElement
{
public:
    explicit BazelLabel(const D2D1_COLOR_F& color, const std::wstring& title);

    auto Initialize(Pimpl* pimpl, hmi_graphics::System* parent) -> bool override;

    auto Render(hmi_graphics::System* parent) -> void override;

    auto SetText(const std::wstring& label) -> void;

private:
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_brush;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_blackBrush;
    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_textFormat;
    Microsoft::WRL::ComPtr<IDWriteTextLayout> m_textLayout;
    D2D1_COLOR_F m_color;
    std::wstring m_label;
};

BazelLabel::BazelLabel(const D2D1_COLOR_F& color, const std::wstring& title)
{
    m_color = color;
    m_label = title;
}

auto BazelLabel::Initialize(Pimpl* pimpl, hmi_graphics::System* parent) -> bool
{
    hmi_graphics::GraphicsElement::Initialize(pimpl, parent);
    parent->GetCachedColorBrush(m_color, &m_brush);
    parent->GetCachedColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &m_blackBrush);
    Microsoft::WRL::ComPtr<IDWriteFactory> dwriteFactory;
    parent->GetDirectWriteFactory(&dwriteFactory);
    dwriteFactory->CreateTextFormat(L"arial", nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL, 11.f, L"", &m_textFormat);
    m_textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    m_textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    auto size = GetSize();
    dwriteFactory->CreateTextLayout(m_label.c_str(), m_label.size(), m_textFormat.Get(), size.width, size.height,
        &m_textLayout);

    return true;
}

auto BazelLabel::SetText(const std::wstring& label) -> void
{
    m_label = label;
    Microsoft::WRL::ComPtr<IDWriteFactory> dwriteFactory;
    auto size = GetSize();
    dwriteFactory->CreateTextLayout(m_label.c_str(), m_label.size(), m_textFormat.Get(), size.width,
        size.height, &m_textLayout);

    GraphicsElement::NotifyUpdated();
}

auto BazelLabel::Render(hmi_graphics::System* parent) -> void
{
    Microsoft::WRL::ComPtr<ID2D1DeviceContext> context;
    parent->GetDirect2dDeviceContext(&context);
    context->SetTarget(GetTarget());
    context->BeginDraw();
    context->Clear(m_color);
    context->DrawTextLayout(D2D1::Point2(0.f, 0.f), m_textLayout.Get(), m_blackBrush.Get());
    context->EndDraw();
}

class PlanPositionIndicator : public hmi_graphics::GraphicsElement
{
public:
    explicit PlanPositionIndicator(float angleHeadingRad);

    auto Initialize(Pimpl* pimpl, hmi_graphics::System* parent) -> bool override;

    auto Render(hmi_graphics::System* parent) -> void override;

    auto SetAngleHeadingRad(float radian) -> void;

private:
    float m_angleHeadingRad;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_brush;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_blackBrush;
    Microsoft::WRL::ComPtr<IDWriteTextFormat> m_textFormat;
};

PlanPositionIndicator::PlanPositionIndicator(float angleHeadingRad)
    : m_angleHeadingRad{ angleHeadingRad }
{
    
}

auto PlanPositionIndicator::Initialize(Pimpl* pimpl, hmi_graphics::System* parent) -> bool
{
    if (!GraphicsElement::Initialize(pimpl, parent))
    {
        return false;
    }

    hmi_graphics::GraphicsElement::Initialize(pimpl, parent);
    parent->GetCachedColorBrush(D2D1::ColorF{D2D1::ColorF::Red}, &m_brush);
    parent->GetCachedColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &m_blackBrush);
    Microsoft::WRL::ComPtr<IDWriteFactory> dwriteFactory;
    parent->GetDirectWriteFactory(&dwriteFactory);
    dwriteFactory->CreateTextFormat(L"arial", nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL, 11.f, L"", &m_textFormat);
    m_textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    m_textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

    Microsoft::WRL::ComPtr<ID2D1DeviceContext> context;
    parent->GetDirect2dDeviceContext(&context);

    auto size = GetSize();
    return true;
}

auto PlanPositionIndicator::SetAngleHeadingRad(float radian) -> void
{
    m_angleHeadingRad = radian;
    NotifyUpdated();
}

auto PlanPositionIndicator::Render(hmi_graphics::System* parent) -> void
{
    Microsoft::WRL::ComPtr<ID2D1DeviceContext> context;
    parent->GetDirect2dDeviceContext(&context);
    context->SetTarget(GetTarget());
    context->BeginDraw();
    context->Clear(D2D1::ColorF{D2D1::ColorF::White, 0.f});
    
    auto size = GetSize();

    context->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(size.width / 2, size.height / 2), size.width / 2 - 2, size.height / 2 - 2),
        m_blackBrush.Get(), 2.f);

    context->SetTransform(D2D1::Matrix3x2F::Scale(D2D1::SizeF(1.F, -1.f)) * D2D1::Matrix3x2F::Rotation(m_angleHeadingRad) * D2D1::Matrix3x2F::Translation(size.width / 2, size.height / 2));
    context->FillRectangle(D2D1::RectF(-20.f, 30.f, 20.f, -30.f), m_brush.Get());
    
    context->EndDraw();
    context->SetTransform(D2D1::IdentityMatrix());
}

class ExampleRenderManager : public IHmiRenderManager
{
public:
    ExampleRenderManager() = default;

    ~ExampleRenderManager();

    auto QueryInterface(const GUID& riid, void** ppvObject) -> HRESULT override
    {
        return E_NOTIMPL;
    }

    auto AddRef() -> ULONG override
    {
        int refCnt = m_refCnt.fetch_add(1);
        if (refCnt <= 0)
        {
            throw std::runtime_error("");
        }

        return refCnt + 1;
    }

    auto Release() -> ULONG override
    {
        int refCnt = m_refCnt.fetch_sub(1);
        if (refCnt == 1)
        {
            delete this;
        }

        return refCnt - 1;
    }

    auto Initialize(hmi_graphics::System* system) -> HRESULT;

    auto SpinOnce() -> HRESULT;

private:
    std::atomic_int m_refCnt = 1;
    PlanPositionIndicator* m_ppi = nullptr;
    std::array<BazelLabel*, 20> m_bazelButtons = {};
};

ExampleRenderManager::~ExampleRenderManager()
{
    delete m_ppi;
    for (auto it : m_bazelButtons)
    {
        delete it;
    }
}

auto ExampleRenderManager::Initialize(hmi_graphics::System* system) -> HRESULT
{
    m_ppi = system->AddElement<PlanPositionIndicator>(100, 100, 30.f);
    for (auto& it : m_bazelButtons)
    {
        wchar_t buf[16]{};
        const int index = &it - m_bazelButtons.data();
        StringCbPrintfW(buf, sizeof(buf) - 1, L"F%02d", index + 1);
        int width = 90;
        int height = 70;
        if (index > 11)
        {
            height = 35;
        }

        it = system->AddElement<BazelLabel>(width, height, D2D1::ColorF(D2D1::ColorF::Green, 0.5f), buf);
    }

    m_ppi->SetPosition(110, 80);
    m_bazelButtons[0]->SetPosition(5, 70 + 75 * 0);
    m_bazelButtons[1]->SetPosition(5, 70 + 75 * 1);
    m_bazelButtons[2]->SetPosition(5, 70 + 75 * 2);
    m_bazelButtons[3]->SetPosition(5, 70 + 75 * 3);
    m_bazelButtons[4]->SetPosition(5, 70 + 75 * 4);
    m_bazelButtons[5]->SetPosition(5, 70 + 75 * 5);
    m_bazelButtons[6]->SetPosition(800 - 90 - 5, 70 + 75 * 0);
    m_bazelButtons[7]->SetPosition(800 - 90 - 5, 70 + 75 * 1);
    m_bazelButtons[8]->SetPosition(800 - 90 - 5, 70 + 75 * 2);
    m_bazelButtons[9]->SetPosition(800 - 90 - 5, 70 + 75 * 3);
    m_bazelButtons[10]->SetPosition(800 - 90 - 5, 70 + 75 * 4);
    m_bazelButtons[11]->SetPosition(800 - 90 - 5, 70 + 75 * 5);
    m_bazelButtons[12]->SetPosition(5 + 100 * 0, 600 - 40);
    m_bazelButtons[13]->SetPosition(5 + 100 * 1, 600 - 40);
    m_bazelButtons[14]->SetPosition(5 + 100 * 2, 600 - 40);
    m_bazelButtons[15]->SetPosition(5 + 100 * 3, 600 - 40);
    m_bazelButtons[16]->SetPosition(5 + 100 * 4, 600 - 40);
    m_bazelButtons[17]->SetPosition(5 + 100 * 5, 600 - 40);
    m_bazelButtons[18]->SetPosition(5 + 100 * 6, 600 - 40);
    m_bazelButtons[19]->SetPosition(5 + 100 * 7, 600 - 40);

    return S_OK;
}

auto ExampleRenderManager::SpinOnce() -> HRESULT
{
    return S_OK;
}

class ApplicationLoader
{
public:
    ApplicationLoader();

    ~ApplicationLoader();
private:

};

class HmiSystemWindow
{
public:
    HmiSystemWindow(const std::wstring &title, int width, int height);

    ~HmiSystemWindow();

    void SpinOnce();

    hmi_graphics::System* GetGraphics() { return m_graphics; }

private:
    static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
    HWND m_hWnd;
    hmi_graphics::System* m_graphics;
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
    dwriteFactory->CreateTextFormat(L"arial", nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL, 11.f, L"", &m_textFormat);
    m_textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    m_textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    auto size = GetSize();
    dwriteFactory->CreateTextLayout(m_label.c_str(), m_label.size(), m_textFormat.Get(), size.width,
        size.height, &m_textLayout);
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

    ExampleRenderManager* manager = new ExampleRenderManager{};
    manager->Initialize(window.GetGraphics());
    
    MSG message{};
    while(message.message != WM_QUIT)
    {
        if(PeekMessage(&message, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
           
        manager->SpinOnce();
        window.SpinOnce();
    }

    manager->Release();

    return 0;
}

static std::atomic_bool init_flag;
static std::mutex init_mutex;

HmiSystemWindow::HmiSystemWindow(const std::wstring& title, int width, int height)
    : m_hWnd(nullptr)
    , m_graphics()
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
    m_hWnd = CreateWindowExW(0, L"KUMA_HMI_WINDOW", title.c_str(), WINDOW_STYLE, CW_USEDEFAULT, CW_USEDEFAULT,
        rc.right, rc.bottom, nullptr, nullptr, hInstance, this);
    ShowWindow(m_hWnd, SW_SHOW);
}

HmiSystemWindow::~HmiSystemWindow()
{
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
        return 0;
    }

    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}
