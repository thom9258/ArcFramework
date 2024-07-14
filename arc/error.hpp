#pragma once

#include <memory>
#include <optional>
#include <string>

#include "sdlgl.hpp"

#define CODEPOSITION __FILE__, __LINE__

namespace arc {

void maybe_throw_gl_error(const char* _file_, size_t _line_);
void maybe_throw_sdl_error(const char* _file_, size_t _line_);

std::optional<std::string> get_sdl_error();
std::optional<std::string> get_gl_error();

class Log {
public:
    Log(const char *_FILE_, size_t _LINE_, const std::string &what) noexcept;
    ~Log() = default;

    [[nodiscard]]
    const std::string& what(void) const noexcept;

    [[nodiscard]]
    std::string file(void) const noexcept;

    [[nodiscard]]
    size_t line(void) const noexcept;

private:
    std::string m_what; ///< log message
    const char* m_file; ///< file log came from (should be retrieved by macro __FILE__)
    size_t      m_line; ///< line log came from (should be retrieved by macro __LINE__)
};

class Warning : public Log {
    using Log::Log; ///< Inherit Log Constructor
};

class Error : public Log {
    using Log::Log; ///< Inherit Log Constructor
};

class Fatal : public Log {
    using Log::Log; ///< Inherit Log Constructor
};

}
