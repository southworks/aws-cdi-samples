namespace Video.Tools
{
    using System.CommandLine;
    using System.CommandLine.IO;
    using System.Diagnostics;

    internal class Logger : ILogger
    {
        private readonly IConsole console;

        public Logger(
            IConsole console,
            TraceLevel traceLevel)
        {
            this.console = console;
            this.TraceLevel = traceLevel;
        }

        public TraceLevel TraceLevel { get; set; }

        public void LogDebug(string message)
        {
            if (this.TraceLevel >= TraceLevel.Verbose)
            {
                this.WriteLineCore("DEBUG: " + message);
            }
        }

        public void LogInformation(string message)
        {
            if (this.TraceLevel >= TraceLevel.Info)
            {
                this.WriteLineCore("INFO: " + message);
            }
        }

        public void LogWarning(string message)
        {
            if (this.TraceLevel >= TraceLevel.Warning)
            {
                this.WriteLineCore("WARN: " + message);
            }
        }

        public void LogError(string message)
        {
            if (this.TraceLevel >= TraceLevel.Error)
            {
                this.WriteLineCore("ERROR: " + message);
            }
        }

        public void WriteLine(string message)
        {
            if (this.TraceLevel >= TraceLevel.Info)
            {
                this.WriteLineCore(message);
            }
        }

        private void WriteLineCore(string message)
        {
            (this.console.IsOutputRedirected ?
                this.console.Error : this.console.Out).WriteLine(message);
        }
    }
}
