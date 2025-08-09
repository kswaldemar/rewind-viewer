//
// Created by valdemar on 10.02.2020.
//

#include "ShaderCollection.h"

// Include embedded shaders
#include "resources/shaders/circle.frag.h"
#include "resources/shaders/circle.geom.h"
#include "resources/shaders/circle.vert.h"
#include "resources/shaders/color_pos.frag.h"
#include "resources/shaders/color_pos.vert.h"
#include "resources/shaders/simple.vert.h"
#include "resources/shaders/uniform_color.frag.h"

ShaderCollection::ShaderCollection()
    : color_pos(Shader::EmbeddedShaders{color_pos_vert, color_pos_frag})
    , circle(Shader::EmbeddedShaders{circle_vert, circle_frag, circle_geom})
    , color(Shader::EmbeddedShaders{simple_vert, uniform_color_frag}) {}
