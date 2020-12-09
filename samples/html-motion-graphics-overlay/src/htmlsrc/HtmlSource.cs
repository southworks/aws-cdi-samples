namespace Video.Tools
{
    using System;
    using System.Diagnostics;
    using System.Drawing;
    using System.IO;
    using System.Threading;
    using CefSharp;
    using CefSharp.OffScreen;
    using Video.Tools.Formats;

    internal class HtmlSource : IDisposable
    {
        public const decimal DefaultScalingFactor = 1.0M;

        private readonly ILogger log;

        private ChromiumWebBrowser browser;
        private Stream stdout;
        private bool disposed;

        public HtmlSource(ILogger log = null)
        {
            this.log = log;

            var settings = new CefSettings()
            {
                PersistSessionCookies = false,
                PersistUserPreferences = false,
                BackgroundColor = 0,
                LogSeverity = MapTraceLevel(this.log.TraceLevel),
                CefCommandLineArgs = { ["disable-gpu-shader-disk-cache"] = "1" }
            };

            Cef.Initialize(settings, performDependencyCheck: true, browserProcessHandler: null);
        }

        public int Capture(
            string sourceUrl,
            FrameFormat frameFormat,
            Size windowSize,
            Point viewportOrigin,
            Size viewportSize,
            Color chromaKeyColor,
            Color backgroundColor,
            decimal scaleFactor,
            CancellationToken cancellationToken)
        {
            var frameNumber = 0;

            this.stdout = Console.OpenStandardOutput();
            this.browser = new ChromiumWebBrowser(sourceUrl);

            if (windowSize != default)
            {
                this.browser.Size = windowSize;
            }

            var windowArea = new Rectangle(Point.Empty, this.browser.Size);

            var viewportRegion = Rectangle.Intersect(
                new Rectangle(viewportOrigin, viewportSize.IsEmpty ? this.browser.Size : viewportSize), windowArea);

            using var isCompleted = new AutoResetEvent(false);

            this.browser.FrameLoadEnd += async (object sender, FrameLoadEndEventArgs args) =>
            {
                if (args.Frame.IsMain)
                {
                    this.log?.LogDebug($"Page has loaded!");

                    var setColorScript = backgroundColor != default
                        ? $"    body.style.background = 'rgba({backgroundColor.R}, {backgroundColor.G}, {backgroundColor.B}, {Math.Round(backgroundColor.A / 255f, 2)})';"
                        : string.Empty;

                    var setScaleFactor = scaleFactor != 1.0M
                        ? $"    body.style.zoom = {scaleFactor};"
                        : string.Empty;

                    var script = $@"(function()
{{
    var body = document.body, html = document.documentElement;
{setColorScript}
    body.style.overflow = 'hidden';
{setScaleFactor}
    return [body.clientWidth, body.clientHeight];
}})();";

                    var scriptTask = await browser.EvaluateScriptAsync(script);
                    browser.Paint += (object sender, OnPaintEventArgs e) =>
                    {
                        if (cancellationToken.IsCancellationRequested)
                        {
                            isCompleted.Set();
                            return;
                        }

                        try
                        {
                            if (this.CaptureFrame(viewportRegion, frameFormat, chromaKeyColor, frameNumber/*, storeFrames*/))
                            {
                                Interlocked.Increment(ref frameNumber);
                            }

                        }
                        catch (Exception exception)
                        {
                            this.log?.LogError($"Frame capture failed: {exception.Message}");
                            isCompleted.Set();
                        }
                    };
                }
            };

            this.log?.LogDebug($"HTML Source: {this.browser.Address}");

            isCompleted.WaitOne();

            this.stdout?.Flush();
            this.stdout?.Dispose();

            Cef.Shutdown();

            return 0;
        }

        public void Dispose()
        {
            Dispose(disposing: true);
        }

        private static LogSeverity MapTraceLevel(TraceLevel traceLevel)
        {
            return traceLevel switch
            {
                TraceLevel.Off => LogSeverity.Disable,
                TraceLevel.Error => LogSeverity.Error,
                TraceLevel.Warning => LogSeverity.Warning,
                TraceLevel.Info => LogSeverity.Info,
                TraceLevel.Verbose => LogSeverity.Verbose,
                _ => LogSeverity.Default
            };
        }

        private bool CaptureFrame(
            Rectangle viewportRegion,
            FrameFormat frameFormat,
            Color chromaKeyColor,
            int frameNumber)
        {
            using var bitmap = this.browser.ScreenshotOrNull();
            if (bitmap == null)
            {
                return false;
            }

            if (this.browser.Size.Width != bitmap.Width || this.browser.Size.Height != bitmap.Height)
            {
                this.log?.LogDebug($"Skipped frame #{frameNumber:000} with different dimensions...");
                return false;
            }

            if (frameFormat != FrameFormat.Rgba && chromaKeyColor != Color.Transparent)
            {
                bitmap.MakeTransparent(chromaKeyColor);
            }

            long byteCount = 0;
            if (frameFormat != FrameFormat.Rgba)
            {
                using var memoryStream = new MemoryStream();
                using var clippedBitmap = bitmap.ClipRegion(viewportRegion);
                clippedBitmap.Save(memoryStream, frameFormat.AsImageFormat());
                memoryStream.Seek(0, SeekOrigin.Begin);
                memoryStream.CopyTo(this.stdout);
                byteCount = memoryStream.Position;
            }
            else
            {
                var rawFrame = bitmap.AsRawFrame(viewportRegion, chromaKeyColor);
                this.stdout.Write(rawFrame, 0, rawFrame.Length);
                byteCount = rawFrame.Length;
            }

            this.log?.LogDebug($"Output {frameFormat.ToString().ToUpper()} frame #{frameNumber:000} with size {byteCount}...");
            this.stdout.Flush();

            return true;
        }

        protected virtual void Dispose(bool disposing)
        {
            if (!disposed)
            {
                if (disposing)
                {
                    this.browser?.Dispose();
                }

                disposed = true;
            }
        }
    }
}
