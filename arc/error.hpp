#pragma once

#include <memory>
#include <optional>

#include "sdlgl.hpp"

#define CODEPOSITION __FILE__, __TYPE__

namespace arc {


enum class ErrorType {
    WARNING,
    ERROR,
};

class Error {
public:
    Error(const char *_FILE_, size_t _LINE_, const std::string &what,
          const ErrorType type)
        : m_file(_FILE_), m_line(_LINE_), m_what(what), m_type(type) {}

    const char*        file(void) { return m_file; }
    size_t             line(void) { return m_line; }
    const std::string& what(void) { return m_what; }
    ErrorType          type(void) { return m_type; }

private:
    const char* m_file;
    size_t m_line;
    std::string m_what;
    ErrorType m_type;
};

[[nodiscard]]
const char* gl_error_string(unsigned int _e);

[[nodiscard]]
std::optional<std::string> get_gl_error(void);

[[nodiscard]]
std::optional<std::string> get_sdl_error(void);

void try_throw_gl_error(const char* _file_, size_t _line_);

void try_throw_sdl_error(const char* _file_, size_t _line_);

}
