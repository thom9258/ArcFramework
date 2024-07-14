#pragma once

#include <memory>
#include <type_traits>

namespace arc {

/*todo: window flags should be replaced with a builder pattern*/

class Window {
public:
    Window(arc::Window&&); ///< Default rvalue constructor
    ~Window();

    [[nodiscard]]
    static Window create(const char *title, int x, int y, int w, int h, uint32_t flags);

    [[nodiscard]]
    std::pair<int, int> size() const noexcept;

    void resize(int w, int h);

private:
    Window();
    class Impl;
    std::unique_ptr<Impl> m_impl{nullptr};
};

}
