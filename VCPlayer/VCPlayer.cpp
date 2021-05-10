// VCPlayer.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "VCPlayer.h"
#include <MMDeviceAPI.h>
#include <functiondiscoverykeys.h>
#include <strsafe.h>

#define MAX_LOADSTRING 100

// 全局变量:
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名

// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

template <class T> void SafeRelease(T** ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}

LPWSTR GetDeviceName(IMMDeviceCollection* DeviceCollection, UINT DeviceIndex)
{
    IMMDevice *device;
    LPWSTR deviceId;
    HRESULT hr;
    hr = DeviceCollection->Item(DeviceIndex, &device);
    if (FAILED(hr))
    {
        OutputDebugStringW(L"DeviceCollection->Item error");
    }
    else
    {
        OutputDebugStringW(L"DeviceCollection->Item success");
    }
    device->GetId(&deviceId);

    IPropertyStore *propertyStore;
    hr = device->OpenPropertyStore(STGM_READ, &propertyStore);
    
    if (FAILED(hr))
    {
        OutputDebugStringW(L"OpenPropertyStore error");
    }
    else
    {
        OutputDebugStringW(L"OpenPropertyStore success");
    }

    PROPVARIANT friendlyName; 
    hr = propertyStore->GetValue(PKEY_Device_FriendlyName, &friendlyName);

    if (FAILED(hr))
    {
        OutputDebugStringW(L"PKEY_Device_FriendlyName error");
    }
    else
    {
        OutputDebugStringW(L"PKEY_Device_FriendlyName success");
    }
    SafeRelease(&propertyStore);

    wchar_t deviceName[128];
    hr = StringCbPrintf(deviceName, sizeof(deviceName), L"%s (%s)", friendlyName.vt != VT_LPWSTR ? L"Unknown" : friendlyName.pwszVal, deviceId);
    if (FAILED(hr))
    {
        OutputDebugStringW(L"StringCbPrintf error");
    }
    else
    {
        OutputDebugStringW(L"StringCbPrintf success");
    }

    PropVariantClear(&friendlyName);
    CoTaskMemFree(deviceId);

    wchar_t* returnValue = _wcsdup(deviceName);
    if (returnValue == NULL)
    {
        printf("Unable to allocate buffer for return\n");
        return NULL;
    }
    return returnValue;
}


bool PickDevice(IMMDevice** DeviceToUse, bool* IsDefaultDevice, ERole* DefaultDeviceRole)
{
    HRESULT hr;
    IMMDeviceEnumerator *deviceEnumerator = NULL;
    IMMDeviceCollection* deviceCollection = NULL;

    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&deviceEnumerator));

    if (FAILED(hr)) {
        OutputDebugStringW(L"MMDeviceEnumerator error");
    }
    else
    {
        OutputDebugStringW(L"MMDeviceEnumerator success");
    }

    hr = deviceEnumerator->EnumAudioEndpoints(eCapture, DEVICE_STATE_ACTIVE, &deviceCollection);

    if (FAILED(hr)) 
    {
        OutputDebugStringW(L"EnumAudioEndpoints error");
    }
    else
    {
        OutputDebugStringW(L"EnumAudioEndpoints success");
    }

    UINT deviceCount;
    hr = deviceCollection->GetCount(&deviceCount);

    if (FAILED(hr)) {
        OutputDebugStringW(L"GetCount error");
    }
    else
    {
        OutputDebugStringW(L"GetCount success");
    }

    for (UINT i = 0; i < deviceCount; i += 1)
    {
        LPWSTR deviceName;

        deviceName = GetDeviceName(deviceCollection, i);
        if (deviceName == NULL)
        {
            OutputDebugStringW(L"deviceName == NULL");
        }
        printf("    %d:  %S\n", i + 3, deviceName);
        free(deviceName);
    }
    IMMDevice* device = NULL;
    ERole deviceRole = eConsole;
    hr = deviceEnumerator->GetDefaultAudioEndpoint(eCapture, deviceRole, &device);
    
    if (FAILED(hr)) {
        OutputDebugStringW(L"GetDefaultAudioEndpoint error");
    }
    else
    {
        OutputDebugStringW(L"GetDefaultAudioEndpoint success");
    }
    *IsDefaultDevice = true;
    *DefaultDeviceRole = deviceRole;
    *DeviceToUse = device;

    return true;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    IMMDevice* device = NULL;
    ERole role;
    bool isDefaultDevice;

    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    if (FAILED(hr)) 
    {
        OutputDebugStringW(L"CoInitializeEx Fail");
    }
    else {
        OutputDebugStringW(L"CoInitializeEx Success");
    }
    PickDevice(&device, &isDefaultDevice, &role);
    // TODO: 在此处放置代码。

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_VCPLAYER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_VCPLAYER));

    MSG msg;

    // 主消息循环:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_VCPLAYER));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_VCPLAYER);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 分析菜单选择:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 在此处添加使用 hdc 的任何绘图代码...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
