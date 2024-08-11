#include <arc/Device.hpp>
#include <arc/Renderer.hpp>
#include <arc/RenderPipeline.hpp>
#include <arc/VertexBuffer.hpp>
#include <arc/IndexBuffer.hpp>
#include <arc/Texture.hpp>

#include <iostream>
#include <chrono>

#define WIDTH 1200
#define HEIGHT 900

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    
    auto device = ArcGraphics::Device::Builder()
        .add_khronos_validation_layer()
        .produce();
   
    auto renderer = ArcGraphics::Renderer::Builder(&device)
        .with_wanted_window_size(1200, 800)
        .with_window_name("Triangle")
        .with_window_flags(SDL_WINDOW_BORDERLESS | SDL_WINDOW_SHOWN)
        .produce();

    const auto vert = ArcGraphics::read_shader_bytecode("../perspective.vert.spv");
    const auto frag = ArcGraphics::read_shader_bytecode("../perspective.frag.spv");

    auto render_pipeline =
        ArcGraphics::RenderPipeline::Builder(&device, &renderer, vert, frag)
        .with_frames_in_flight(3)
        .produce();

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
    
    if (!vertex_buffer)
        throw std::runtime_error("Failed to create vertex buffer!");
    
    auto index_buffer = ArcGraphics::IndexBuffer::create(device.physical_device(),
                                                         device.logical_device(),
                                                         indices);
    
    if (!index_buffer)
        throw std::runtime_error("Failed to create index buffer!");
    

    auto image = ArcGraphics::Image::load_from_path("../texture.jpg");
    
    if (!image)
        throw std::runtime_error("Failed to load image from path!");
    
    auto texture = ArcGraphics::Texture::create_staging(device.physical_device(),
                                                        device.logical_device(),
                                                        render_pipeline.command_pool(),
                                                        renderer.graphics_queue(),
                                                        image.get());

    if (!texture)
        throw std::runtime_error("Failed to create texture from image!");

    auto view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f),
                                glm::vec3(0.0f, 0.0f, 0.0f),
                                glm::vec3(0.0f, 0.0f, 1.0f));

    const auto rendersize = render_pipeline.render_size();

    auto proj = glm::perspective(glm::radians(45.0f),
                                     rendersize.width / (float)rendersize.height,
                                     0.1f,
                                     10.0f);
    proj[1][1] *= -1;

    auto startTime = std::chrono::high_resolution_clock::now();

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
        render_pipeline.start_frame(view, proj);
        
        ArcGraphics::DrawableGeometry triangle{};
        triangle.vertices = vertex_buffer.get();
        triangle.indices = index_buffer.get();
        

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
        triangle.model = glm::rotate(glm::mat4(1.0f),
                                     time * glm::radians(90.0f),
                                     glm::vec3(0.0f, 0.0f, 1.0f));
 
        render_pipeline.add_geometry(triangle);
        render_pipeline.draw_frame();
    }
    
    texture->destroy(device.logical_device());
    render_pipeline.destroy();
    renderer.destroy();
    device.destroy();
}
