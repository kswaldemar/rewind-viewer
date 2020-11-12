#pragma once

#include <array>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include "ActiveSocket.h"

/**
 *  Class for interaction with rewind-viewer from your own strategy class
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
 *  For available types see enum PrimitiveType in Frame.h header
 *
 *  Colors has ARGB format, if you set only RGB component alpha channel will be set to full opaque value 255.
 *
 *  Note: All command only affect currently rendering frame and will not appear in the next frame
 *
 *  Layers:
 *   - By default all primitives will be drawn in DEFAULT_LAYER (see Frame.h for more information)
 *   - Layers are 1-indexed to better match with shortcuts
 *     So the last available layer has index LAYERS_COUNT (see Frame.h)
 *   - Layers are drawn in ascending order
 */
class RewindClient {
public:
    enum Color : uint32_t {
        COLOR_RED = 0xFF0000,
        COLOR_GREEN = 0x00FF00,
        COLOR_BLUE = 0x0000FF,
        COLOR_TRANSPARENT = 0xaa000000, ///ARGB format
        COLOR_YELLOW = 0x00FFFF00, ///Zero transparency mean fully opaque
        COLOR_WHITE = 0xFFFFFF
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

    void circle(double x, double y, double r, uint32_t color, bool fill = false) {
        static const char *fmt =
            R"({"type": "circle", "p": [%lf, %lf], "r": %lf, "color": %u, "fill": %s})";
        send(format(fmt, x, y, r, color, fill ? "true" : "false"));
    }

    void rectangle(double x1, double y1, double x2, double y2, uint32_t color, bool fill = false) {
        static const char *fmt =
            R"({"type": "rectangle", "tl": [%lf, %lf], "br": [%lf, %lf], "color": %u, "fill": %s})";
        send(format(fmt, x1, y1, x2, y2, color, fill ? "true" : "false"));
    }

    void rectangle(double x1, double y1, double x2, double y2, const std::array<uint32_t, 4> &colors, bool fill = true) {
        static const char *fmt =
            R"({"type": "rectangle", "tl": [%lf, %lf], "br": [%lf, %lf], "color": [%u, %u, %u, %u], "fill": %s})";
        send(format(fmt, x1, y1, x2, y2, colors[0], colors[1], colors[2], colors[3], fill ? "true" : "false"));
    }

    void triangle(double x1, double y1, double x2, double y2, double x3, double y3, uint32_t color, bool fill = false) {
        static const char *fmt =
            R"({"type": "triangle", "points": [%lf, %lf, %lf, %lf, %lf, %lf], "color": %u, "fill": %s})";
        send(format(fmt, x1, y1, x2, y2, x3, y3, color, fill ? "true" : "false"));
    }

    void triangle(double x1, double y1, double x2, double y2, double x3, double y3, const std::array<uint32_t, 3> &colors, bool fill = true) {
        static const char *fmt =
            R"({"type": "triangle", "points": [%lf, %lf, %lf, %lf, %lf, %lf], "color": [%u, %u, %u], "fill": %s})";
        send(format(fmt, x1, y1, x2, y2, x3, y3, colors[0], colors[1], colors[2], fill ? "true" : "false"));
    }

    void line(double x1, double y1, double x2, double y2, uint32_t color) {
        static const char *fmt =
            R"({"type": "polyline", "points": [%lf, %lf, %lf, %lf], "color": %u})";
        send(format(fmt, x1, y1, x2, y2, color));
    }

    void polyline(const std::vector<double> &points, uint32_t color) {
        std::string s = R"({"type": "polyline", "points": [)";
        bool first = true;
        for (double p : points) {
            if (!first) s += ",";
            first = false;
            s += format("%lf", p);
        }
        s += format(R"(], "color": %u})", color);
        send(s);
    }

    /**
     * Pass arbitrary user message to be stored in frame
     * Message content displayed in separate window inside viewer
     * Can be used several times per frame
     * It can be used like printf, e.g.: message("This %s will be %s", "string", "formatted")
     */
    template<typename... Args>
    void message(Args... args) {
        std::string s = R"({"type": "message", "message": ")";
        s += format(args...);
        s += "\"}";
        send(s);
    }

    void popup(double x, double y, double r, const std::string& text) {
        static const char *fmt =
            R"({"type": "popup", "p": [%lf, %lf], "r": %lf, "text": "%s"})";
        send(format(fmt, x, y, r, text.c_str()));
    }

    void popup(double x1, double y1, double x2, double y2, const std::string& text) {
        static const char *fmt =
            R"({"type": "popup", "tl": [%lf, %lf], "br": [%lf, %lf], "text": "%s"})";
        send(format(fmt, x1, y1, x2, y2, text.c_str()));
    }

    void set_options(int layer, bool permanent = false) {
        static const char *fmt =
            R"({"type": "options", "layer": %i, "permanent": %s})";
        send(format(fmt, layer, permanent ? "true" : "false"));
    }

    /**
     * Should be send on end of move function
     * all turn primitives will be rendered after that point
     */
    void end_frame() {
        send(R"({"type": "end"})");
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
