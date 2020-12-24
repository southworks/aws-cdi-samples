namespace Video.Tools
{
    using System;
    using System.Drawing;
    using System.Drawing.Imaging;
    using System.Runtime.InteropServices;
    using Video.Tools.Formats;

    public static class ImageExtensions
    {
        public static Bitmap ClipRegion(
            this Bitmap bitmap,
            Rectangle region = default)
        {
            Bitmap clippedBitmap = bitmap;

            if (region != default)
            {
                clippedBitmap = bitmap.Clone(region, bitmap.PixelFormat);
            }

            return clippedBitmap;
        }

        public static byte[] AsRawFrame(
            this Bitmap bitmap,
            Rectangle region = default,
            Color tranparentColor = default)
        {
            BitmapData bitmapData = null;

            try
            {
                bitmapData = bitmap.LockBits(
                    region.IsEmpty ? new Rectangle(Point.Empty, bitmap.Size) : region,
                    ImageLockMode.ReadOnly,
                    bitmap.PixelFormat);

                var transparentColorValue = tranparentColor.ToArgb();
                var bitsPerPixel = Image.GetPixelFormatSize(bitmapData.PixelFormat);
                int pixelCount = bitmapData.Width * bitmapData.Height;
                int bytesPerPixel = bitsPerPixel / 8;
                int bytesPerLine = bitmapData.Width * bytesPerPixel;

                var rawData = new byte[pixelCount * bytesPerPixel];
                var rawDataPtr = bitmapData.Scan0;

                for (int y = 0; y < bitmapData.Height; y++)
                {
                    int ptrOffset = y * bitmapData.Stride;
                    int bufferOffset = y * bytesPerLine;

                    Marshal.Copy(rawDataPtr + ptrOffset, rawData, bufferOffset, bytesPerLine);

                    for (int x = 0; x < bitmapData.Width; x++, bufferOffset += bytesPerPixel)
                    {
                        if (BitConverter.ToInt32(rawData, bufferOffset) == (transparentColorValue))
                        {
                            Buffer.SetByte(rawData, bufferOffset + 3, 0);
                        }
                    }
                }

                return rawData;
            }
            finally
            {
                if (bitmapData != null)
                {
                    bitmap.UnlockBits(bitmapData);
                }
            }
        }

        public static void MakeTransparent(
            this Bitmap bitmap,
            Rectangle region = default)
        {
            BitmapData bitmapData = bitmap.LockBits(
                region.IsEmpty ? new Rectangle(Point.Empty, bitmap.Size) : region,
                ImageLockMode.ReadOnly,
                bitmap.PixelFormat);

            var bitsPerPixel = Image.GetPixelFormatSize(bitmapData.PixelFormat);
            int bytesPerPixel = bitsPerPixel / 8;
            var byteCount = bitmapData.Stride * bitmapData.Height;
            var rawData = new byte[byteCount];

            Marshal.Copy(bitmapData.Scan0, rawData, 0, byteCount);

            for (var y = 0; y < bitmap.Height; y++)
            {
                for (var x = 0; x < bitmap.Width; x++)
                {
                    var index = y * bitmapData.Stride + x * 4;
                    if (rawData[index + 2] + rawData[index + 1] + rawData[index + 0] <= 0)
                    {
                        rawData[index + 3] = 0;
                    }
                }
            }

            Marshal.Copy(rawData, 0, bitmapData.Scan0, rawData.Length);
            bitmap.UnlockBits(bitmapData);
            var ptr = bitmapData.Scan0;

            for (int y = 0; y < bitmapData.Height; y++)
            {
                int ptrOffset = y * bitmapData.Stride;
                int lineBytes = bitmapData.Width * bytesPerPixel;
                int bufferOffset = y * lineBytes;

                Marshal.Copy(ptr + ptrOffset, rawData, bufferOffset, lineBytes);
            }
        }

        public static ImageFormat AsImageFormat(this FrameFormat frameFormat)
        {
            return frameFormat switch
            {
                FrameFormat.Bmp => ImageFormat.Bmp,
                FrameFormat.Gif => ImageFormat.Gif,
                FrameFormat.Jpeg => ImageFormat.Jpeg,
                FrameFormat.Png => ImageFormat.Png,
                FrameFormat.Rgba => throw new NotSupportedException($"Unexpected '{frameFormat}' frame format specified."),
                FrameFormat.Unspecified => throw new NotSupportedException($"Unexpected '{frameFormat}' frame format specified."),
                _ => throw new NotSupportedException($"'{frameFormat}' frame format is not currently supported.")
            };
        }

        public static Color AsSystemColor(this string value)
        {
            var color = string.IsNullOrWhiteSpace(value) ? Color.Transparent : ColorTranslator.FromHtml(value);

            return color;
        }

        public static bool IsValidHtmlColor(this string value)
        {
            try
            {
                value.AsSystemColor();
            }
            catch (Exception)
            {
                return false;
            }

            return true;
        }
    }
}
