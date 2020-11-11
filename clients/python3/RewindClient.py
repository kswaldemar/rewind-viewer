import _socket
import json

class RewindClient():
    RED = 0xff0000
    GREEN = 0x00ff00
    BLUE = 0x0000ff
    DARK_RED = 0x770000
    DARK_GREEN = 0x007700
    DARK_BLUE = 0x000077
    TRANSPARENT = 0x7f000000
    INVISIBLE = 0x01000000

    def __init__(self, host=None, port=None):
        self._socket = _socket.socket()
        self._socket.setsockopt(_socket.IPPROTO_TCP, _socket.TCP_NODELAY, True)
        if host is None:
            host = "127.0.0.1"
            port = 9111
        self._socket.connect((host, port))

    @staticmethod
    def _to_geojson(points):
        flat = []
        for p in points:
            flat.append(p[0])
            flat.append(p[1])
        return flat

    def _send(self, obj):
        if self._socket:
            self._socket.sendall(json.dumps(obj).encode('utf-8'))

    def line(self, x1, y1, x2, y2, color):
        self._send({
            'type': 'polyline',
            'points': [x1, y1, x2, y2],
            'color': color
        })

    def polyline(self, points, color):
        self._send({
            'type': 'polyline',
            'points': RewindClient._to_geojson(points),
            'color': color
        })

    def circle(self, x, y, radius, color, fill=False):
        self._send({
            'type': 'circle',
            'p': [x, y],
            'r': radius,
            'color': color,
            'fill': fill
        })

    def rectangle(self, x1, y1, x2, y2, color, fill=False):
        self._send({
            'type': 'rectangle',
            'tl': [x1, y1],
            'br': [x2, y2],
            'color': color,
            'fill': fill
        })

    def triangle(self, p1, p2, p3, color, fill=False):
        self._send({
            'type': 'triangle',
            'points': RewindClient._to_geojson([p1, p2, p3]),
            'color': color,
            'fill': fill
        })

    def circle_popup(self, x, y, radius, message):
        self._send({
            'type': 'popup',
            'p': [x, y],
            'r': radius,
            'text': message
        })

    def rect_popup(self, tl, br, message):
        self._send({
            'type': 'popup',
            'tl': RewindClient._to_geojson([tl]),
            'br': RewindClient._to_geojson([br]),
            'text': message
        })

    def message(self, msg):
        self._send({
            'type': 'message',
            'message': msg
        })

    def set_options(self, layer=None, permanent=None):
        data = {'type': 'options'}
        if layer is not None:
            data['layer'] = layer
        if permanent is not None:
            data['permanent'] = permanent
        self._send(data)

    def end_frame(self):
        self._send({'type': 'end'})
