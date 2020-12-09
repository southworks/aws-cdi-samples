namespace Video.Tools.Formats
{
    // NOTE: when enabling new formats keep AsImageFormat extension method in sync
    public enum FrameFormat
    {
        Unspecified,
        // Raw (RGBA) image format.
        Rgba,
        // Bitmap (BMP) image format.
        Bmp,
        // Graphics Interchange Format (GIF) image format.
        Gif,
        // Joint Photographic Experts Group (JPEG) image format.
        Jpeg,
        // W3C Portable Network Graphics (PNG) image format.
        Png,
    }
}
