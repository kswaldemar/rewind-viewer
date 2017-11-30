#include <cstdlib>
#include <cstdio>
#include <string>
#include <cstdint>

#include "ActiveSocket.h"

/**
 *  Class for interaction with rewind-viewer from your own startegy class
 *  
 *  Implemented using CActiveSocket, which is shipped with cpp-cgdk
 *  For each frame (game tick) rewind-viewer expect "end" command at frame end
 *  All objects should be represented as json string, 
 *  and will be decoded at viewer side to corresponding structures
 *
 *  Every object has mandatory field "type" and arbitrary number of additional fields
 *  For example end will looks like
 *      {"type": "end"}
 *  
 *  For available types see enum PrimitveType in Frame.h header
 *
 *  For terrain and weather support see area_description command.
 *  You can send several terrain/weather commands with same coordinates.
 *  Note: Terrain is static command and affect all frames. So you need to send it only once per game session.
 *
 *  Colors has ARGB format, if you set only RGB component alpha channel will be set to full opaque value 255.
 *
 *  Note: All command, except terrain, only affect currently rendering frame and will not appear in next frame
 *
 *  Layers:
 *   - Circle, Line and Rectangle support explicit layer where they need to be drawn
 *   - By default all primitives will be drawn in DEFAULT_LAYER (see Frame.h for more information)
 *   - Layers are 1-indexed to better match with shortcuts.
 *     So latest available layer has index LAYERS_COUNT (see Frame.h)
 *   - Layers drawed in ascending order
 */
class RewindClient {
public:
    ///You may use another value, but units always will be drawn in DEFAULT_LAYER, described in Frame.h
    constexpr static int DEFAULT_LAYER = 3;

    enum Color : uint32_t {
        COLOR_RED = 0xFF0000,
        COLOR_GREEN = 0x00FF00,
        COLOR_BLUE = 0x0000FF,
        COLOR_GRAY = 0x273142,
        COLOR_TRANSPARENT = 0xaa000000, ///ARGB format
        COLOR_YELLOW = 0x00FFFF00, ///Zero transparency mean fully opaque
    };

    enum class Side {
        ALLY = -1,
        NEUTRAL = 0,
        ENEMY = 1,
    };

    enum class UnitType {
        UNKNOWN = 0,
        TANK = 1,
        IFV = 2,
        ARRV = 3,
        HELICOPTER = 4,
        FIGHTER = 5
    };

    enum class FacilityType {
        CONTROL_CENTER = 0,
        VEHICLE_FACTORY = 1,
    };

    enum class AreaType {
        UNKNOWN = 0,
        FOREST,
        SWAMP,
        RAIN,
        CLOUD,
    };

    /**
     * Singleton
     */
    static RewindClient &instance() {
        static std::string HOST = "127.0.0.1";
        static uint16_t PORT = 9111;
        static RewindClient inst(HOST, PORT);
        return inst;
    }

    /**
     * Should be send on end of move function
     * all turn primitives will be rendered after that point
     */
    void end_frame() {
        send(R"({"type":"end"})");
    }

    void circle(double x, double y, double r, uint32_t color, size_t layer = DEFAULT_LAYER) {
        static const char *fmt =
            R"({"type": "circle", "x": %lf, "y": %lf, "r": %lf, "color": %u, "layer": %u})";
        send(format(fmt, x, y, r, color, layer));
    }

    void popup(double x, double y, double r, std::string text) {
        static const char *fmt =
            R"({"type": "popup", "x": %lf, "y": %lf, "r": %lf, "text": %s})";
        send(format(fmt, x, y, r, text.c_str()));
    }

    void rect(double x1, double y1, double x2, double y2, uint32_t color, size_t layer = DEFAULT_LAYER) {
        static const char *fmt =
            R"({"type": "rectangle", "x1": %lf, "y1": %lf, "x2": %lf, "y2": %lf, "color": %u, "layer": %u})";
        send(format(fmt, x1, y1, x2, y2, color, layer));
    }

    void line(double x1, double y1, double x2, double y2, uint32_t color, size_t layer = DEFAULT_LAYER) {
        static const char *fmt =
            R"({"type": "line", "x1": %lf, "y1": %lf, "x2": %lf, "y2": %lf, "color": %u, "layer": %u})";
        send(format(fmt, x1, y1, x2, y2, color, layer));
    }

    /**
     * Living unit - circle with HP and cooldown bars
     * @param x - absolute X coordinate
     * @param y - absolute X coordinate
     * @param r - radius in game units
     * @param hp - current health
     * @param max_hp - maximum health
     * @param cooldown - current cooldown. Cooldown bar only shown when value > 0
     * @param max_cooldown - maximum cooldown. Cooldwon bar disabled if value == 0
     * @param selected - true if selected by user (will have different color)
     * @param side - enemy, ally or neutral
     * @param course - faced direction, need only for correct texture display
     * @param utype - unit type, need to choose texture, UNKNOWN mean untextured
     */
    void living_unit(double x, double y, double r,
                     int hp, int max_hp,
                     int cooldown, int max_cooldown,
                     bool selected,
                     Side side, double course = 0, UnitType utype = UnitType::UNKNOWN) {
        static const char *fmt =
            R"({"type": "unit", "x": %lf, "y": %lf, "r": %lf, "hp": %d, "max_hp": %d, "enemy": %d, "unit_type":%d, )"
                R"("rem_cooldown": %d, "cooldown": %d, "selected":%d, "course": %.3lf})";
        send(format(fmt, x, y, r, hp, max_hp,
                    static_cast<int>(side), static_cast<int>(utype),
                    cooldown, max_cooldown, selected, course));
    }

    /**
    * Facility - rectangle with texture and progress bars
    * @param cell_x - x cell of top left facility part
    * @param cell_y - y cell of top left facility part
    * @param type - type of facility
    * @param side - enemy, ally or neutral
    * @param production - current production progress, set to 0 if no production
    * @param max_production - maximum production progress, used together with `production`
    * @param capture - current capture progress, should be in range [-max_capture, max_capture],
    * where negative values mean that facility is capturing by enemy
    * @param max_capture - maximum capture progress, used together with `capture`
    */
    void facility(int cell_x, int cell_y,
                  FacilityType type, Side side,
                  int production, int max_production,
                  int capture, int max_capture) {
        static const char *fmt =
            R"({"type": "facility", "x": %d, "y": %d, "facility_type": %d, "enemy": %d,)"
                R"("production": %d, "max_production": %d, "capture": %d, "max_capture": %d})";
        send(format(fmt, cell_x, cell_y,
                    static_cast<int>(type), static_cast<int>(side),
                    production, max_production, capture, max_capture));
    }

    /**
     * Weather or terrain description in specific cell
     * See Frame.h AreaType for available terrain and weather types
     */
    void area_description(int cell_x, int cell_y, AreaType area_type) {
        static const char *fmt =
            R"({"type": "area", "x": %d, "y": %d, "area_type": %d})";
        send(format(fmt, cell_x, cell_y, static_cast<int>(area_type)));
    }


    /**
     * Pass arbitrary user message to be stored in frame
     * Message content displayed in separate window inside viewer
     * Can be used several times per frame
     */
    template<typename... Args>
    void message(Args... args) {
        std::string s = R"({"type": "message", "message": ")";
        s += format(args...);
        s += "\"}\n";
        send(s);
    }

private:
    template<typename... Args>
    static inline std::string format(const char *fmt, Args... args) {
        static char buf[2048];
        int bytes = sprintf(buf, fmt, args...);
        buf[bytes] = '\0';
        return std::string(buf);
    }

    RewindClient(const std::string &host, uint16_t port) {
        socket_.Initialize();
        socket_.DisableNagleAlgoritm();
        if (!socket_.Open(reinterpret_cast<const uint8_t *>(host.c_str()), port)) {
            fprintf(stderr, "RewindClient:: Cannot open viewer socket. Launch viewer before strategy");
        }
    }

    void send(const std::string &buf) {
        socket_.Send(reinterpret_cast<const uint8_t *>(buf.c_str()), buf.size());
    }

    CActiveSocket socket_;
};
