import _socket
import json


class RewindClient():
    def __init__(self, host=None, port=None):
        self._socket = _socket.socket()
        self._socket.setsockopt(_socket.IPPROTO_TCP, _socket.TCP_NODELAY, True)
        if host is None:
            host = "127.0.0.1"
            port = 9111
        self._socket.connect((host, port))

    def _send(self, obj):
        if self._socket:
            self._socket.sendall(json.dumps(obj).encode('utf-8'))

    def line(self, x1, y1, x2, y2, color):
        self._send({
            'type': 'line',
            'x1': x1,
            'y1': y1,
            'x2': x2,
            'y2': y2,
            'color': color
        })

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

    def popup_message(self, x, y, radius, message):
        self._send({
            'type': 'popup',
            'x': x,
            'y': y,
            'r': radius,
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
