//
// Created by valdemar on 22.10.17.
//
///Drawable primitives for Russian AI Cup 2017
#pragma once

#include <cstdint>

struct Point {
    float x;
    float y;
};

struct OrientPoint : Point {
    float course;
};

struct Unit : OrientPoint {
    uint16_t hp;
    uint32_t id;
};