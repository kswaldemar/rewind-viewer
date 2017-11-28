using System;
using System.Drawing;
using System.Globalization;
using System.IO;
using System.Net.Sockets;
using System.Text;
using System.Threading;

namespace Com.CodeGame.CodeWars2017.DevKit.CSharpCgdk
{
    /// <inheritdoc />
    /// <summary>
    ///     A simple implementation of rewind client in C#.
    /// </summary>
    /// <remarks>
    ///     Note: You have to reference System.Drawing in order to use the client.
    ///     Usage:
    ///     RewindClient.Instance.AreaDescription(2, 4, AreaType.Cloud);
    ///     RewindClient.Instance.End();
    /// </remarks>
    /// <seealso cref="T:System.IDisposable" />
    // ReSharper disable once UnusedMember.Global
    // ReSharper disable once StyleCop.SA1201
    public class RewindClient : IDisposable
    {
        private const int BufferSizeBytes = 1 << 20;

        private const int DefaultLayer = 3;

        private static RewindClient _instance;

        private readonly TcpClient _client;

        private readonly BinaryWriter _writer;

        private RewindClient(string host, int port)
        {
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

        public static RewindClient Instance => _instance ?? (_instance = new RewindClient("127.0.0.1", 9111));

        public void Dispose()
        {
            _client?.Dispose();
            _writer?.Dispose();
            _instance?.Dispose();
        }

        public void End()
        {
            SendCommand("{\"type\":\"end\"}");
        }

        public void Circle(double x, double y, double r, Color color, int layer = DefaultLayer)
        {
            SendCommand($"{{\"type\": \"circle\", \"x\": {x}, \"y\": {y}, \"r\": {r}, \"color\": {color.ToArgb()}, \"layer\": {layer}}}");
        }

        public void Rectangle(double x1, double y1, double x2, double y2, Color color, int layer = DefaultLayer)
        {
            SendCommand($"{{\"type\": \"rectangle\", \"x1\": {x1}, \"y1\": {y1}, \"x2\": {x2}, \"y2\": {y2}, \"color\": {color.ToArgb()}, \"layer\": {layer}}}");
        }

        public void Line(double x1, double y1, double x2, double y2, Color color, int layer = DefaultLayer)
        {
            SendCommand($"{{\"type\": \"line\", \"x1\": {x1}, \"y1\": {y1}, \"x2\": {x2}, \"y2\": {y2}, \"color\": {color.ToArgb()}, \"layer\": {layer}}}");
        }

        public void LivingUnit(
            double x,
            double y,
            double r,
            int hp,
            int maxHp,
            Side side,
            double course = 0,
            UnitType unitType = UnitType.Unknown,
            int remCooldown = 0,
            int maxCooldown = 0,
            bool selected = false)
        {
            SendCommand($"{{\"type\": \"unit\", \"x\": {x}, \"y\": {y}, \"r\": {r}, \"hp\": {hp}, \"max_hp\": {maxHp}, \"enemy\": {(int)side}, \"unit_type\": {(int)unitType}, \"course\": {course},"
                        + $"\"rem_cooldown\":{remCooldown}, \"cooldown\":{maxCooldown}, \"selected\":{(selected ? 1 : 0)} }}");
        }

        public void AreaDescription(int cellX, int cellY, AreaType areaType)
        {
            SendCommand($"{{\"type\": \"area\", \"x\": {cellX}, \"y\": {cellY}, \"area_type\": {(int)areaType}}}");
        }

        public void Message(string message)
        {
            SendCommand($"{{\"type\": \"message\", \"message\": \"{message}\"}}");
        }

        public void Popup(double x, double y, double r, string text)
        {
            SendCommand($"{{\"type\": \"popup\", \"x\": {x}, \"y\": {y}, \"r\": {r}, \"text\": \"{text}\"}}");
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
    }

    public enum Side
    {
        Our = -1,
        Neutral = 0,
        Enemy = 1
    }

    public enum UnitType
    {
        Unknown = 0,
        Tank = 1,
        Ifv = 2,
        Arrv = 3,
        Helicopter = 4,
        Fighter = 5,
    }

    public enum AreaType
    {
        Unknown = 0,
        Forest = 1,
        Swamp = 2,
        Rain = 3,
        Cloud = 4
    }
}
