#pragma once

#include <memory>
#include <optional>

#include "sdlgl.hpp"

#define CODEPOSITION __FILE__, __LINE__

namespace arc {


enum class MessageType {
    WARNING,
    ERROR,
    FATAL,
};

class Message {
public:
    Message(const char *_FILE_, size_t _LINE_, const std::string &what,
            MessageType type)
        : m_file(_FILE_), m_line(_LINE_), m_what(what), m_type(type) {}

    const std::string& what(void) { return m_what; }
    MessageType        type(void) { return m_type; }
    const char*        file(void) { return m_file; }
    size_t             line(void) { return m_line; }

private:
    const char* m_file;
    size_t m_line;
    std::string m_what;
    MessageType m_type;
};

void log_info(const std::string &what);

Message warning(const char *_FILE_, size_t _LINE_, const std::string &what);
Message error(const char *_FILE_, size_t _LINE_, const std::string &what);
Message fatal(const char *_FILE_, size_t _LINE_, const std::string &what);

void try_throw_gl_error(const char* _file_, size_t _line_);
void try_throw_sdl_error(const char* _file_, size_t _line_);

}
