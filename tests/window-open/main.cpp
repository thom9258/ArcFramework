#include <arc/window.hpp>

//https://gist.github.com/koute/7391344
//https://learnopengl.com/Getting-started/Hello-Window

#define WIDTH 1200
#define HEIGHT 900

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    auto window = arc::Window::create("mywindow", 0, 0, 1200, 800, 0);


    if (!renderer.init(1200, 900)) {
        return 1;
    }

    bool exit = false;
    SDL_Event event;

    while (!exit) {
        renderer.frame_begin();

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                exit = true;
                break;

            case SDL_KEYDOWN:
                switch( event.key.keysym.sym ) {
                case SDLK_ESCAPE:
                    exit = true;
                    break;
                }
                break;

            case SDL_WINDOWEVENT: {
                const SDL_WindowEvent& wev = event.window;
                switch (wev.event) {
                case SDL_WINDOWEVENT_RESIZED:
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                    /*Not handled yet*/
                    break;
                case SDL_WINDOWEVENT_CLOSE:
                    exit = true;
                    break;
                }
            } break;
            }
        }
        renderer.frame_end();
    }
    renderer.destroy();
}
