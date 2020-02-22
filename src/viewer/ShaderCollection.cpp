//
// Created by valdemar on 10.02.2020.
//

#include "ShaderCollection.h"

ShaderCollection::ShaderCollection()
    : line("lines.vert", "lines.frag")
    , circle("circle.vert", "circle.frag", "circle.geom")
    , color("simple.vert", "uniform_color.frag")
{ }
