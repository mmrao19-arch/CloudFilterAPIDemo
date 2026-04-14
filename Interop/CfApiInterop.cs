using System;
using System.Runtime.InteropServices;

internal static class CfApiInterop
{
    private const string DllName = "cfAPINativeEngine.dll";

    [DllImport(DllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
    internal static extern int RegisterSyncRoot(string syncRootPath, string identity, string displayName);

    [DllImport(DllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
    internal static extern int CreatePlaceholderFile(string placeholderPath, ulong fileSize);

    [DllImport(DllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
    internal static extern int TriggerHydration(string filePath);

    [DllImport(DllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
    internal static extern int NotifyFileStateChange(string filePath, int state);

    // Small managed-friendly wrappers:
    internal static void RegisterSyncRootSafe(string path, string id, string name)
    {
        int hr = RegisterSyncRoot(path, id, name);
        if (hr != 0) throw new System.ComponentModel.Win32Exception(hr);
    }
}
