package main

import (
	"net"
	"bufio"
	"image/color"
	"fmt"
	"math"
	"math/rand"
)

type SideType int

const (
	SIDE_OUR     SideType = -1 + iota
	SIDE_NEUTRAL
	SIDE_ENEMY
)

type AreaType int

const (
	AREA_UNKNOWN AreaType = 0 + iota
	AREA_FOREST
	AREA_SWAMP
	AREA_RAIN
	AREA_CLOUD
)

type UnitType int

const (
	UNIT_UNKNOWN    UnitType = 0 + iota
	UNIT_TANK
	UNIT_IFV
	UNIT_ARRV
	UNIT_HELICOPTER
	UNIT_FIGHTER
)



type FacilityType int

const (
	CONTROL_CENTER    FacilityType = 0 + iota
	VEHICLE_FACTORY
)

type RewindClient struct {
	conn   net.Conn
	reader *bufio.Reader
	writer *bufio.Writer
}

func Client() *RewindClient {
	var host, port string

	host, port = "127.0.0.1", "9111"

	cli := new(RewindClient);
	if err := cli.Dial(host, port); err == nil {
		return cli;
	} else {
		panic(err)
	}
}

func (c *RewindClient) Close() {
	c.conn.Close();
}

func (c *RewindClient) Dial(host, port string) (err error) {
	if c.conn, err = net.Dial("tcp", host+":"+port); err == nil {
		c.reader = bufio.NewReader(c.conn)
		c.writer = bufio.NewWriter(c.conn)
	}

	return
}

func getIntFromColor(color color.Color) uint32 {
	var red, green, blue, alpha = color.RGBA();
	var colorInt = (alpha << 24) + (red << 16) + (green << 8) + (blue);
	return colorInt
}

func (c *RewindClient) circle(x float64, y float64, r float64, color color.Color, layer int) {
	c.writeString(fmt.Sprintf("{\"type\": \"circle\", \"x\": %f, \"y\": %f, \"r\": %f, \"color\": %d, \"layer\": %d}", x, y, r, getIntFromColor(color), layer));
}

func (c *RewindClient) popup(x float64, y float64, r float64, text string) {
	c.writeString(fmt.Sprintf("{\"type\": \"popup\", \"x\": %f, \"y\": %f, \"r\": %f, \"text\": \"%s \"}", x, y, r, text));
}

func (c *RewindClient) rect(x1 float64, y1 float64, x2 float64, y2 float64, color color.Color, layer int) {
	c.writeString(fmt.Sprintf("{\"type\": \"rectangle\", \"x1\": %f, \"y1\": %f, \"x2\": %f, \"y2\": %f, \"color\": %d, \"layer\": %d}", x1, y1, x2, y2, getIntFromColor(color), layer));
}

func (c *RewindClient) line(x1 float64, y1 float64, x2 float64, y2 float64, color color.Color, layer int) {
	c.writeString(fmt.Sprintf("{\"type\": \"line\", \"x1\": %f, \"y1\": %f, \"x2\": %f, \"y2\": %f, \"color\": %d, \"layer\": %d}", x1, y1, x2, y2, getIntFromColor(color), layer));
}

func (c *RewindClient) facility(cell_x int, cell_y int, type FacilityType, side SideType, production int, max_production int, capture int, max_capture int) {
	c.writeString(fmt.Sprintf("{\"type\": \"facility\", \"x\": %d, \"y\": %d, \"facility_type\": %d, \"enemy\": %d, \"production\": %d, \"max_production\": %d, \"capture\": %d, \"max_capture\": %d}",cell_x, cell_y, type, side, production, max_production, capture, max_capture));
}

func (c *RewindClient) livingUnit(x float64, y float64, r float64, hp int, maxHp int, side SideType, course float64, unit UnitType, remCooldown int, maxCooldown int, selected bool) {
	var isSelected int = 0
	if selected {
		isSelected = 1
	}
	c.writeString(fmt.Sprintf("{\"type\": \"unit\", \"x\": %f, \"y\": %f, \"r\": %f, \"hp\": %d, \"max_hp\": %d, \"enemy\": %d, \"unit_type\":%d, \"course\": %.3f,"+
		"\"rem_cooldown\":%d, \"cooldown\":%d, \"selected\":%d }",
		x, y, r, hp, maxHp, side, unit, course, remCooldown, maxCooldown, isSelected));
}

func (c *RewindClient) livingUnitSmaller(x float64, y float64, r float64, hp int, maxHp int, side SideType) {
	c.livingUnit(x, y, r, hp, maxHp, side, 0, UNIT_UNKNOWN, 0, 0, false)
}

func (c *RewindClient) areaDescription(cellX int, cellY int, areaType AreaType) {
	c.writeString(fmt.Sprintf("{\"type\": \"area\", \"x\": %d, \"y\": %d, \"area_type\": %d}", cellX, cellY, areaType));
}

func (c *RewindClient) message(str string) {
	c.writeString(fmt.Sprintf("{\"type\": \"message\", \"message\" : \"%s\"}", str));
}

func (c *RewindClient) endFrame() {
	c.writeString("{\"type\":\"end\"}");
}

func (c *RewindClient) writeString(v string) {
	if _, err := c.writer.WriteString(v); err != nil {
		panic(err)
	}
	c.flush();
}

func (c *RewindClient) flush() {
	if err := c.writer.Flush(); err != nil {
		panic(err)
	}
}

func main() {
	rc := Client()
	worldSize := 1024;
	random := rand.New(rand.NewSource(123));
	for x := 0; x < worldSize/32; x++ {
		for y := 0; y < worldSize/32; y++ {
			rc.areaDescription(x, y, AreaType(random.Intn(5)));
		}
	}
	rc.message("Hello World");
	rc.endFrame();
	for i := 0; i < 2000; i++ {
		rc.message(fmt.Sprintf("Step %d", i));
		for j := 0; j < 100; j++ {
			color := color.RGBA{uint8(random.Intn(256)), uint8(random.Intn(256)), uint8(random.Intn(256)), uint8(random.Intn(256))};
			unitX := random.Float64() * float64(worldSize);
			unitY := random.Float64()*32 + float64(i);
			rc.livingUnit(
				unitX,
				unitY,
				2+random.Float64()*10,
				10,
				100,
				SideType(random.Intn(3)-1),
				math.Pi*random.Float64(),
				UnitType(random.Intn(6)),
				int(60*random.Float32()),
				60,
				false);
			rc.line(unitX, unitY, random.Float64()*float64(worldSize), random.Float64()*float64(worldSize), color, 3);
			circleX := unitX + random.Float64()*40 - 20;
			circleY := unitY + random.Float64()*40 - 20;
			rc.circle(circleX, circleY, 10+random.Float64()*10, color, 2);
			rectX1 := circleX + random.Float64()*100;
			rectY1 := circleY + random.Float64()*100;
			rc.rect(rectX1, rectY1, rectX1+random.Float64()*40, rectY1+random.Float64()*40, color, 1);
		}
		rc.endFrame();
	}

	rc.message("Bye!");
	rc.endFrame();
	rc.Close();
}
