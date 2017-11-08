/**
 *  Class for interaction with rewind-viewer from your own startegy class
 *  
 *  Implemented using CActiveSocket, which is shipped with swift-cgdk
 *  For each frame (game tick) rewind-viewer expect "end" command at frame end
 *  All objects should be represented as json string, 
 *  and will be decoded at viewer side to corresponding structures
 *
 *  Every object has mandatory field "type" and arbitrary number of additional fields
 *  For example end will looks like
 *      {"type": "end"}
 *  
 *  For available types see enum PrimitveType in <path to primitives here> 
 *
 *  In order to provide support for your own primitives:
 *     - add type with name of your choice to PrimitiveType in <path to primitives here> 
 *     - add structure with field description in file mentioned above 
 *       (you don't need to be expert in c++ to do that, just use already defined types as reference)
 *     - add function for desirialisation from json in same file (see defined types in <path to primitives here>)
 *     - send json string with your object instance description
 *  
 *  Note: All command only affect currently rendering frame and will not appear in next frame
 *  Currently you should send object each frame to display it in viewer
 */

public class RewindClient {
  public static let instance = RewindClient(host: "127.0.0.1", port: 9111)!
  
  public enum Color: UInt32 {
    case red   = 0xFF0000
    case green = 0x00FF00
    case blue  = 0x0000FF
    case gray  = 0x273142
  }
  
  fileprivate let tc: TCPClient // просто для короткой запись, ибо писал в блокноте
  
  ///Should be send on end of move function
  ///all turn primitives can be rendered after that point
  public func endFrame() {
    send("{\"type\":\"end\"}")
  }
  
  public func circle(x: Double, y: Double, r: Double, color: UInt32) {
    send("{\"type\": \"circle\", \"x\": \(x), \"y\": \(y), \"r\": \(r), \"color\": \(color)}")
  }
  
  public func rect(x1: Double, y1: Double, x2: Double, y2: Double, color: UInt32) {
    send("{\"type\": \"rectangle\", \"x1\": \(x1), \"y1\": \(y1), \"x2\": \(x2), \"y2\": \(y2), \"color\": \(color)}")
  }
  
  public func line(x1: Double, y1: Double, x2: Double, y2: Double, color: UInt32) {
    send("{\"type\": \"line\", \"x1\": \(x1), \"y1\": \(y1), \"x2\": \(x2), \"y2\": \(y2), \"color\": \(color)}")
  }
  
  ///Living unit - circle with HP bar
  ///x, y, r - same as for circle
  ///hp, max_hp - current life level and maximum level respectively
  ///enemy - 3 state variable: 1 - for enemy; -1 - for friend; 0 - neutral.
  ///course - parameter needed only to properly rotate textures (it unused by untextured units)
  ///unit_type - define used texture, value 0 means 'no texture'. For supported textures see enum UnitType in Frame.h
  public func livingUnit(x: Double, y: Double, r: Double, hp: Int, maxHP: Int, enemy: Int, course: Double = 0, utype: Int = 0) {
    send("{\"type\": \"unit\", \"x\": \(x), \"y\": \(y), \"r\": \(r), \"hp\": \(hp), \"max_hp\": \(maxHP), \"enemy\": \(enemy), \"unit_type\": \(utype), \"course\": \(course)}")
  }
  
  public func message(_ msg: String) {
    send("{\"type\": \"message\", \"message\": \(msg)}")
  }
  
  init?(host: String, port: Int) {
    tc = TCPClient(address: host, port: Int32(port))
    if !tc.connect() {
      print("Can't connect to host: \(host) port: \(port)")
      return nil
    }
  }
  
  private func send(_ str: String) {
    tc.write(bytes: str.utf8.map{ Byte(bitPattern: $0) })
  }
}
