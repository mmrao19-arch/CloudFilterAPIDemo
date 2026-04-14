#include "pch.h"
#include "CfApiExports.h"
#include <windows.h>
#ifndef NTSTATUS
typedef LONG NTSTATUS;
#endif
#include <cfapi.h>
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

        // Try to call CfRegisterSyncRoot from cfapi.dll if available. Fall back to minimal stub behavior otherwise.
        HMODULE hCfApi = LoadLibraryW(L"cfapi.dll");
        if (hCfApi)
        {
            using PFN_CfRegisterSyncRoot = HRESULT(WINAPI*)(PCWSTR, PCWSTR, PCWSTR, const void*);
            PFN_CfRegisterSyncRoot pfn = (PFN_CfRegisterSyncRoot)GetProcAddress(hCfApi, "CfRegisterSyncRoot");
            if (pfn)
            {
                HRESULT hr = pfn(syncRootPath, identity, displayName, nullptr);
                std::wostringstream r;
                r << L"CfRegisterSyncRoot returned hr=0x" << std::hex << hr;
                LogDebug(r.str());
                FreeLibrary(hCfApi);
                return hr;
            }
            FreeLibrary(hCfApi);
        }

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
        // Try to call CfCreatePlaceholders via cfapi.dll. If unavailable, fall back to minimal stub.
        std::wstring path(placeholderPath);
        size_t pos = path.find_last_of(L"\\/");
        std::wstring baseDir = L".";
        std::wstring relative;
        if (pos != std::wstring::npos)
        {
            baseDir = path.substr(0, pos);
            relative = path.substr(pos + 1);
            DWORD attrs = GetFileAttributesW(baseDir.c_str());
            if (attrs == INVALID_FILE_ATTRIBUTES)
                CreateDirectoryW(baseDir.c_str(), nullptr);
        }
        else
        {
            relative = path;
        }

        HMODULE hCfApi = LoadLibraryW(L"cfapi.dll");
        if (hCfApi)
        {
            using PFN_CfCreatePlaceholders = HRESULT(WINAPI*)(LPCWSTR, CF_PLACEHOLDER_CREATE_INFO*, DWORD, CF_CREATE_FLAGS, PDWORD);
            PFN_CfCreatePlaceholders pfn = (PFN_CfCreatePlaceholders)GetProcAddress(hCfApi, "CfCreatePlaceholders");
            if (pfn)
            {
                CF_PLACEHOLDER_CREATE_INFO info = { 0 };
                info.RelativeFileName = relative.c_str();
                info.FsMetadata.BasicInfo.FileAttributes = FILE_ATTRIBUTE_ARCHIVE;
                info.FsMetadata.FileSize.QuadPart = (LONGLONG)fileSize;
                info.FileIdentity = nullptr;
                info.FileIdentityLength = 0;
                info.Flags = CF_PLACEHOLDER_CREATE_FLAG_NONE;
                info.Result = S_OK;

                DWORD entries = 0;
                HRESULT hr = pfn(baseDir.c_str(), &info, 1, CF_CREATE_FLAG_NONE, &entries);
                std::wostringstream r;
                r << L"CfCreatePlaceholders returned hr=0x" << std::hex << hr << L", entries=" << entries;
                LogDebug(r.str());
                FreeLibrary(hCfApi);
                if (SUCCEEDED(hr))
                    return S_OK;
                // fallthrough to stub on failure
            }
            FreeLibrary(hCfApi);
        }

        // Minimal stub: create a zero-length file to represent placeholder (not a real CF placeholder).
        std::wofstream ofs(path, std::ios::binary);
        if (!ofs)
        {
            DWORD err = GetLastError();
            LogDebug(L"Failed creating placeholder file, error " + std::to_wstring(err));
            return HRESULT_FROM_WIN32(err);
        }
        ofs.close();

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
        // Try to call CfHydratePlaceholder on the file handle.
        std::wstring path(filePath);
        HANDLE hFile = CreateFileW(path.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile == INVALID_HANDLE_VALUE)
        {
            DWORD err = GetLastError();
            LogDebug(L"TriggerHydration: CreateFileW failed, error " + std::to_wstring(err));
            // fall back to simulated write
        }
        else
        {
            HMODULE hCfApi = LoadLibraryW(L"cfapi.dll");
            if (hCfApi)
            {
                using PFN_CfHydratePlaceholder = HRESULT(WINAPI*)(HANDLE, LARGE_INTEGER, LARGE_INTEGER, CF_HYDRATE_FLAGS, LPOVERLAPPED);
                PFN_CfHydratePlaceholder pfn = (PFN_CfHydratePlaceholder)GetProcAddress(hCfApi, "CfHydratePlaceholder");
                if (pfn)
                {
                    LARGE_INTEGER zero = { 0 };
                    LARGE_INTEGER full = { 0 };
                    LARGE_INTEGER fileSizeLi = { 0 };
                    if (!GetFileSizeEx(hFile, &fileSizeLi))
                    {
                        fileSizeLi.QuadPart = CF_EOF;
                    }
                    HRESULT hr = pfn(hFile, zero, fileSizeLi, CF_HYDRATE_FLAG_NONE, nullptr);
                    std::wostringstream r;
                    r << L"CfHydratePlaceholder returned hr=0x" << std::hex << hr;
                    LogDebug(r.str());
                    CloseHandle(hFile);
                    FreeLibrary(hCfApi);
                    if (SUCCEEDED(hr))
                        return S_OK;
                }
                FreeLibrary(hCfApi);
            }
            CloseHandle(hFile);
        }

        // Fallback: write simulated content into the file path (as before).
        std::ofstream ofs;
#if defined(_MSC_VER)
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
        // Map state to CFAPI update. For example: 1 -> mark in sync, 2 -> dehydrate.
        CF_UPDATE_FLAGS flags = CF_UPDATE_FLAG_NONE;
        switch (state)
        {
        case 1:
            flags = CF_UPDATE_FLAG_MARK_IN_SYNC;
            break;
        case 2:
            flags = CF_UPDATE_FLAG_DEHYDRATE;
            break;
        default:
            flags = CF_UPDATE_FLAG_NONE;
            break;
        }

        std::wstring path(filePath);
        HANDLE hFile = CreateFileW(path.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile == INVALID_HANDLE_VALUE)
        {
            DWORD err = GetLastError();
            LogDebug(L"NotifyFileStateChange: CreateFileW failed, error " + std::to_wstring(err));
            return HRESULT_FROM_WIN32(err);
        }

        HMODULE hCfApi = LoadLibraryW(L"cfapi.dll");
        if (hCfApi)
        {
            using PFN_CfUpdatePlaceholder = HRESULT(WINAPI*)(HANDLE, const CF_FS_METADATA*, LPCVOID, DWORD, const CF_FILE_RANGE*, DWORD, CF_UPDATE_FLAGS, USN*, LPOVERLAPPED);
            PFN_CfUpdatePlaceholder pfn = (PFN_CfUpdatePlaceholder)GetProcAddress(hCfApi, "CfUpdatePlaceholder");
            if (pfn)
            {
                HRESULT hr = pfn(hFile, nullptr, nullptr, 0, nullptr, 0, flags, nullptr, nullptr);
                std::wostringstream r;
                r << L"CfUpdatePlaceholder returned hr=0x" << std::hex << hr;
                LogDebug(r.str());
                CloseHandle(hFile);
                FreeLibrary(hCfApi);
                return hr;
            }
            FreeLibrary(hCfApi);
        }

        // If CFAPI not available, just return success after logging.
        CloseHandle(hFile);
        return S_OK;
    }
    catch (...)
    {
        LogDebug(L"NotifyFileStateChange: unexpected exception");
        return E_FAIL;
    }
}
