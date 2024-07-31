#include <arc/Device.hpp>
#include <arc/Renderer.hpp>
#include <arc/Shader.hpp>
#include <arc/VertexBuffer.hpp>
#include <arc/IndexBuffer.hpp>

#include <iostream>

#define WIDTH 1200
#define HEIGHT 900

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    
    auto device = ArcGraphics::Device::Builder()
        .add_khronos_validation_layer()
        .produce();
    
    std::cout << "device\n"
              << "  instance id: " << device.instance() << "\n"
              << "  physical device id: " << device.physical_device() << "\n"
              << "  logical device id: " << device.logical_device() << std::endl;
    
    auto renderer = ArcGraphics::Renderer::Builder(&device)
        .with_wanted_window_size(1200, 800)
        .with_window_name("Triangle")
        .with_window_flags(SDL_WINDOW_BORDERLESS | SDL_WINDOW_SHOWN)
        .produce();

    const auto vert = ArcGraphics::read_shader_bytecode("../triangle.vert.spv");
    const auto frag = ArcGraphics::read_shader_bytecode("../triangle.frag.spv");

    auto render_pipeline =
        ArcGraphics::RenderPipeline::Builder(&device, &renderer, vert, frag)
        .with_frames_in_flight(3)
        .produce();
    

    std::cout << "===========================================\n"
              << " Produced the entire pipeline\n"
              << "===========================================\n"
              << std::endl;

    const ArcGraphics::VertexBuffer::vector_type vertices = {
        {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
    };
    const ArcGraphics::IndexBuffer::vector_type indices = {
        0, 1, 2, 2, 3, 0
    };

    auto vertex_buffer =
        ArcGraphics::VertexBuffer::create_staging(device.physical_device(),
                                                  device.logical_device(),
                                                  render_pipeline.command_pool(),
                                                  renderer.graphics_queue(),
                                                  vertices);
    
    if (vertex_buffer == nullptr)
        throw std::runtime_error("Failed to create vertex buffer!");
    
    auto index_buffer = ArcGraphics::IndexBuffer::create(device.physical_device(),
                                                         device.logical_device(),
                                                         indices);
    
    if (index_buffer == nullptr)
        throw std::runtime_error("Failed to create index buffer!");
    
    render_pipeline.add_geometry(vertex_buffer.get(), index_buffer.get());
    
 
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
                    //context.window_resized_event();
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
        render_pipeline.draw_frame();
    }
    
    render_pipeline.destroy();
    renderer.destroy();
    device.destroy();

    
}
