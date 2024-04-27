#pragma once

#include <memory>
#include <functional>

#include "error.hpp"
#include "sdlgl.hpp"

namespace arc {

class Window;
using WindowPtr = std::shared_ptr<const Window>;

class Window {
public:
    Window(SDL_Window* ptr);
    ~Window();
    Window(Window&&) = delete;
    Window(const Window&) = delete;
    Window operator=(Window&&) = delete;
    Window operator=(const Window&) = delete;

    const SDL_Window* ptr(void) const { return m_ptr; }
private:
    SDL_Window* m_ptr;
};

[[nodiscard]]
WindowPtr make_Window(const char *title, int x, int y, int w, int h, Uint32 flags);

[[nodiscard]]
std::pair<int, int> size(const WindowPtr window) noexcept;

void resize(WindowPtr window, int w, int h);

}
