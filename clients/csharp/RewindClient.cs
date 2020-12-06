using System;
using System.Collections.Generic;
using System.Drawing;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Net.Sockets;
using System.Text;
using System.Threading;

namespace RewindViewer
{
    /// <inheritdoc />
    /// <summary>
    ///     A simple implementation of rewind client in C#.
    /// </summary>
    public class RewindClient : IDisposable
    {
        private static class Primitives
        {
            public const string Circle = "circle";
            public const string Rectangle = "rectangle";
            public const string Triangle = "triangle";
            public const string Polyline = "polyline";
            public const string Message = "message";
            public const string Popup = "popup";
            public const string Options = "options";
            public const string End = "end";
        }

        private const string DefaultHost = "127.0.0.1";
        private const int DefaultPort = 9111;

        private const int LayerMin = 1;
        private const int LayerMax = 10;

        private const int BufferSizeBytes = 1 << 20;

        private readonly TcpClient _client;
        private readonly BinaryWriter _writer;

        private static RewindClient _instance;
        private static readonly object _locker = new object();

        public RewindClient() : this(DefaultHost, DefaultPort)
        {
        }

        public RewindClient(string host, int port)
        {
            if (string.IsNullOrWhiteSpace(host))
            {
                throw new ArgumentException(nameof(host));
            }
            if (port < 1)
            {
                throw new ArgumentException(nameof(port));
            }

            _client = new TcpClient(host, port)
            {
                SendBufferSize = BufferSizeBytes,
                ReceiveBufferSize = BufferSizeBytes,
                NoDelay = true
            };

            _writer = new BinaryWriter(_client.GetStream());
            _client = new TcpClient(host, port);

            CultureInfo newCInfo = (CultureInfo)Thread.CurrentThread.CurrentCulture.Clone();
            newCInfo.NumberFormat.NumberDecimalSeparator = ".";
            Thread.CurrentThread.CurrentCulture = newCInfo;
        }

        public static RewindClient Instance
        {
            get
            {
                if (_instance == null)
                {
                    lock (_locker)
                    {
                        if (_instance == null)
                        {
                            _instance = new RewindClient();
                        }
                    }
                }
                return _instance;
            }
        }

        public void Dispose()
        {
            _client?.Dispose();
            _writer?.Dispose();
            _instance?.Dispose();
        }

        #region Circle

        public void Circle(Point center, double r, Color color, bool fill)
        {
            Circle(center.X, center.Y, r, color, fill);
        }

        public void Circle(PointF center, double r, Color color, bool fill)
        {
            Circle(center.X, center.Y, r, color, fill);
        }

        public void Circle(double x, double y, double r, Color color, bool fill)
        {
            string command =
                "{" +
                    $"\"type\": \"{Primitives.Circle}\"," +
                    $"\"p\": [{x}, {y}]," +
                    $"\"r\": {r}," +
                    $"\"color\": {color.ToArgb()}," +
                    $"\"fill\": {BoolToJString(fill)}" +
                "}";
            SendCommand(command);
        }

        #endregion Circle

        #region Rectangle

        public void Rectangle(Point tl, Point br, Color color)
        {
            Rectangle(tl.X, tl.Y, br.X, br.Y, color);
        }

        public void Rectangle(Point tl, Point br, IList<Color> colors)
        {
            Rectangle(tl.X, tl.Y, br.X, br.Y, colors);
        }

        public void Rectangle(PointF tl, PointF br, Color color)
        {
            Rectangle(tl.X, tl.Y, br.X, br.Y, color);
        }

        public void Rectangle(PointF tl, PointF br, IList<Color> colors)
        {
            Rectangle(tl.X, tl.Y, br.X, br.Y, colors);
        }

        public void Rectangle(double x1, double y1, double x2, double y2, Color color)
        {
            Rectangle(x1, y1, x2, y2, new List<Color> { color });
        }

        public void Rectangle(double x1, double y1, double x2, double y2, IList<Color> colors)
        {
            string command =
                "{" +
                    $"\"type\": \"{Primitives.Rectangle}\"," +
                    $"\"tl\": [{x1}, {y1}]," +
                    $"\"br\": [{x2}, {y2}]," +
                    $"\"color\": {ColorToJString(colors)}" +
                "}";
            SendCommand(command);
        }

        #endregion Rectangle

        #region Triangle

        public void Triangle(IList<Point> points, Color color, bool fill)
        {
            Triangle(points.Select(p => new PointF(p.X, p.Y)).ToList(), new List<Color> { color }, fill);
        }

        public void Triangle(IList<Point> points, IList<Color> colors, bool fill)
        {
            Triangle(points.Select(p => new PointF(p.X, p.Y)).ToList(), colors, fill);
        }

        public void Triangle(IList<PointF> points, Color color, bool fill)
        {
            Triangle(points, new List<Color> { color }, fill);
        }

        public void Triangle(IList<PointF> points, IList<Color> colors, bool fill)
        {
            string command =
                "{" +
                    $"\"type\": \"{Primitives.Triangle}\"," +
                    $"\"points\": {PointFToJString(points)}," +
                    $"\"color\": {ColorToJString(colors)}," +
                    $"\"fill\": {BoolToJString(fill)}" +
                "}";
            SendCommand(command);
        }

        #endregion Triangle

        #region Polyline

        public void Line(Point p1, Point p2, Color color)
        {
            Line(p1.X, p1.Y, p2.X, p2.Y, color);
        }

        public void Line(PointF p1, PointF p2, Color color)
        {
            Line(p1.X, p1.Y, p2.X, p2.Y, color);
        }

        public void Line(double x1, double y1, double x2, double y2, Color color)
        {
            string command =
                "{" +
                    $"\"type\": \"{Primitives.Polyline}\"," +
                    $"\"points\": [{x1}, {y1}, {x2}, {y2}]," +
                    $"\"color\": {color.ToArgb()}" +
                "}";
            SendCommand(command);
        }

        public void Polyline(IList<PointF> points, Color color)
        {
            string command =
                "{" +
                    $"\"type\": \"{Primitives.Polyline}\"," +
                    $"\"points\": {PointFToJString(points)}," +
                    $"\"color\": {color.ToArgb()}" +
                "}";
            SendCommand(command);
        }

        #endregion Polyline

        #region Message

        public void Message(string message)
        {
            string command =
                "{" +
                    $"\"type\": \"{Primitives.Message}\"," +
                    $"\"message\": \"{message}\"" +
                "}";
            SendCommand(command);
        }

        #endregion Message

        #region Popup

        public void Popup(Point center, double r, string text)
        {
            Popup(center.X, center.Y, r, text);
        }

        public void Popup(PointF center, double r, string text)
        {
            Popup(center.X, center.Y, r, text);
        }

        public void Popup(double x, double y, double r, string text)
        {
            string command =
                "{" +
                    $"\"type\": \"{Primitives.Popup}\"," +
                    $"\"p\": [{x}, {y}]," +
                    $"\"r\": {r}," +
                    $"\"text\": \"{text}\"" +
                "}";
            SendCommand(command);
        }

        public void RectangularPopup(Point tl, Point br, string text)
        {
            RectangularPopup(tl.X, tl.Y, br.X, br.Y, text);
        }

        public void RectangularPopup(PointF tl, PointF br, string text)
        {
            RectangularPopup(tl.X, tl.Y, br.X, br.Y, text);
        }

        public void RectangularPopup(double tlX, double tlY, double brX, double brY, string text)
        {
            string command =
                "{" +
                    $"\"type\": \"{Primitives.Popup}\"," +
                    $"\"tl\": [{tlX}, {tlY}]," +
                    $"\"br\": [{brX}, {brY}]," +
                    $"\"text\": \"{text}\"" +
                "}";
            SendCommand(command);
        }

        #endregion Popup

        #region Options

        public void Options(int layer, bool pemanent)
        {
            if ((layer < LayerMin) || (layer > LayerMax))
            {
                throw new ArgumentOutOfRangeException(nameof(layer));
            }

            string command =
                "{" +
                    $"\"type\": \"{Primitives.Options}\"," +
                    $"\"layer\": {layer}," +
                    $"\"permanent\": {BoolToJString(pemanent)}" +
                "}";
            SendCommand(command);
        }

        #endregion Options

        #region End

        public void End()
        {
            SendCommand($"{{\"type\":\"{Primitives.End}\"}}");
        }

        #endregion End

        #region Helpers

        private string ColorToJString(IList<Color> color)
        {
            if (color == null)
            {
                throw new ArgumentNullException(nameof(color));
            }

            if (color.Count == 1)
            {
                return color[0].ToArgb().ToString();
            }
            else
            {
                return $"[{string.Join(", ", color.Select(x => x.ToArgb()))}]";
            }
        }

        private string PointFToJString(IList<PointF> points)
        {
            if (points == null)
            {
                throw new ArgumentNullException(nameof(points));
            }

            return $"[{string.Join(", ", points.Select(p => $"{p.X}, {p.Y}"))}]";
        }

        private string BoolToJString(bool b)
        {
            return b ? "true" : "false";
        }

        private void SendCommand(string command)
        {
            WriteString(command);
            _writer.Flush();
        }

        private void WriteInt(int value)
        {
            _writer.Write(value);
        }

        private void WriteString(string value)
        {
            if (value == null)
            {
                WriteInt(-1);
                return;
            }

            var bytes = Encoding.UTF8.GetBytes(value);
            _writer.Write(bytes, 0, bytes.Length);
        }

        #endregion Helpers
    }
}