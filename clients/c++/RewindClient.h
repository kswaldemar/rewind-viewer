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
    enum Color : uint32_t {
        COLOR_RED = 0xFF0000,
        COLOR_GREEN = 0x00FF00,
        COLOR_BLUE = 0x0000FF,
        COLOR_GRAY = 0x273142,
        COLOR_TRANSPARENT = 0xaa000000, ///ARGB format
        COLOR_YELLOW = 0x00FFFF00, ///Zero transparency mean fully opaque
    };

    RewindClient(const RewindClient &) = delete;
    RewindClient &operator=(const RewindClient&) = delete;

    /**
     * Singletone
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
