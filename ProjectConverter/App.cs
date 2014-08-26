using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Windows.Forms;
using System.Diagnostics;
using System.Threading;
using System.IO;

namespace ProjectConverter {
    public partial class App : Form {
        const int NO_PROCESS = 0;
        Process[] vsProcesses;
        Process vsProcess;
        Thread tProcessTermination, tProcessPoll;
        Dictionary<int, List<FileInfo>> trackedFiles;

        public App() {
            InitializeComponent();
            trackedFiles = new Dictionary<int, List<FileInfo>>();
            trackedFiles[NO_PROCESS] = new List<FileInfo>();
            log.AppendText("Please Select A List Of Files To Track\n");
            log.AppendText("Select A Process To Track, If Necessary\n");
        }

        void RebuildProcessList() {
            // Find Processes
            var processes = Process.GetProcesses();
            vsProcesses = processes.Where((p) => {
                return p.ProcessName.Equals("devenv");
            }).ToArray();

            // Add Processes
            comboBox1.Items.Clear();
            comboBox1.Items.Add("None");
            if(vsProcesses.Length > 0) {
                foreach(var p in vsProcesses) {
                    comboBox1.Items.Add(p.MainWindowTitle);
                }
            }
        }
        List<FileInfo> GetTrackMap() {
            List<FileInfo> l;
            int id = vsProcess == null ? NO_PROCESS : vsProcess.Id;
            if(!trackedFiles.TryGetValue(id, out l)) {
                l = new List<FileInfo>();
                trackedFiles[id] = l;
            }
            return l;
        }

        void VSProcessWaiter() {
            vsProcess.WaitForExit();
            if(checkBox1.Checked) {
                Process.GetCurrentProcess().Kill();
            }
            else {
                vsProcess = null;
                Invoke((MethodInvoker)delegate { 
                    RebuildProcessList();
                    comboBox1.SelectedIndex = 0;
                });
            }
        }
        void VSProcessPoller() {
            while(true) {
                var remoteFiles = GetTrackMap();
                if(remoteFiles.Count != 0) {
                    var files = new FileInfo[remoteFiles.Count];
                    remoteFiles.CopyTo(files);
                    foreach(var fi in remoteFiles) {
                        Convert(fi);
                    }
                }
                Thread.Sleep(2000);
            }
        }
        void CreateProcessThreads() {
            if(vsProcess == null) return;
            tProcessTermination = new Thread(VSProcessWaiter);
            tProcessTermination.Priority = ThreadPriority.BelowNormal;
            tProcessTermination.Start();
            tProcessPoll = new Thread(VSProcessPoller);
            tProcessPoll.Priority = ThreadPriority.BelowNormal;
            tProcessPoll.Start();
        }
        void TerminateProcessThreads() {
            if(tProcessTermination != null && tProcessTermination.ThreadState == System.Threading.ThreadState.Running) {
                tProcessTermination.Abort();
                tProcessTermination = null;
            }
            if(tProcessPoll != null && tProcessPoll.ThreadState == System.Threading.ThreadState.Running) {
                tProcessPoll.Abort();
                tProcessPoll = null;
            }
        }
        void Convert(FileInfo fi) {
            string data;
            try {
                using(var s = fi.OpenRead()) {
                    data = new StreamReader(s).ReadToEnd();
                }
                Regex rgx = new Regex(@"<(?<Outer>\w+) Include=(?<Name>[^>]+)>\s+<(?<Inner>\w+)>(?<Data>[^<]+)</\k<Inner>>\s+</\k<Outer>>");
                if(rgx.Match(data).Success) {
                    data = rgx.Replace(data,
                        @"<${Outer} Include=${Name}><${Inner}>${Data}</${Inner}></${Outer}>"
                        );
                    using(var s = fi.Create()) {
                        var w = new StreamWriter(s);
                        w.Write(data);
                        w.Flush();
                    }
                    Invoke((MethodInvoker)delegate {
                        log.AppendText("Modified: " + fi.FullName + "\n");
                    });
                }
            }
            catch(Exception e) {
                Invoke((MethodInvoker)delegate {
                    log.AppendText("Error: " + fi.FullName + "\n");
                    log.AppendText(e.Message + "\n");
                });
            }
            return;

        }

        // Process Selection
        private void comboBox1_SelectedIndexChanged(object sender, EventArgs e) {
            TerminateProcessThreads();
            files.Items.Clear();
            files.Items.AddRange((from fi in GetTrackMap() select fi.FullName).ToArray());

            if(comboBox1.SelectedIndex == 0) {
                vsProcess = null;
            }
            else {
                vsProcess = vsProcesses[comboBox1.SelectedIndex - 1];
                CreateProcessThreads();
            }
        }
        // Process Refresh
        void comboBox1_DropDown(object sender, System.EventArgs e) {
            RebuildProcessList();
        }
        // Auto-Shutdown Detection
        private void checkBox1_CheckedChanged(object sender, EventArgs e) {
            if(checkBox1.Checked) {
                if(tProcessTermination == null && vsProcess != null) {
                    CreateProcessThreads();
                }
            }
            else {
                TerminateProcessThreads();
            }
        }
        // Add Tracked File
        private void button1_Click(object sender, EventArgs e) {
            var l = GetTrackMap();
            using(var fd = new OpenFileDialog()) {
                fd.ShowDialog();
                foreach(var f in fd.FileNames) {
                    FileInfo fi = new FileInfo(f);
                    if(!l.Contains(fi, new FileNameComparer())) {
                        l.Add(fi);
                    }
                }
            }
            files.Items.Clear();
            files.Items.AddRange((from fi in l select fi.FullName).ToArray());
        }
        // Removed Tracked File
        private void button2_Click(object sender, EventArgs e) {
            var l = GetTrackMap();
            foreach(var file in files.SelectedItems) {
                FileInfo fi = new FileInfo(file.ToString());
                l.RemoveAll((f) => { return f.FullName.Equals(fi.FullName); });
            }
            files.Items.Clear();
            files.Items.AddRange((from fi in l select fi.FullName).ToArray());
        }
        class FileNameComparer : IEqualityComparer<FileInfo> {
            public bool Equals(FileInfo x, FileInfo y) {
                return x.FullName.Equals(y.FullName);
            }
            public int GetHashCode(FileInfo obj) {
                return obj.FullName.GetHashCode();
            }
        }

        [STAThread]
        static void Main(string[] args) {
            using(App app = new App()) {
                app.ShowDialog();
            }
        }
    }
}
