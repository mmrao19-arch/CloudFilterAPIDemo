#include "pch.h"

#include "CFAPIWrapper.h"
// Use DllImport to call the native cfAPINativeEngine exports directly from managed C++/CLI.
using namespace System;
using namespace System::Runtime::InteropServices;

private ref class NativeMethods abstract sealed
{
public:
    [DllImport("cfAPINativeEngine.dll", CharSet = CharSet::Unicode, CallingConvention = CallingConvention::StdCall, SetLastError = true)]
    static int RegisterSyncRoot(String^ path, String^ identity, String^ displayName);

    [DllImport("cfAPINativeEngine.dll", CharSet = CharSet::Unicode, CallingConvention = CallingConvention::StdCall, SetLastError = true)]
    static int CreatePlaceholderFile(String^ path, unsigned long long fileSize);

    [DllImport("cfAPINativeEngine.dll", CharSet = CharSet::Unicode, CallingConvention = CallingConvention::StdCall, SetLastError = true)]
    static int TriggerHydration(String^ path);

    [DllImport("cfAPINativeEngine.dll", CharSet = CharSet::Unicode, CallingConvention = CallingConvention::StdCall, SetLastError = true)]
    static int NotifyFileStateChange(String^ path, int state);
};

void CFAPIWrapper::CFApi::RegisterSyncRoot(String^ syncRootPath, String^ identity, String^ displayName)
{
    if (syncRootPath == nullptr) throw gcnew ArgumentNullException("syncRootPath");
    int hr = NativeMethods::RegisterSyncRoot(syncRootPath, identity, displayName);
    if (hr != 0) throw gcnew System::ComponentModel::Win32Exception(hr);
}

void CFAPIWrapper::CFApi::CreatePlaceholderFile(String^ placeholderPath, unsigned long long fileSize)
{
    if (placeholderPath == nullptr) throw gcnew ArgumentNullException("placeholderPath");
    int hr = NativeMethods::CreatePlaceholderFile(placeholderPath, fileSize);
    if (hr != 0) throw gcnew System::ComponentModel::Win32Exception(hr);
}

void CFAPIWrapper::CFApi::TriggerHydration(String^ filePath)
{
    if (filePath == nullptr) throw gcnew ArgumentNullException("filePath");
    int hr = NativeMethods::TriggerHydration(filePath);
    if (hr != 0) throw gcnew System::ComponentModel::Win32Exception(hr);
}

void CFAPIWrapper::CFApi::NotifyFileStateChange(String^ filePath, int state)
{
    if (filePath == nullptr) throw gcnew ArgumentNullException("filePath");
    int hr = NativeMethods::NotifyFileStateChange(filePath, state);
    if (hr != 0) throw gcnew System::ComponentModel::Win32Exception(hr);
}

