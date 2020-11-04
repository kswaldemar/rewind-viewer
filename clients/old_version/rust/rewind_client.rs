extern crate serde;
extern crate serde_json;

use std::io;
use std::net::TcpStream;
use self::serde::{Serialize, Serializer};
use self::serde::ser::SerializeStruct;


// Usage example:
//
// #[path = "rewind_client.rs"]
// mod rewind_client;
//
// use self::rewind_client::{RewindClient, Circle, SendToRewind, DEFAULT_LAYER};
//
// let mut rewind = RewindClient::new().expect("Failed to create rewind client");
//
// Circle {x: 1.0, y: 2.0, r: 3.0, color: 0xAABBCC, layer: DEFAULT_LAYER}
//     .send_to_rewind(&mut rewind).unwrap();

#[allow(dead_code)]
pub struct RewindClient {
    stream: TcpStream,
}

#[allow(dead_code)]
impl RewindClient {
    pub fn new() -> io::Result<Self> {
        Self::with_host_port("127.0.0.1", 9111)
    }

    pub fn with_host_port<'r>(host: &'r str, port: u16) -> io::Result<Self> {
        let stream = TcpStream::connect((host, port))?;
        stream.set_nodelay(true)?;
        Ok(RewindClient { stream: stream })
    }

    pub fn send<T: Serialize>(&mut self, value: &T) -> io::Result<()> {
        self.stream.write_as_json(value)
    }
}

pub trait SendToRewind: Serialize {
    fn send_to_rewind(&self, rewind: &mut RewindClient) -> io::Result<()> {
        rewind.send(&self)
    }
}

#[allow(dead_code)]
pub const DEFAULT_LAYER: i32 = 3;

impl<T: Serialize + ?Sized> SendToRewind for T {}

pub trait WriteAsJson: io::Write {
    fn write_as_json<T: Serialize>(&mut self, value: &T) -> io::Result<()> {
        use self::serde_json;
        self.write_all(serde_json::to_string(value)?.as_bytes())
    }
}

impl<T: io::Write + ?Sized> WriteAsJson for T {}

#[allow(dead_code)]
pub struct Circle {
    pub x: f64,
    pub y: f64,
    pub r: f64,
    pub color: u32,
    pub layer: i32,
}

impl Serialize for Circle {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
        where S: Serializer {
        let mut state = serializer.serialize_struct("Circle", 5)?;
        state.serialize_field("type", "circle")?;
        state.serialize_field("x", &self.x)?;
        state.serialize_field("y", &self.y)?;
        state.serialize_field("r", &self.r)?;
        state.serialize_field("color", &self.color)?;
        state.serialize_field("layer", &self.layer)?;
        state.end()
    }
}

#[allow(dead_code)]
pub struct Popup<'r> {
    pub x: f64,
    pub y: f64,
    pub r: f64,
    pub text: &'r str,
}

impl<'r> Serialize for Popup<'r> {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
        where S: Serializer {
        let mut state = serializer.serialize_struct("Popup", 5)?;
        state.serialize_field("type", "popup")?;
        state.serialize_field("x", &self.x)?;
        state.serialize_field("y", &self.y)?;
        state.serialize_field("r", &self.r)?;
        state.serialize_field("text", self.text)?;
        state.end()
    }
}

#[allow(dead_code)]
pub struct Rectangle {
    pub x1: f64,
    pub y1: f64,
    pub x2: f64,
    pub y2: f64,
    pub color: u32,
    pub layer: i32,
}

impl Serialize for Rectangle {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
        where S: Serializer {
        let mut state = serializer.serialize_struct("Rectangle", 7)?;
        state.serialize_field("type", "rectangle")?;
        state.serialize_field("x1", &self.x1)?;
        state.serialize_field("y1", &self.y1)?;
        state.serialize_field("x2", &self.x2)?;
        state.serialize_field("y2", &self.y2)?;
        state.serialize_field("color", &self.color)?;
        state.serialize_field("layer", &self.layer)?;
        state.end()
    }
}

#[allow(dead_code)]
pub struct Line {
    pub x1: f64,
    pub y1: f64,
    pub x2: f64,
    pub y2: f64,
    pub color: u32,
    pub layer: i32,
}

impl Serialize for Line {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
        where S: Serializer {
        let mut state = serializer.serialize_struct("Line", 7)?;
        state.serialize_field("type", "line")?;
        state.serialize_field("x1", &self.x1)?;
        state.serialize_field("y1", &self.y1)?;
        state.serialize_field("x2", &self.x2)?;
        state.serialize_field("y2", &self.y2)?;
        state.serialize_field("color", &self.color)?;
        state.serialize_field("layer", &self.layer)?;
        state.end()
    }
}

#[allow(dead_code)]
pub struct Unit {
    pub x: f64,
    pub y: f64,
    pub r: f64,
    pub hp: i32,
    pub max_hp: i32,
    pub rem_cooldown: i32,
    pub cooldown: i32,
    pub enemy: Side,
    pub course: f64,
    pub selected: bool,
    pub unit_type: UnitType,
}

#[allow(dead_code)]
#[derive(Clone, Copy, Eq, PartialEq, PartialOrd, Ord, Hash)]
pub enum Side {
    Ally = -1,
    Neutral = 0,
    Enemy = 1,
}

#[allow(dead_code)]
#[derive(Clone, Copy, Eq, PartialEq, PartialOrd, Ord, Hash)]
pub enum UnitType {
    Unknown = 0,
    Tank = 1,
    Ifv = 2,
    Arrv = 3,
    Helicopter = 4,
    Fighter = 5,
}

impl Serialize for Unit {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
        where S: Serializer {
        let mut state = serializer.serialize_struct("Unit", 12)?;
        state.serialize_field("type", "unit")?;
        state.serialize_field("x", &self.x)?;
        state.serialize_field("y", &self.y)?;
        state.serialize_field("r", &self.r)?;
        state.serialize_field("hp", &self.hp)?;
        state.serialize_field("max_hp", &self.max_hp)?;
        state.serialize_field("rem_cooldown", &self.rem_cooldown)?;
        state.serialize_field("cooldown", &self.cooldown)?;
        state.serialize_field("enemy", &(self.enemy as i32))?;
        state.serialize_field("course", &self.course)?;
        state.serialize_field("selected", &self.selected)?;
        state.serialize_field("unit_type", &(self.unit_type as i32))?;
        state.end()
    }
}

#[allow(dead_code)]
pub struct Area {
    pub x: i32,
    pub y: i32,
    pub area_type: AreaType,
}

#[allow(dead_code)]
#[derive(Clone, Copy, Eq, PartialEq, PartialOrd, Ord, Hash)]
pub enum AreaType {
    Unknown = 0,
    Forest = 1,
    Swamp = 2,
    Rain = 3,
    Cloud = 4,
}

impl Serialize for Area {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
        where S: Serializer {
        let mut state = serializer.serialize_struct("Area", 4)?;
        state.serialize_field("type", "area")?;
        state.serialize_field("x", &self.x)?;
        state.serialize_field("y", &self.y)?;
        state.serialize_field("area_type", &(self.area_type as i32))?;
        state.end()
    }
}

#[allow(dead_code)]
pub struct End;

impl Serialize for End {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
        where S: Serializer {
        let mut state = serializer.serialize_struct("End", 1)?;
        state.serialize_field("type", "end")?;
        state.end()
    }
}

#[allow(dead_code)]
pub struct Message<'r> {
    pub message: &'r str,
}

impl<'r> Serialize for Message<'r> {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
        where S: Serializer {
        let mut state = serializer.serialize_struct("Message", 2)?;
        state.serialize_field("type", "message")?;
        state.serialize_field("message", self.message)?;
        state.end()
    }
}
