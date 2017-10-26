#include <cstdlib>
#include <cstdio>
#include <string>
#include <cstdint>

#include "ActiveSocket.h"

/**
 *  Class for interaction with rewind-viewer from your own startegy class
 *  
 *  Implemented using CActiveSocket, but cpp-cgdk depends from it anyway
 *  For each frame (game tick) rewind-viewer expect "begin" command at frame start and "end" command at frame end
 *  All objects should be represented as json string, 
 *  and will be decoded at viewer side to corresponding structures
 *  
 *  For example begin will looks like
 *      {"type": "begin"}
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
class RewindClient {
    template<typename... Args>
    static inline std::string format(const char *fmt, Args... args) {
        static char buf[2048];
        int bytes = sprintf(buf, fmt, args...);
        buf[bytes] = '\0';
        return buf;
    }

public:
    enum class Color : uint32_t {
        RED   = 0xFF0000,
        GREEN = 0x00FF00,
        BLUE  = 0x0000FF,
        GRAY  = 0x273142,
    };

    ///Singleton
    static RewindClient& instance() {
        static std::string HOST = "127.0.0.1";
        static uint16_t PORT = 7000;
        static RewindClient inst(HOST, PORT);
        return inst;
    }

    ///Initiate new frame recording
    void beginFrame() {
        send(R"({"type":"begin"})");
    }

    ///Should be send on end of move function
    ///all turn primitives can be rendered after that point
    void endFrame() {
        send(R"({"type":"end"})");
    }

    void circle(double x, double y, double r, Color color = Color::GRAY, bool fill = false) {
        static const char *fmt =
            R"({"type": "circle", "x": %lf, "y": %lf, "r": %lf, "color": %d, "fill": %d})";
        send(format(fmt, x, y, r, static_cast<int32_t>(color), fill));
    }
    void fillCircle(double x, double y, double r, Color color = Color::GRAY) {
        circle(x, y, r, color, true);
    }

    void rect(double x1, double y1, double x2, double y2,
              Color color = Color::GRAY, bool fill = false) {
        static const char *fmt =
            R"({"type": "rectangle", "p1": {"x": %lf, "y": %lf}, "p1": {"x": %lf, "y": %lf}, "color": %ud, "fill": %d})";
        send(format(fmt, x1, y1, x2, y2, static_cast<int32_t>(color), fill));
    }
    void fillRect(double x1, double y1, double x2, double y2, Color color = Color::GRAY) {
        rect(x1, y1, x2, y2, color, true);
    }

    void line(double x1, double y1, double x2, double y2, Color color = Color::GRAY) {
        static const char *fmt =
            R"({"type": "line", "p1": {"x": %lf, "y": %lf}, "p1": {"x": %lf, "y": %lf}, "color": %ud})";
        send(format(fmt, x1, y1, x2, y2, static_cast<int32_t>(color)));
    }

    ///Pass arbitrary user message to be stored in frame
    ///Message content displayed in separate window inside viewer
    ///Can be used several times per frame
    ///By default viewer buffer is 2048 bytes, so string longer is not supported, 
    ///but you can easily split it on client side
    template<typename... Args>
    void message(Args... args) {
        std::string s = R"({"type": "message", "message": ")";
        s += format(args...);
        s += "\"}\n";
        send(s);
    }

    ///Unit example
    ///Hint: Viewer treat object with equal id as same object between frames
    void livingUnit(double x, double y, double r, int hp, int maxhp, int id) {
        static const char *fmt =
            R"({"type": "unit", "x": %lf, "y": %lf, "r": %lf, "hp": %d, "maxhp": %d, "id": %d})";
        send(format(fmt, x, y, r, hp, maxhp, id));
    }

private:
    RewindClient(const std::string &host, uint16_t port) {
        socket_.Initialize();
        socket_.DisableNagleAlgoritm();
        if (!socket_.Open(reinterpret_cast<const uint8_t*>(host.c_str()), port)) {
            fprintf(stderr, "RewindClient:: Cannot open viewer socket. Launch viewer before strategy");
        }
    }

    void send(const std::string &buf) {
        socket_.Send(reinterpret_cast<const uint8_t *>(buf.c_str()), buf.size());
    }

    CActiveSocket socket_;
};