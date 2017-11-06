import _socket
import json

class RewindClient():
    
    def __init__(self, host=None, port=None):
        self.socket = _socket.socket()
        self.socket.setsockopt(_socket.IPPROTO_TCP, _socket.TCP_NODELAY, True)
        if host is None:
            host = "127.0.0.1"
            port = 7000
        self.socket.connect((host, port))
        
    def close(self):
        self.socket.close()
        
    def _send(self, obj):
        if self.socket:
            self.socket.sendall(json.dumps(obj).encode('utf-8'))
            
    def circle(self, x, y, radius, color):
        self._send({
            'type': 'circle',
            'x': x,
            'y': y,
            'r': radius,
            'color': color
        })
        
    def rectangle(self, x1, y1, x2, y2, color):
        self._send({
            'type': 'rectangle',
            'x1': x1,
            'y1': y1,
            'x2': x2,
            'y2': y2,
            'color': color
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

    def living_unit(self, x, y, radius, hp, max_hp, enemy, course, unit_type):
        self._send({
            'type': 'unit',
            'x': x,
            'y': y,
            'r': radius,
            'hp': hp,
            'max_hp': max_hp,
            'enemy': enemy,
            'course': course,
            'unit_type': unit_type
        })

    def end_frame(self):
        self._send({'type': 'end'})