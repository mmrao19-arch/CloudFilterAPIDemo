using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Collections.ObjectModel;
// Use local NativeInterop to P/Invoke native exports
using static CFWPFUI.NativeInterop;
using System.ComponentModel;
using System.Runtime.CompilerServices;

namespace CFWPFUI
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        ObservableCollection<FileItem> items = new ObservableCollection<FileItem>();

        public MainWindow()
        {
            InitializeComponent();
            FilesList.ItemsSource = items;
            SyncRootPathBox.Text = System.IO.Path.Combine(System.Environment.GetFolderPath(System.Environment.SpecialFolder.UserProfile), "CloudSyncRoot");
        }

        private void BtnRegister_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                // Use P/Invoke interop to call native exports
                RegisterSyncRootSafe(SyncRootPathBox.Text, "com.example.sync", "My Sync Root");
                StatusBlock.Text = "Sync root registered.";
            }
            catch (System.Exception ex)
            {
                StatusBlock.Text = "Register failed: " + ex.Message;
            }
        }

        private void BtnRefresh_Click(object sender, RoutedEventArgs e)
        {
            // For prototype: list files under sync root directory
            items.Clear();
            try
            {
                var dir = SyncRootPathBox.Text;
                if (!System.IO.Directory.Exists(dir)) { StatusBlock.Text = "Sync root does not exist."; return; }
                foreach (var f in System.IO.Directory.GetFiles(dir, "*", System.IO.SearchOption.TopDirectoryOnly))
                {
                    var fi = new System.IO.FileInfo(f);
                    items.Add(new FileItem { Name = fi.Name, Path = fi.FullName, Size = fi.Exists ? fi.Length : -1, Status = "Unknown" });
                }
                StatusBlock.Text = "Refreshed.";
            }
            catch (System.Exception ex)
            {
                StatusBlock.Text = "Refresh failed: " + ex.Message;
            }
        }

        private void FilesList_DragOver(object sender, DragEventArgs e)
        {
            if (e.Data.GetDataPresent(DataFormats.FileDrop)) e.Effects = DragDropEffects.Copy;
            else e.Effects = DragDropEffects.None;
            e.Handled = true;
        }

        private void FilesList_Drop(object sender, DragEventArgs e)
        {
            if (!e.Data.GetDataPresent(DataFormats.FileDrop)) return;
            var files = (string[])e.Data.GetData(DataFormats.FileDrop);
            // For prototype: copy files into sync root directory and add placeholder
            try
            {
                var dir = SyncRootPathBox.Text;
                if (!System.IO.Directory.Exists(dir)) System.IO.Directory.CreateDirectory(dir);
                foreach (var f in files)
                {
                    var dest = System.IO.Path.Combine(dir, System.IO.Path.GetFileName(f));
                    System.IO.File.Copy(f, dest, true);
                    items.Add(new FileItem { Name = System.IO.Path.GetFileName(dest), Path = dest, Size = new System.IO.FileInfo(dest).Length, Status = "Uploaded" });
                }
                StatusBlock.Text = "Files uploaded.";
            }
            catch (System.Exception ex)
            {
                StatusBlock.Text = "Drop failed: " + ex.Message;
            }
        }

        private void BtnCreatePlaceholder_Click(object sender, RoutedEventArgs e)
        {
            // Create placeholder for selected item
            if (FilesList.SelectedItem is FileItem fi)
            {
                try
                {
                    int hr = CreatePlaceholderFileSafe(fi.Path, (ulong)fi.Size);
                    if (hr != 0) throw new System.ComponentModel.Win32Exception(hr);
                    fi.Status = "Placeholder";
                    StatusBlock.Text = "Placeholder created.";
                }
                catch (System.Exception ex)
                {
                    StatusBlock.Text = "Create placeholder failed: " + ex.Message;
                }
            }
        }

        private void BtnHydrate_Click(object sender, RoutedEventArgs e)
        {
            if (FilesList.SelectedItem is FileItem fi)
            {
                try
                {
                    int hr2 = TriggerHydrationSafe(fi.Path);
                    if (hr2 != 0) throw new System.ComponentModel.Win32Exception(hr2);
                    fi.Status = "Downloaded";
                    StatusBlock.Text = "Hydration triggered.";
                }
                catch (System.Exception ex)
                {
                    StatusBlock.Text = "Hydration failed: " + ex.Message;
                }
            }
        }

        private void BtnNotify_Click(object sender, RoutedEventArgs e)
        {
            if (FilesList.SelectedItem is FileItem fi)
            {
                try
                {
                    int hr3 = NotifyFileStateChangeSafe(fi.Path, 1);
                    if (hr3 != 0) throw new System.ComponentModel.Win32Exception(hr3);
                    fi.Status = "Synced";
                    StatusBlock.Text = "Notified.";
                }
                catch (System.Exception ex)
                {
                    StatusBlock.Text = "Notify failed: " + ex.Message;
                }
            }
        }

        class FileItem : INotifyPropertyChanged
        {
            public string Name { get; set; }
            public string Path { get; set; }
            public long Size { get; set; }
            public string SizeDisplay => Size >= 0 ? Size.ToString() : "-";
            string _status = "Unknown";
            public string Status { get => _status; set { _status = value; OnPropertyChanged(); } }

            public event PropertyChangedEventHandler? PropertyChanged;
            void OnPropertyChanged([CallerMemberName] string? n = null) => PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(n));
        }
    }
}