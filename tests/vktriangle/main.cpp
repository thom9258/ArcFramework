#include <arc/Context.hpp>

#define WIDTH 1200
#define HEIGHT 900

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    
    const arc::GraphicsContext::ValidationLayers validation_layers = {
        "VK_LAYER_KHRONOS_validation"
    };

    const arc::GraphicsContext::DeviceExtensions device_extensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
    
    const auto vertex_bytecode = arc::read_shader_bytecode("../triangle.vert.spv");
    const auto fragment_bytecode = arc::read_shader_bytecode("../triangle.frag.spv");

    auto context = arc::GraphicsContext::create(WIDTH,
                                                HEIGHT,
                                                validation_layers,
                                                device_extensions,
                                                vertex_bytecode,
                                                fragment_bytecode
                                                );

    bool exit = false;
    SDL_Event event;
    while (!exit) {
        
        /* =======================================================
         * Handle Inputs
         */
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

        /* =======================================================
         * Draw Frame
         */
        context->draw_frame();
    }
}
