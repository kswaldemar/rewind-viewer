//
// Created by valdemar on 10.02.2020.
//

#include "ShaderCollection.h"

ShaderCollection::ShaderCollection()
    : line("lines2.vert", "lines2.frag")
    , circle("circle2.vert", "circle2.frag", "circle2.geom")
{ }
