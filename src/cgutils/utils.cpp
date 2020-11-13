//
// Created by valdemar on 21.10.17.
//

#include "utils.h"

#include <common/logger.h>

#include <string>

namespace cg {
namespace details {

void gl_check_error(const char *file, int line) {
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        std::string error;
        //@formatter:off
        switch (errorCode) {
            case GL_INVALID_ENUM:                 error = "INVALID_ENUM"; break;
            case GL_INVALID_VALUE:                error = "INVALID_VALUE"; break;
            case GL_INVALID_OPERATION:            error = "INVALID_OPERATION"; break;
            case GL_OUT_OF_MEMORY:                error = "OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:error = "INVALID_FRAMEBUFFER_OPERATION"; break;
            default:                              error = "UNKONWN_ERROR"; break;
        }
        //@formatter:on
        LOG_ERROR("OPENGL:: %s | %s (%d)", error.c_str(), file, line);
    }
}

}  // namespace details

void APIENTRY debug_output_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei,
                                    const GLchar *message, const void *) {
    // ignore non-significant error/warning codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204) {
        return;
    }

    LOG_WARN("Debug message (%u): %s", id, message);

    //@formatter:off
    switch (source)
    {
        case GL_DEBUG_SOURCE_API_ARB:             fprintf(stderr, "Source: API"); break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB:   fprintf(stderr, "Source: Window System"); break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER_ARB: fprintf(stderr, "Source: Shader Compiler"); break;
        case GL_DEBUG_SOURCE_THIRD_PARTY_ARB:     fprintf(stderr, "Source: Third Party"); break;
        case GL_DEBUG_SOURCE_APPLICATION_ARB:     fprintf(stderr, "Source: Application"); break;
        case GL_DEBUG_SOURCE_OTHER_ARB:           fprintf(stderr, "Source: Other"); break;
        default: break;
    }
    fprintf(stderr, "\n");

    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR_ARB:               fprintf(stderr, "Type: Error"); break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB: fprintf(stderr, "Type: Deprecated Behaviour"); break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB:  fprintf(stderr, "Type: Undefined Behaviour"); break;
        case GL_DEBUG_TYPE_PORTABILITY_ARB:         fprintf(stderr, "Type: Portability"); break;
        case GL_DEBUG_TYPE_PERFORMANCE_ARB:         fprintf(stderr, "Type: Performance"); break;
        case GL_DEBUG_TYPE_OTHER_ARB:               fprintf(stderr, "Type: Other"); break;
        default: break;
    }
    fprintf(stderr, "\n");

    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH_ARB:         fprintf(stderr, "Severity: high"); break;
        case GL_DEBUG_SEVERITY_MEDIUM_ARB:       fprintf(stderr, "Severity: medium"); break;
        case GL_DEBUG_SEVERITY_LOW_ARB:          fprintf(stderr, "Severity: low"); break;
        default: break;
    }
    fprintf(stderr, "\n");
    //@formatter:on
}

}  // namespace cg
