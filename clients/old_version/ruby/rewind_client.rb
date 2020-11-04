require 'json'
require 'socket'

module AreaType
  UNKNOWN = 0
  FOREST  = 1
  SWAMP   = 2
  RAIN    = 3
  CLOUD   = 4

  def self.random
    const_get(constants.sample)
  end
end

module Side
  OUR     = -1
  NEUTRAL = 0
  ENEMY   = 1

  def self.random
    const_get(constants.sample)
  end
end

module UnitType
  UNKNOWN    = 0
  TANK       = 1
  IFV        = 2
  ARRV       = 3
  HELICOPTER = 4
  FIGHTER    = 5

  def self.random
    const_get(constants.sample)
  end
end

# Packs rgba value into a integer
class Color

  attr_reader :r, :g, :b, :a

  def initialize(r, g, b, a = 0)
    @r = r
    @g = g
    @b = b
    @a = a
  end

  def self.random
    Color.new(
      rand(255),
      rand(255),
      rand(255),
      rand(255),
    )
  end

  def opacity(value)
    @a = value
    self
  end

  # as described in https://docs.oracle.com/javase/7/docs/api/java/awt/Color.html#getRGB()
  def to_i
    (a << 24) + (r << 16) + (g << 8) + b
  end

  RED   = Color.new(0xff, 0, 0)
  GREEN = Color.new(0, 0xff, 0)
  BLUE  = Color.new(0, 0, 0xff)
  WHITE = Color.new(0xff, 0xff, 0xff)
  BLACK = Color.new(0, 0, 0)
end


class RewindClient

  def initialize(host = '127.0.0.1', port = 9111)
    @socket = TCPSocket.new(host, port)
    socket.setsockopt(Socket::IPPROTO_TCP, Socket::TCP_NODELAY, 1)
  end

  def frame
    yield self
  ensure
    end_frame
  end

  # finalizes frame composition. Must be called for anything to be drawn.
  # You may want to use RewindClient#frame method, which calls this at the end of the block.
  def end_frame
    send_json(type: 'end')
  end

  # @param x [Float]
  # @param y [Float]
  # @param r [Float] radius
  # @param color [Integer] RGBA (see Color class)
  # @param layer [Integer] layer to draw on
  def circle(x, y, r, color, layer)
    send_json(type: 'circle', x: x, y: y, r: r, color: color.to_i, layer: layer)
  end

  def rect(x1, y1, x2, y2, color, layer)
    send_json(type: 'rectangle', x1: x1, y1: y1, x2: x2, y2: y2, color: color.to_i, layer: layer)
  end

  def line(x1, y1, x2, y2, color, layer)
    send_json(type: 'line', x1: x1, y1: y1, x2: x2, y2: y2, color: color.to_i, layer: layer)
  end

  def popup(x, y, r, text)
    send_json(type: 'popup', x: x, y: y, r: r, text: text)
  end

  def living_unit(x, y, r, hp, max_hp, side, course = 0, unit_type = UnitType::UNKNOWN, rem_cooldown = 0, max_cooldown = 0, selected = false)
    send_json(
      type:         'unit',
      x:            x, y: y, r: r, hp: hp, max_hp: max_hp, enemy: side,
      unit_type:    unit_type, course: course.round(3),
      rem_cooldown: rem_cooldown, cooldown: max_cooldown,
      selected:     selected ? 1 : 0,
    )
  end
  
  # @param cell_x [Integer] x of top-left facility cell
  # @param cell_y [Integer] y of top-left facility cell
  # @param type [Integer] one of FacilityType constants
  # @param side [Integer] ally, neutral or enemy
  # @param production [Integer] current production progress, set to 0 if no production
  # @param max_production [Integer] maximum production progress, used together with `production`
  # @param capture [Integer] current capture progress, should be in range [-max_capture, max_capture], where negative values mean that facility is being captured by enemy
  # @param max_capture [Integer] maximum capture progress, used together with `capture`
  def facility(cell_x, cell_y, type, side, production, max_production, capture, max_capture)
    send_json(
      type:           'facility',
      x:              cell_x,
      y:              cell_y,
      facility_type:  type,
      enemy:          side,
      production:     production,
      max_production: max_production,
      capture:        capture,
      max_capture:    max_capture
    )
  end

  def area_description(cell_x, cell_y, area_type)
    send_json(type: 'area', x: cell_x, y: cell_y, area_type: area_type)
  end

  def message(msg)
    send_json(type: 'message', message: msg)
  end

  def close
    socket.close
  end

  private

  attr_reader :socket

  def send_json(hash)
    puts hash.to_json
    socket.write(hash.to_json)
    socket.write("\n")
    socket.flush
  end
end

if __FILE__ == $0
  rc = RewindClient.new

  world_size = 1024
  rc.frame do
    0.upto(31) do |x|
      0.upto(31) do |y|
        rc.area_description(x, y, AreaType.random)
      end
    end

    rc.message('hello world')
  end

  100.times do |i|
    rc.frame do
      rc.message("Step #{i}")

      100.times do
        color = Color::BLACK.opacity(rand(255))
        # color = Color::RED

        unit_x = rand(world_size)
        unit_y = rand(32) + i

        rc.living_unit(
          unit_x, unit_y,
          2 + rand(10),
          10,
          100,
          Side.random,
          Math::PI * rand,
          UnitType.random,
          rand(60),
          60,
          rand() > 0.8
        )

        rc.line(unit_x, unit_y, rand(world_size), rand(world_size), color, 3)


        circle_x = unit_x + rand(40) - 20
        circle_y = unit_y + rand(40) - 20

        rc.circle(circle_x, circle_y, 10 + rand(10), color, 2)

        rect_x1 = circle_x + rand(100)
        rect_y1 = circle_y + rand(100)
        rc.rect(rect_x1, rect_y1, rect_x1 + rand(40), rect_y1 + rand(40), color, 1)

        msg = <<-TEXT
Hello!
This is popup message!
abcdefg
        TEXT
        rc.popup(128, 128, 28, msg)
      end
    end
    sleep(0.1)
  end

  rc.frame do
    rc.message('Bye!')
  end

  rc.close
end
