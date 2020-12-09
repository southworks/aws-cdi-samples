namespace Video.Tools
{
    using System;
    using System.CommandLine;
    using System.CommandLine.Invocation;
    using System.CommandLine.Parsing;
    using System.Diagnostics;
    using System.Drawing;
    using System.Reflection;
    using System.Threading;
    using System.Threading.Tasks;
    using Video.Tools.Formats;

    public class Program
    {
        public delegate int RunHandler(
            string sourceUrl,
            FrameFormat frameFormat,
            Size windowSize,
            Point viewportOrigin,
            Size viewportSize,
            string chromaColor,
            string backgroundColor,
            decimal scaleFactor,
            IConsole console,
            TraceLevel traceLevel,
            bool quietMode,
            CancellationToken cancellationToken);

        public static async Task<int> Main(params string[] args)
        {
#if NETCOREAPP
            var subProcessExe = new CefSharp.BrowserSubprocess.BrowserSubprocessExecutable();
            var result = subProcessExe.Main(args);
            if (result > 0)
            {
                return result;
            }
#endif
            var sourceArgument = new Argument<string>(
                "sourceUrl", 
                "URL of the page to capture");

            var windowSizeOption = new Option<Size>(
                new[] { "--window_size", "-ws" },
                "Capture window size specified as width,height without spaces! (e.g. 800,600).")
            {
                Name = "windowSize"
            };

            var viewportOriginOption = new Option<Point>(
                new[] { "--viewport_origin", "-vo" },
                "Viewport origin specified as left,top coordinate without spaces! (e.g. 10,20). Origin is (0, 0) when left unspecified.")
            {
                Name = "viewportOrigin"
            };

            var viewportSizeOption = new Option<Size>(
                new[] { "--viewport_size", "-vs" },
                "Viewport size specified as width,height without spaces! (e.g. 800,600). Captures area below and to the right of the viewport origin when left unspecified.")
            {
                Name = "viewportSize"
            };

            var scaleFactorOption = new Option<decimal>(
                new[] { "--scale_factor", "-sf" },
                () => HtmlSource.DefaultScalingFactor,
                $"Scale factor applied to HTML content.")
            {
                Name = "scaleFactor"
            };

            var frameFormatOption = new Option<FrameFormat>(
                new[] { "--format", "-f" },
                () => FrameFormat.Rgba,
                $"Image format for output frames. Default is {FrameFormat.Rgba}.")
            {
                Name = "frameFormat"
            };

            var chromaColorOption = new Option<string>(
                new[] { "--chromakey-color", "-c" },
                "Pixels with the specified HTML color value are mapped as transparent (e.g. #00FF00, green).")
            {
                Name = "chromaHtmlColor"
            };

            chromaColorOption.Argument.AddValidator(
                arg => {
                    var colorValue = arg.GetValueOrDefault<string>();
                    return !colorValue.IsValidHtmlColor() ? $"ERROR: Chroma color value '{colorValue}' has an invalid format." : null;
                });

            var backgroundColorOption = new Option<string>(
                new[] { "--background-color", "-b" },
                "Replaces the background color of the captured page (e.g. #00FF00, green).")
            {
                Name = "backgroundHtmlColor"
            };

            backgroundColorOption.Argument.AddValidator(
                arg => {
                    var colorValue = arg.GetValueOrDefault<string>();
                    return !colorValue.IsValidHtmlColor() ? $"ERROR: Background color value '{colorValue}' has an invalid format." : null;
                });

            var traceLevelOption = new Option<TraceLevel>(
                new[] { "--log", "-l" },
                () => TraceLevel.Info,
                $"Console logging level.")
            {
                Name = "traceLevel"
            };

            var quietModeOption = new Option<bool>(
                new[] { "--quiet", "-q" },
                $"Hide banner and input options.")
            {
                Name = "quietMode"
            };

            var rootCommand = new RootCommand()
            {
                sourceArgument,
                windowSizeOption,
                viewportSizeOption,
                viewportOriginOption,
                frameFormatOption,
                chromaColorOption,
                backgroundColorOption,
                scaleFactorOption,
                traceLevelOption,
                quietModeOption
            };

            rootCommand.Description =
@"HTML Video Source

- Captures output from a Web page as a sequence of images to be used as a video source.
";
            rootCommand.Handler = CommandHandler.Create((RunHandler)Run);

            result = await rootCommand.InvokeAsync(args);

            return result;
        }

        public static int Run(
            string sourceUrl,
            FrameFormat frameFormat,
            Size windowSize,
            Point viewportOrigin,
            Size viewportSize,
            string chromaHtmlColor,
            string backgroundHtmlColor,
            decimal scaleFactor,
            IConsole console,
            TraceLevel traceLevel,
            bool quietMode,
            CancellationToken cancellationToken)
        {
            var profiler = Stopwatch.StartNew();
            var log = new Logger(console, traceLevel) as ILogger;

            try
            {
                var version = (Assembly.GetEntryAssembly() ?? Assembly.GetExecutingAssembly())
                    .GetCustomAttribute<AssemblyInformationalVersionAttribute>()
                    .InformationalVersion;

                var chromaKeyColor = chromaHtmlColor.AsSystemColor();
                var backgroundColor = backgroundHtmlColor.AsSystemColor();
                if (!quietMode)
                {
                    log?.WriteLine($"HTML Image Source - version {version}");
                    log?.WriteLine();
                    log?.WriteLine("Options:");
                    log?.WriteLine($"  Source                    : {sourceUrl}");
                    log?.WriteLine($"  Windows size              : {(windowSize.IsEmpty ? "Unspecified" : windowSize.ToString().Trim('{', '}'))}");
                    log?.WriteLine($"  Viewport origin           : {viewportOrigin}");
                    log?.WriteLine($"  Viewport size             : {viewportSize}");
                    log?.WriteLine($"  Frame format              : {frameFormat.ToString().ToUpper()}");
                    log?.WriteLine($"  Chroma key                : {chromaKeyColor}");
                    log?.WriteLine($"  Background color          : {backgroundColor}");
                    log?.WriteLine($"  Scale factor              : {scaleFactor}");
                    log?.WriteLine();
                }

                using var source = new HtmlSource(log);
                var result = source.Capture(
                    sourceUrl,
                    frameFormat,
                    windowSize,
                    viewportOrigin,
                    viewportSize,
                    chromaKeyColor,
                    backgroundColor,
                    scaleFactor,
                    cancellationToken);

                return result;
            }
            catch (OperationCanceledException)
            {
                log?.LogError("The operation was cancelled.");
                return 1;
            }
            catch (Exception exception)
            {
                log?.LogError(exception.ToString());
                return 2;
            }
            finally
            {
                log?.WriteLine($"Execution complete: elapsed time {profiler.ElapsedMilliseconds}ms.");
            }
        }
    }
}
