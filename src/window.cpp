#include "../arc/window.hpp"
#include "../arc/sdlgl.hpp"
#include "../arc/error.hpp"

#include "create_resource.hpp"

namespace arc {

class Window::Impl {
public:
    Impl(Impl&&) = delete;
    Impl(const Impl&) = delete;
    Impl operator=(Impl&&) = delete;
    Impl operator=(const Impl&) = delete;

    Impl(const char *title, int x, int y, int w, int h, Uint32 flags)
        : m_ptr(create_resource(SDL_CreateWindow, SDL_DestroyWindow, 
                                title, x, y, w, h, flags))
    {
    }
    ~Impl() = default;
    SDL_Window* raw() {return m_ptr.get();}

private:
    std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> m_ptr;
};

//Window::Window() = default;

Window::~Window() = default;

Window Window::create(const char *title, int x, int y, int w, int h, uint32_t flags) {
    Window window{};
    window.m_impl = std::make_unique<Impl>(title, x, y, w, h, flags);
    return window;
}

std::pair<int, int> Window::size() const noexcept {
    int w;
    int h;
    SDL_GetWindowSize(m_impl->raw(), &w, &h);
    return {w, h};
}

void Window::resize(int w, int h) {
    SDL_SetWindowSize(m_impl->raw(), w, h);
    maybe_throw_sdl_error(CODEPOSITION);
}

}
