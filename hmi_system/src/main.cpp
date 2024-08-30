#include <atomic>
#include <mutex>
#include <string>
#include <Windows.h>
#include <graphics/graphics_system.h>
#include <graphics/graphics_element.h>
#include <wrl/client.h>

class RedButton;
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
    RedButton* m_button;
};

class RedButton: public hmi_graphics::GraphicsElement
{
public:
    RedButton() = default;

    bool Initialize(Pimpl* pimpl, hmi_graphics::System* parent) override;

    void Render(hmi_graphics::System* parent) override;
private:
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_brush;
};

bool RedButton::Initialize(Pimpl* pimpl, hmi_graphics::System* parent)
{
    GraphicsElement::Initialize(pimpl, parent);
    parent->GetCachedColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &m_brush);
    return true;
}

void RedButton::Render(hmi_graphics::System* parent)
{
    Microsoft::WRL::ComPtr<ID2D1DeviceContext> context;
    parent->GetDirect2dDeviceContext(&context);
    context->SetTarget(GetTarget());
    context->BeginDraw();
    context->Clear(D2D1::ColorF(D2D1::ColorF::Red));
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
    delete m_button;
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
        instance->m_button = graphics->AddElement<RedButton>(100, 100);
        return 0;
    }

    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}
