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

    auto render_pipeline =
        ArcGraphics::RenderPipeline::Builder(&device, &renderer, vert, frag)
        .with_frames_in_flight(3)
        .produce();

    const auto vert = ArcGraphics::read_shader_bytecode("../perspective.vert.spv");
    const auto frag = ArcGraphics::read_shader_bytecode("../perspective.frag.spv");

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
    
    
    /* ===================================================================
     * Create Uniform Buffers
     */
    std::vector<std::shared_ptr<BasicUniformBuffer>> uniform_viewports;
    for (size_t i = 0; i < m_max_frames_in_flight; i++) {
        auto uniform = BasicUniformBuffer::create(m_device->physical_device(),
                                                  m_device->logical_device(),
                                                  sizeof(ViewPort));
        uniform_viewports.push_back(std::shared_ptr<BasicUniformBuffer>(uniform.release()));
    }
    std::cout << "created uniform buffers" << std::endl;
    
    //TODO: I have found out that i should not create a texture image view and sampler,
    // per-texture, but rather one specific one per- renderpipeline!
    // https://vulkan-tutorial.com/Texture_mapping/Combined_image_sampler
    // i need to figure out how to create a sampler and view to be able to facilitate
    // different types of formats, and sizes?
    // one approach seems to be to allocate view/sampler of some max width height, and
    // in the shader accept texture size and to transformation from there,
    std::vector<VkDescriptorImageInfo> uniform_images;
    for (size_t i = 0; i < m_max_frames_in_flight; i++) {
        VkDescriptorImageInfo image_info = TMP_texture->image_infos()[i];
        uniform_images.push_back(image_info);
    }
    std::cout << "created uniform images" << std::endl;
    
    for (size_t i = 0; i < m_max_frames_in_flight; i++) {
        // Setup Descriptor buffer for ViewPort
        VkDescriptorBufferInfo buffer_info =
            uniform_viewports[i]->descriptor_buffer_info();
        
        VkWriteDescriptorSet descriptor_write{};
        descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_write.dstSet = descriptor_sets[i];
        descriptor_write.dstBinding = 0;
        descriptor_write.dstArrayElement = 0;
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_write.descriptorCount = 1;
        descriptor_write.pBufferInfo = &buffer_info;
        descriptor_write.pImageInfo = nullptr; // Optional
        descriptor_write.pTexelBufferView = nullptr; // Optional
    
        uint32_t count = 1;
        vkUpdateDescriptorSets(m_device->logical_device(),
                               count,
                               &descriptor_write,
                               0,
                               nullptr);
    }
    
    std::cout << "Allocated uniform buffers!" << std::endl;

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
