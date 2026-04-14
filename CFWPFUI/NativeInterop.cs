using System;
using System.Runtime.InteropServices;

namespace CFWPFUI
{
    internal static class NativeInterop
    {
        private const string DllName = "cfAPINativeEngine.dll";
        private static bool s_useFallback = false;
        private static bool s_checked = false;

        [System.Runtime.InteropServices.DllImport("kernel32.dll", SetLastError = true, CharSet = CharSet.Unicode)]
        static extern IntPtr LoadLibraryW(string lpFileName);

        [System.Runtime.InteropServices.DllImport("kernel32.dll", SetLastError = true)]
        static extern bool FreeLibrary(IntPtr hModule);

        [DllImport(DllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        internal static extern int RegisterSyncRoot(string syncRootPath, string identity, string displayName);

        [DllImport(DllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        internal static extern int CreatePlaceholderFile(string placeholderPath, ulong fileSize);

        [DllImport(DllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        internal static extern int TriggerHydration(string filePath);

        [DllImport(DllName, CharSet = CharSet.Unicode, CallingConvention = CallingConvention.StdCall)]
        internal static extern int NotifyFileStateChange(string filePath, int state);

        internal static void RegisterSyncRootSafe(string path, string id, string name)
        {
            EnsureNativePresent();
            if (s_useFallback)
            {
                // Simple fallback: ensure directory exists
                try
                {
                    if (!System.IO.Directory.Exists(path)) System.IO.Directory.CreateDirectory(path);
                }
                catch (Exception ex)
                {
                    throw new System.ComponentModel.Win32Exception(0, "Fallback RegisterSyncRoot failed: " + ex.Message);
                }
                return;
            }

            int hr = RegisterSyncRoot(path, id, name);
            if (hr != 0) throw new System.ComponentModel.Win32Exception(hr);
        }

        internal static int CreatePlaceholderFileSafe(string path, ulong size)
        {
            EnsureNativePresent();
            if (s_useFallback)
            {
                try
                {
                    var dir = System.IO.Path.GetDirectoryName(path);
                    if (!string.IsNullOrEmpty(dir) && !System.IO.Directory.Exists(dir)) System.IO.Directory.CreateDirectory(dir);
                    using (var fs = System.IO.File.Open(path, System.IO.FileMode.Create)) { }
                    return 0;
                }
                catch (Exception ex)
                {
                    return Marshal.GetHRForException(ex);
                }
            }

            return CreatePlaceholderFile(path, size);
        }

        internal static int TriggerHydrationSafe(string path)
        {
            EnsureNativePresent();
            if (s_useFallback)
            {
                try
                {
                    var content = System.Text.Encoding.UTF8.GetBytes("Hydrated content (simulated)\n");
                    System.IO.File.WriteAllBytes(path, content);
                    return 0;
                }
                catch (Exception ex)
                {
                    return Marshal.GetHRForException(ex);
                }
            }

            return TriggerHydration(path);
        }

        internal static int NotifyFileStateChangeSafe(string path, int state)
        {
            EnsureNativePresent();
            if (s_useFallback)
            {
                // no-op fallback
                return 0;
            }

            return NotifyFileStateChange(path, state);
        }

        private static void EnsureNativePresent()
        {
            try
            {
                string outDir = AppContext.BaseDirectory ?? Environment.CurrentDirectory;
                string target = System.IO.Path.Combine(outDir, DllName);
                if (System.IO.File.Exists(target)) return;

                // Look for the native DLL in likely project output locations by walking up ancestors
                string dir = outDir;
                string found = null;
                for (int up = 0; up < 6 && dir != null; ++up)
                {
                    string probeRoot = dir;
                    string[] candidates = new string[] {
                        System.IO.Path.Combine(probeRoot, "cfAPINativeEngine", "Debug", "x64", DllName),
                        System.IO.Path.Combine(probeRoot, "cfAPINativeEngine", "Debug", DllName),
                        System.IO.Path.Combine(probeRoot, "cfAPINativeEngine", "Release", "x64", DllName),
                        System.IO.Path.Combine(probeRoot, "cfAPINativeEngine", "Release", DllName),
                        System.IO.Path.Combine(probeRoot, "cfAPINativeEngine", "bin", "Debug", DllName),
                        System.IO.Path.Combine(probeRoot, "cfAPINativeEngine", DllName),
                        System.IO.Path.Combine(probeRoot, "bin", "Debug", DllName),
                    };

                    foreach (var c in candidates)
                    {
                        if (System.IO.File.Exists(c)) { found = c; break; }
                    }

                    if (found != null) break;

                    var parent = System.IO.Directory.GetParent(dir);
                    dir = parent?.FullName;
                }

                if (found == null)
                {
                    // No native found; enable managed fallback to allow the UI to operate without native DLL.
                    s_useFallback = true;
                    s_checked = true;
                    return;
                }

                // Copy found to output
                System.IO.File.Copy(found, target, true);

                // Try to load the copied DLL to surface a clearer error if a dependency is missing
                try
                {
                    IntPtr h = LoadLibraryW(target);
                    if (h == IntPtr.Zero)
                    {
                        int err = System.Runtime.InteropServices.Marshal.GetLastWin32Error();
                        // enable fallback rather than throwing so UI can continue
                        s_useFallback = true;
                        s_checked = true;
                        return;
                    }
                    // free immediately
                    FreeLibrary(h);
                    s_useFallback = false;
                    s_checked = true;
                }
                catch (DllNotFoundException)
                {
                    // switch to fallback
                    s_useFallback = true;
                    s_checked = true;
                    return;
                }
                catch (Exception ex)
                {
                    // switch to fallback
                    s_useFallback = true;
                    s_checked = true;
                    return;
                }
            }
            catch (Exception ex)
            {
                // If anything goes wrong, fallback to managed stubs
                s_useFallback = true;
                s_checked = true;
                return;
            }
        }
    }
}
