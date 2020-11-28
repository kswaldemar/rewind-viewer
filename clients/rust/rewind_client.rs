use std::io;
use std::io::Write;
use std::net::TcpStream;

use serde::Serialize;

// usage:
// let mut rewind_client = RewindClient::new().unwrap();
// rewind_client.line(0.0, 0.0, 100.0, 100.0, 0xff0000).unwrap(); // color=0xrrggbb or 0xaarrggbb
// rewind_client.end_frame().unwrap();
//
// [dependencies]
// serde = { version = "1.0.117", features = ["derive"] }
// serde_json = "1.0.59"

#[allow(dead_code)]
pub struct RewindClient {
    stream: TcpStream,
}

#[allow(dead_code)]
impl RewindClient {
    pub const RED: u32 = 0xff0000;
    pub const GREEN: u32 = 0x00ff00;
    pub const BLUE: u32 = 0x0000ff;
    pub const DARK_RED: u32 = 0x770000;
    pub const DARK_GREEN: u32 = 0x007700;
    pub const DARK_BLUE: u32 = 0x000077;
    pub const TRANSPARENT: u32 = 0x7f000000;
    pub const INVISIBLE: u32 = 0x01000000;

    pub fn new() -> io::Result<Self> {
        Self::with_host_port("127.0.0.1", 9111)
    }

    pub fn with_host_port(host: &str, port: u16) -> io::Result<Self> {
        let stream = TcpStream::connect((host, port))?;
        stream.set_nodelay(true)?;
        Ok(RewindClient { stream })
    }

    fn send<T: Serialize>(&mut self, value: &T) -> io::Result<()> {
        self.stream.write_all(serde_json::to_string(value)?.as_bytes())
    }

    pub fn line(&mut self, x1: f64, y1: f64, x2: f64, y2: f64, color: u32) -> io::Result<()> {
        self.send(&Request::Polyline {
            points: vec![x1, y1, x2, y2],
            color,
        })
    }

    pub fn polyline(&mut self, points: &[f64], color: u32) -> io::Result<()> {
        self.send(&Request::Polyline {
            points: points.to_vec(),
            color,
        })
    }

    pub fn circle(&mut self, x: f64, y: f64, radius: f64, color: u32, fill: bool) -> io::Result<()> {
        self.send(&Request::Circle {
            p: [x, y],
            r: radius,
            color,
            fill,
        })
    }

    pub fn rectangle(&mut self, x1: f64, y1: f64, x2: f64, y2: f64, color: u32, fill: bool) -> io::Result<()> {
        self.send(&Request::Rectangle {
            tl: [x1, y1],
            br: [x2, y2],
            color,
            fill,
        })
    }

    pub fn rectangle_with_colors(&mut self, x1: f64, y1: f64, x2: f64, y2: f64, colors: [u32; 4], fill: bool) -> io::Result<()> {
        self.send(&Request::RectangleWithColors {
            tl: [x1, y1],
            br: [x2, y2],
            color: colors,
            fill,
        })
    }

    pub fn triangle(&mut self, x1: f64, y1: f64, x2: f64, y2: f64, x3: f64, y3: f64, color: u32, fill: bool) -> io::Result<()> {
        self.send(&Request::Triangle {
            points: [x1, y1, x2, y2, x3, y3],
            color,
            fill,
        })
    }

    pub fn triangle_with_colors(&mut self, x1: f64, y1: f64, x2: f64, y2: f64, x3: f64, y3: f64, colors: [u32; 3], fill: bool) -> io::Result<()> {
        self.send(&Request::TriangleWithColors {
            points: [x1, y1, x2, y2, x3, y3],
            color: colors,
            fill,
        })
    }

    pub fn circle_popup(&mut self, x: f64, y: f64, radius: f64, text: &str) -> io::Result<()> {
        self.send(&Request::CirclePopup {
            p: [x, y],
            r: radius,
            text: text.to_string(),
        })
    }

    pub fn rectangle_popup(&mut self, x1: f64, y1: f64, x2: f64, y2: f64, text: &str) -> io::Result<()> {
        self.send(&Request::RectanglePopup {
            tl: [x1, y1],
            br: [x2, y2],
            text: text.to_string(),
        })
    }

    pub fn options(&mut self, layer: i32, permanent: bool) -> io::Result<()> {
        self.send(&Request::Options {
            layer,
            permanent
        })
    }

    pub fn message(&mut self, text: &str) -> io::Result<()> {
        self.send(&Request::Message {
            message: text.to_string(),
        })
    }

    pub fn end_frame(&mut self) -> io::Result<()> {
        self.send(&Request::End)
    }
}

#[derive(Serialize)]
#[serde(tag = "type")]
#[serde(rename_all = "lowercase")]
enum Request {
    End,
    Polyline { points: Vec<f64>, color: u32 },
    Circle { p: [f64; 2], r: f64, color: u32, fill: bool },
    Rectangle { tl: [f64; 2], br: [f64; 2], color: u32, fill: bool },
    #[serde(rename = "rectangle")]
    RectangleWithColors { tl: [f64; 2], br: [f64; 2], color: [u32; 4], fill: bool },
    Triangle { points: [f64; 6], color: u32, fill: bool },
    #[serde(rename = "triangle")]
    TriangleWithColors { points: [f64; 6], color: [u32; 3], fill: bool },
    #[serde(rename = "popup")]
    CirclePopup { p: [f64; 2], r: f64, text: String },
    #[serde(rename = "popup")]
    RectanglePopup { tl: [f64; 2], br: [f64; 2], text: String },
    Options { layer: i32, permanent: bool },
    Message { message: String },
}