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
public:
    enum Color : uint32_t {
        COLOR_RED   = 0xFF0000,
        COLOR_GREEN = 0x00FF00,
        COLOR_BLUE  = 0x0000FF,
        COLOR_GRAY  = 0x273142,
    };

    ///Singleton
    static RewindClient& instance() {
        static std::string HOST = "127.0.0.1";
        static uint16_t PORT = 7000;
        static RewindClient inst(HOST, PORT);
        return inst;
    }

    ///Should be send on end of move function
    ///all turn primitives can be rendered after that point
    void end_frame() {
        send(R"({"type":"end"})");
    }

    void circle(double x, double y, double r, uint32_t color) {
        static const char *fmt =
            R"({"type": "circle", "x": %lf, "y": %lf, "r": %lf, "color": %u})";
        send(format(fmt, x, y, r, color));
    }

    void rect(double x1, double y1, double x2, double y2, uint32_t color) {
        static const char *fmt =
            R"({"type": "rectangle", "x1": %lf, "y1": %lf, "x2": %lf, "y2": %lf, "color": %u})";
        send(format(fmt, x1, y1, x2, y2, color));
    }

    void line(double x1, double y1, double x2, double y2, uint32_t color) {
        static const char *fmt =
            R"({"type": "line", "x1": %lf, "y1": %lf, "x2": %lf, "y2": %lf, "color": %u})";
        send(format(fmt, x1, y1, x2, y2, color));
    }

    ///Living unit - circle with HP bar
    ///x, y, r - same as for circle
    ///hp, max_hp - current life level and maximum level respectively
    ///enemy - 3 state variable: 1 - for enemy; -1 - for friend; 0 - neutral.
    ///course - parameter needed only to properly rotate textures (it unused by untextured units)
    ///unit_type - define used texture, value 0 means 'no texture'. For supported textures see enum UnitType in Frame.h
    void living_unit(double x, double y, double r, int hp, int max_hp,
                     int enemy, double course = 0, int utype = 0) {
        static const char *fmt =
            R"({"type": "unit", "x": %lf, "y": %lf, "r": %lf, "hp": %d, "max_hp": %d, "enemy": %d, "unit_type":%d, )"
            R"("course": %.3lf})";
        send(format(fmt, x, y, r, hp, max_hp, enemy, utype, course));
    }

    ///Weather or terrain description in specific cell
    ///See Frame.h AreaType for available terrain and weather types
    void area_description(int cell_x, int cell_y, int area_type) {
        static const char *fmt =
            R"({"type": "area", "x": %d, "y": %d, "area_type": %d})";
        send(format(fmt, cell_x, cell_y, area_type));
    }


    ///Pass arbitrary user message to be stored in frame
    ///Message content displayed in separate window inside viewer
    ///Can be used several times per frame
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
        if (!socket_.Open(reinterpret_cast<const uint8_t*>(host.c_str()), port)) {
            fprintf(stderr, "RewindClient:: Cannot open viewer socket. Launch viewer before strategy");
        }
    }

    void send(const std::string &buf) {
        socket_.Send(reinterpret_cast<const uint8_t *>(buf.c_str()), buf.size());
    }

    CActiveSocket socket_;
};
