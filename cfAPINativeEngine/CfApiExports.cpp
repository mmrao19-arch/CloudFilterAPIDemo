#include "pch.h"
#include "CfApiExports.h"
#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>

// Simple helper to log (visible in Visual Studio Output window).
static void LogDebug(const std::wstring& msg)
{
    std::wstring out = L"[CfApiExports] " + msg + L"\n";
    OutputDebugStringW(out.c_str());
}

HRESULT __stdcall RegisterSyncRoot(const wchar_t* syncRootPath, const wchar_t* identity, const wchar_t* displayName)
{
    if (!syncRootPath)
        return E_INVALIDARG;

    try
    {
        std::wostringstream ss;
        ss << L"RegisterSyncRoot called. Path=\"" << syncRootPath << L"\"";
        if (identity) ss << L", Identity=\"" << identity << L"\"";
        if (displayName) ss << L", DisplayName=\"" << displayName << L"\"";
        LogDebug(ss.str());

        // TODO: Replace this stub with real CFAPI call:
        // #include <cfapi.h>
        // HRESULT hr = CfRegisterSyncRoot(...);   // follow MSDN signature
        // return hr;

        // Minimal behavior: create the directory if needed so explorer can see it.
        DWORD attrs = GetFileAttributesW(syncRootPath);
        if (attrs == INVALID_FILE_ATTRIBUTES)
        {
            if (!CreateDirectoryW(syncRootPath, nullptr))
            {
                DWORD err = GetLastError();
                LogDebug(L"CreateDirectoryW failed with error " + std::to_wstring(err));
                return HRESULT_FROM_WIN32(err);
            }
            LogDebug(L"Sync root directory created.");
        }

        return S_OK;
    }
    catch (...)
    {
        LogDebug(L"RegisterSyncRoot: unexpected exception");
        return E_FAIL;
    }
}

HRESULT __stdcall CreatePlaceholderFile(const wchar_t* placeholderPath, unsigned __int64 fileSize)
{
    if (!placeholderPath)
        return E_INVALIDARG;

    try
    {
        std::wostringstream ss;
        ss << L"CreatePlaceholderFile called. Path=\"" << placeholderPath << L"\", Size=" << fileSize;
        LogDebug(ss.str());

        // TODO: Replace with CfCreatePlaceholders (CFAPI). The real implementation must create a placeholder
        // structure, set the necessary reparse point and CF placeholder metadata.
        // Minimal stub: create an empty file and mark it with FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS if desired.

        // Create directory path if needed.
        std::wstring path(placeholderPath);
        size_t pos = path.find_last_of(L"\\/");
        if (pos != std::wstring::npos)
        {
            std::wstring dir = path.substr(0, pos);
            DWORD attrs = GetFileAttributesW(dir.c_str());
            if (attrs == INVALID_FILE_ATTRIBUTES)
                CreateDirectoryW(dir.c_str(), nullptr);
        }

        // Create a zero-length file to represent placeholder (stub).
        std::wofstream ofs(path, std::ios::binary);
        if (!ofs)
        {
            DWORD err = GetLastError();
            LogDebug(L"Failed creating placeholder file, error " + std::to_wstring(err));
            return HRESULT_FROM_WIN32(err);
        }
        ofs.close();

        // Optionally set placeholder attributes (not true CF placeholder).
        // SetFileAttributesW(path.c_str(), FILE_ATTRIBUTE_ARCHIVE);

        return S_OK;
    }
    catch (...)
    {
        LogDebug(L"CreatePlaceholderFile: unexpected exception");
        return E_FAIL;
    }
}

HRESULT __stdcall TriggerHydration(const wchar_t* filePath)
{
    if (!filePath)
        return E_INVALIDARG;

    try
    {
        std::wostringstream ss;
        ss << L"TriggerHydration called. Path=\"" << filePath << L"\"";
        LogDebug(ss.str());

        // TODO: In a CFAPI implementation, call CfExecute or queue a hydration callback which will
        // open the network stream (LocalStack/S3), download content, and write into the placeholder
        // using CfSetPlaceholderData or by writing into the hydrated file.

        // Simulated hydration: write sample content into the file to simulate download.
        std::wstring path(filePath);
        std::ofstream ofs;
#if defined(_MSC_VER)
        // Convert wstring to narrow for std::ofstream (simple conversion, adequate for ASCII test)
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, path.c_str(), -1, NULL, 0, NULL, NULL);
        std::string utf8Path(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, path.c_str(), -1, &utf8Path[0], size_needed, NULL, NULL);
        utf8Path.resize(size_needed - 1);
        ofs.open(utf8Path, std::ios::binary | std::ios::trunc);
#else
        ofs.open(std::string(path.begin(), path.end()), std::ios::binary | std::ios::trunc);
#endif
        if (!ofs)
        {
            DWORD err = GetLastError();
            LogDebug(L"TriggerHydration: failed to open file for write, error " + std::to_wstring(err));
            return HRESULT_FROM_WIN32(err);
        }

        const char* content = "Hydrated content (simulated)\n";
        ofs.write(content, (std::streamsize)strlen(content));
        ofs.close();

        LogDebug(L"TriggerHydration: wrote simulated content.");
        return S_OK;
    }
    catch (...)
    {
        LogDebug(L"TriggerHydration: unexpected exception");
        return E_FAIL;
    }
}

HRESULT __stdcall NotifyFileStateChange(const wchar_t* filePath, int state)
{
    if (!filePath)
        return E_INVALIDARG;

    try
    {
        std::wostringstream ss;
        ss << L"NotifyFileStateChange called. Path=\"" << filePath << L"\", State=" << state;
        LogDebug(ss.str());

        // TODO: Map `state` to your domain (e.g., 0=Pending, 1=Synced, 2=Downloaded) and update
        // internal metadata plus call CfReportProviderProgress / CFAPI notifications if required.

        return S_OK;
    }
    catch (...)
    {
        LogDebug(L"NotifyFileStateChange: unexpected exception");
        return E_FAIL;
    }
}
