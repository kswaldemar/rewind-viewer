import _socket
import json


class RewindClient():
    def __init__(self, host=None, port=None):
        self.socket = _socket.socket()
        self.socket.setsockopt(_socket.IPPROTO_TCP, _socket.TCP_NODELAY, True)
        if host is None:
            host = "127.0.0.1"
            port = 9111
        self.socket.connect((host, port))

    def close(self):
        self.socket.close()

    def _send(self, obj):
        if self.socket:
            self.socket.sendall(json.dumps(obj).encode('utf-8'))

    def circle(self, x, y, radius, color, fill=False):
        self._send({
            'type': 'circle',
            'x': x,
            'y': y,
            'r': radius,
            'color': color,
            'fill': fill
        })

    def rectangle(self, x1, y1, x2, y2, color, fill=False):
        self._send({
            'type': 'rectangle',
            'x1': x1,
            'y1': y1,
            'x2': x2,
            'y2': y2,
            'color': color,
            'fill': fill
        })

    def line(self, x1, y1, x2, y2, color):
        self._send({
            'type': 'line',
            'x1': x1,
            'y1': y1,
            'x2': x2,
            'y2': y2,
            'color': color
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

    def set_permanent(self, value):
        self._send({
            'type': 'options',
            'permanent': value
        })

    def end_frame(self):
        self._send({'type': 'end'})
