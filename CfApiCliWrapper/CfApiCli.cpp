// C++/CLI wrapper for native cfAPINativeEngine.dll
// Provides a managed surface callable from C# (WPF) that forwards to the native exports via LoadLibrary/GetProcAddress.

#include <windows.h>
#include <string>
#include <mutex>

// Avoid 'using namespace System' to prevent symbol collisions with Windows headers (IServiceProvider etc.)
#include <vcclr.h>


typedef HRESULT(__stdcall* fnRegister)(const wchar_t*, const wchar_t*, const wchar_t*);
typedef HRESULT(__stdcall* fnCreatePlaceholder)(const wchar_t*, unsigned __int64);
typedef HRESULT(__stdcall* fnTriggerHydration)(const wchar_t*);
typedef HRESULT(__stdcall* fnNotify)(const wchar_t*, int);

static HMODULE g_hModule = NULL;
static fnRegister g_Register = NULL;
static fnCreatePlaceholder g_CreatePlaceholder = NULL;
static fnTriggerHydration g_TriggerHydration = NULL;
static fnNotify g_Notify = NULL;
static std::once_flag g_initFlag;

static void LoadNativeExports()
{
    // Attempt to load the native provider DLL from the process directory.
    g_hModule = LoadLibraryW(L"cfAPINativeEngine.dll");
    if (!g_hModule)
        return;

    g_Register = (fnRegister)GetProcAddress(g_hModule, "RegisterSyncRoot");
    g_CreatePlaceholder = (fnCreatePlaceholder)GetProcAddress(g_hModule, "CreatePlaceholderFile");
    g_TriggerHydration = (fnTriggerHydration)GetProcAddress(g_hModule, "TriggerHydration");
    g_Notify = (fnNotify)GetProcAddress(g_hModule, "NotifyFileStateChange");
}

static void EnsureLoadedOrThrow()
{
    std::call_once(g_initFlag, LoadNativeExports);
    if (!g_hModule || !g_Register || !g_CreatePlaceholder || !g_TriggerHydration || !g_Notify)
        throw gcnew DllNotFoundException("Failed to load cfAPINativeEngine.dll or its exports. Make sure the native DLL is available next to the app executable.");
}

namespace CfApiCliWrapper {

    public ref class CfApiManaged sealed
    {
    public:
        static void RegisterSyncRoot(System::String^ syncRootPath, System::String^ identity, System::String^ displayName)
        {
            EnsureLoadedOrThrow();
            if (syncRootPath == nullptr)
                throw gcnew ArgumentNullException("syncRootPath");

            pin_ptr<const wchar_t> pPath = PtrToStringChars(syncRootPath);
            pin_ptr<const wchar_t> pId = identity ? PtrToStringChars(identity) : nullptr;
            pin_ptr<const wchar_t> pName = displayName ? PtrToStringChars(displayName) : nullptr;

            HRESULT hr = g_Register(pPath, pId, pName);
            if (FAILED(hr))
                throw gcnew System::Runtime::InteropServices::COMException("RegisterSyncRoot failed.", hr);
        }

        static void CreatePlaceholderFile(System::String^ placeholderPath, unsigned long long fileSize)
        {
            EnsureLoadedOrThrow();
            if (placeholderPath == nullptr)
                throw gcnew ArgumentNullException("placeholderPath");

            pin_ptr<const wchar_t> pPath = PtrToStringChars(placeholderPath);
            HRESULT hr = g_CreatePlaceholder(pPath, (unsigned __int64)fileSize);
            if (FAILED(hr))
                throw gcnew System::Runtime::InteropServices::COMException("CreatePlaceholderFile failed.", hr);
        }

        static void TriggerHydration(System::String^ filePath)
        {
            EnsureLoadedOrThrow();
            if (filePath == nullptr)
                throw gcnew ArgumentNullException("filePath");

            pin_ptr<const wchar_t> pPath = PtrToStringChars(filePath);
            HRESULT hr = g_TriggerHydration(pPath);
            if (FAILED(hr))
                throw gcnew System::Runtime::InteropServices::COMException("TriggerHydration failed.", hr);
        }

        static void NotifyFileStateChange(System::String^ filePath, int state)
        {
            EnsureLoadedOrThrow();
            if (filePath == nullptr)
                throw gcnew ArgumentNullException("filePath");

            pin_ptr<const wchar_t> pPath = PtrToStringChars(filePath);
            HRESULT hr = g_Notify(pPath, state);
            if (FAILED(hr))
                throw gcnew System::Runtime::InteropServices::COMException("NotifyFileStateChange failed.", hr);
        }
    };

}
