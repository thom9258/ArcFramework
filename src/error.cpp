#include "../arc/error.hpp"

#include <iostream>

namespace arc {


Log::Log(const char *_FILE_, size_t _LINE_, const std::string &what) noexcept
    : m_what(what), m_file(_FILE_), m_line(_LINE_)
{}


const std::string &Log::what(void) const noexcept {
    return m_what;
}

std::string Log::file(void) const noexcept {
    return m_file;
}

size_t Log::line(void) const noexcept {
    return m_line;
}

void log_info(const std::string &what) {
    (void)what;
}

const char* gl_error_string(const unsigned int err) 
{
    switch (err) {
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

void maybe_throw_gl_error(const char* _file_, size_t _line_) {
    const auto err = get_gl_error();
    if (err)
        throw Error(_file_, _line_, "GL error: " + *err);
}

std::optional<std::string> get_sdl_error(void)
{
    const char* err = SDL_GetError();
    if (err[0] == '\0')
        return {};
    return {err};
}

void maybe_throw_sdl_error(const char* _file_, size_t _line_) {
    const auto err = get_sdl_error();
    if (err)
        throw Error(_file_, _line_, "SDL error: " + *err);
}

}
