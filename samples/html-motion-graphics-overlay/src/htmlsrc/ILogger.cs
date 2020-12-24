namespace Video.Tools
{
    using System.Diagnostics;

    internal interface ILogger
    {
        TraceLevel TraceLevel { get; set; }

        void LogDebug(string message = null);

        void LogInformation(string message = null);

        void LogWarning(string message = null);

        void LogError(string message = null);

        void WriteLine(string message = null);
    }
}
