#include <arc/Device.hpp>

#include <iostream>

#define WIDTH 1200
#define HEIGHT 900

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    const auto device = ArcGraphics::Device::Builder()
        .add_khronos_validation_layer()
        .produce();
    
    std::cout << "device\n"
              << "  instance id: " << device.instance() << "\n"
              << "  physical device id: " << device.physical_device() << "\n"
              << "  logical device id: " << device.logical_device() << std::endl;

    bool exit = false;
    SDL_Event event;
    while (!exit) {
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
    }
}
