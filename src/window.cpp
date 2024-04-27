#include "../arc/window.hpp"

namespace arc {

Window::Window(SDL_Window* ptr) : m_ptr(ptr) {}

Window::~Window() {
    SDL_DestroyWindow((SDL_Window*)m_ptr);
}

WindowPtr make_Window(const char *title, int x, int y, int w, int h, Uint32 flags) {
    if (w < 1 || h < 1 || title == nullptr)
        throw fatal(CODEPOSITION, "Window parameters are invalid");

    auto window = std::make_shared<Window>(SDL_CreateWindow(title, x, y, w, h, flags));
    try_throw_sdl_error(CODEPOSITION);
    if (!window)
        throw fatal(CODEPOSITION, "Window could not be created");

    log_info("Initialized window");
    return window;
}

std::pair<int, int> size(const WindowPtr window) noexcept {
    if (!window)
        return {0,0};
    int w;
    int h;
    SDL_GetWindowSize((SDL_Window*)window->ptr(), &w, &h);
    return {w, h};
}

void resize(WindowPtr window, int w, int h) {
    SDL_SetWindowSize((SDL_Window*)window->ptr(), w, h);
    try_throw_sdl_error(CODEPOSITION);
}

}
