//
// Created by valdemar on 10.02.2020.
//
#pragma once

#include <cgutils/Shader.h>

struct ShaderCollection {
    ShaderCollection();

    Shader line;
    Shader circle;
    // Forward pass vertices with uniform-specified color
    Shader color;
};
