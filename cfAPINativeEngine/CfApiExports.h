#pragma once
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

// Exported functions for C# interop (P/Invoke).
// All functions return HRESULT; on failure they set GetLastError where appropriate.

// Register the sync root path. `syncRootPath` must be a null-terminated UTF-16 path.
// `identity` / `displayName` are optional human-readable strings.
__declspec(dllexport) HRESULT __stdcall RegisterSyncRoot(const wchar_t* syncRootPath, const wchar_t* identity, const wchar_t* displayName);

// Create a single placeholder file on disk. `placeholderPath` is the full file path.
// `fileSize` is the cloud file size. This function should call CfCreatePlaceholders in a real implementation.
__declspec(dllexport) HRESULT __stdcall CreatePlaceholderFile(const wchar_t* placeholderPath, unsigned __int64 fileSize);

// Trigger hydration (download) for a single file. In a real implementation this will start the CFAPI hydrate flow.
__declspec(dllexport) HRESULT __stdcall TriggerHydration(const wchar_t* filePath);

// Notify the native engine that file state changed (e.g., uploaded, deleted). `state` is provider-defined.
__declspec(dllexport) HRESULT __stdcall NotifyFileStateChange(const wchar_t* filePath, int state);

#ifdef __cplusplus
}
#endif
