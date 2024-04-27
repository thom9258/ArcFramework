#include "../arc/error.hpp"

#include <iostream>

namespace arc {

const char* gl_error_string(unsigned int _e) 
{
    switch (_e) {
    case GL_NO_ERROR:
        return "no error";
    case GL_OUT_OF_MEMORY:
        return "out of memory";
    case GL_INVALID_ENUM:
        return "invalid enum";
    case GL_INVALID_VALUE:
        return "invalid value";
    case GL_INVALID_OPERATION:
        return "invalid operation";
    case GL_INVALID_FRAMEBUFFER_OPERATION:
        return "invalid framebuffer operation";
    case GL_STACK_UNDERFLOW:
        return "stack underflow";
    case GL_STACK_OVERFLOW:
        return "stack overflow";
    default:
        return "unknown error, should not be possible...";
    }
}

std::optional<std::string> get_gl_error(void)
{
    GLenum err = glGetError();
    if (err == GL_NO_ERROR)
        return {};
    return {gl_error_string(err)};
}

void try_throw_gl_error(const char* _file_, size_t _line_) {
    const auto err = get_gl_error();
    if (err)
        throw Error(_file_, _line_, "GL error: " + *err, ErrorType::ERROR);
}

std::optional<std::string> get_sdl_error(void)
{
    const char* err = SDL_GetError();
    if (err[0] == '\0')
        return {};
    return {err};
}

void try_throw_sdl_error(const char* _file_, size_t _line_) {
    const auto err = get_sdl_error();
    if (err)
        throw Error(_file_, _line_, "SDL error: " + *err, ErrorType::ERROR);
}

}
