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

    /* ===================================================================
     * Create DescriptorSet Layout for the pipeline
     */
    VkDescriptorSetLayoutBinding viewport_layout_binding{};
    viewport_layout_binding.binding = 0;
    viewport_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    viewport_layout_binding.descriptorCount = 1;
    viewport_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding texture_sampler_layout_binding{};
    texture_sampler_layout_binding.binding = 1;
    texture_sampler_layout_binding.descriptorCount = 1;
    texture_sampler_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    texture_sampler_layout_binding.pImmutableSamplers = nullptr;
    texture_sampler_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    const std::vector<VkDescriptorSetLayoutBinding> bindings = 
        {viewport_layout_binding, texture_sampler_layout_binding};
    
    VkDescriptorSetLayoutCreateInfo descriptorset_layout_info{};
    descriptorset_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorset_layout_info.bindingCount = static_cast<uint32_t>(bindings.size());
    descriptorset_layout_info.pBindings = bindings.data();

    VkDescriptorSetLayout descriptorset_layout{};
    auto status = vkCreateDescriptorSetLayout(device.logical_device(),
                                              &descriptorset_layout_info,
                                              nullptr,
                                              &descriptorset_layout);

    if (status != VK_SUCCESS)
        throw std::runtime_error("failed to create descriptor set layout!");

    const auto vert = ArcGraphics::read_shader_bytecode("../perspective.vert.spv");
    const auto frag = ArcGraphics::read_shader_bytecode("../perspective.frag.spv");
    auto pipeline =
        ArcGraphics::RenderPipeline::Builder(&device, &renderer,
                                             vert, frag,
                                             descriptorset_layout)
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
                                                  pipeline.command_pool(),
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
                                                        pipeline.command_pool(),
                                                        renderer.graphics_queue(),
                                                        pipeline.max_frames_in_flight(),
                                                        image.get());

    if (!texture)
        throw std::runtime_error("Failed to create texture from image!");
    
    /* ===================================================================
     * Create Descriptor Sets
     */
    std::vector<VkDescriptorPoolSize> descriptor_pool_sizes{};
    for (const auto& binding: bindings) {
        VkDescriptorPoolSize size{};
        size.type = binding.descriptorType;
        size.descriptorCount = pipeline.max_frames_in_flight();
        descriptor_pool_sizes.push_back(size);
    }

    VkDescriptorPoolCreateInfo descriptor_pool_info{};
    descriptor_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptor_pool_info.poolSizeCount = static_cast<uint32_t>(descriptor_pool_sizes.size());
    descriptor_pool_info.pPoolSizes = descriptor_pool_sizes.data();
    descriptor_pool_info.maxSets = pipeline.max_frames_in_flight();

    VkDescriptorPool descriptor_pool{};
    status = vkCreateDescriptorPool(device.logical_device(),
                                    &descriptor_pool_info,
                                    nullptr,
                                    &descriptor_pool);
    if (status != VK_SUCCESS)
        throw std::runtime_error("Failed to create descriptor pool for uniform buffer!");




    std::vector<VkDescriptorSetLayout> layouts(pipeline.max_frames_in_flight(),
                                               descriptorset_layout);
    
    VkDescriptorSetAllocateInfo descriptor_pool_alloc_info{};
    descriptor_pool_alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptor_pool_alloc_info.descriptorPool = descriptor_pool;
    descriptor_pool_alloc_info.descriptorSetCount = 
        static_cast<uint32_t>(layouts.size());
    descriptor_pool_alloc_info.pSetLayouts = layouts.data();

    std::vector<VkDescriptorSet> descriptorsets(pipeline.max_frames_in_flight());
    status = vkAllocateDescriptorSets(device.logical_device(),
                                      &descriptor_pool_alloc_info,
                                      descriptorsets.data());
    if (status != VK_SUCCESS)
        throw std::runtime_error("Failed to allocate descriptor sets!");

    std::cout << "Allocated descriptor sets!" << std::endl;
    
    /* ===================================================================
     * Create Uniform Buffers
     */
    std::vector<std::shared_ptr<ArcGraphics::BasicUniformBuffer>> uniform_viewports;
    for (size_t i = 0; i < pipeline.max_frames_in_flight(); i++) {
        auto uniform = ArcGraphics::BasicUniformBuffer::create(device.physical_device(),
                                                               device.logical_device(),
                                                               sizeof(ArcGraphics::ViewPort));
        uniform_viewports.push_back(std::shared_ptr<ArcGraphics::BasicUniformBuffer>(uniform.release()));
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
    for (size_t i = 0; i < pipeline.max_frames_in_flight(); i++) {
        VkDescriptorImageInfo image_info = texture->image_infos()[i];
        uniform_images.push_back(image_info);
    }
    std::cout << "created uniform images" << std::endl;
    
    for (size_t i = 0; i < pipeline.max_frames_in_flight(); i++) {
        // Setup Descriptor buffer for ViewPort
        VkDescriptorBufferInfo buffer_info =
            uniform_viewports[i]->descriptor_buffer_info();
        
        VkWriteDescriptorSet descriptor_write{};
        descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_write.dstSet = descriptorsets[i];
        descriptor_write.dstBinding = 0;
        descriptor_write.dstArrayElement = 0;
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_write.descriptorCount = 1;
        descriptor_write.pBufferInfo = &buffer_info;
        descriptor_write.pImageInfo = nullptr; // Optional
        descriptor_write.pTexelBufferView = nullptr; // Optional
    
        uint32_t count = 1;
        vkUpdateDescriptorSets(device.logical_device(),
                               count,
                               &descriptor_write,
                               0,
                               nullptr);
    }
    
    std::cout << "Allocated uniform buffers!" << std::endl;

    auto view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f),
                                glm::vec3(0.0f, 0.0f, 0.0f),
                                glm::vec3(0.0f, 0.0f, 1.0f));

    const auto rendersize = pipeline.render_size();

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
         * Start Command Buffer
         */
        const auto frameindex = pipeline.wait_for_next_frame();
        const auto flight_frame = pipeline.current_flight_frame(); 
        if (!frameindex)
            break;
        auto command_buffer = pipeline.begin_command_buffer(*frameindex);

        /* =======================================================
         * Submit Drawing Commands
         */
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        /* Bind Viewport
         */
        ArcGraphics::ViewPort viewport{};
        viewport.view = view;
        viewport.proj = proj;
        viewport.model = glm::rotate(glm::mat4(1.0f),
                                     time * glm::radians(90.0f),
                                     glm::vec3(0.0f, 0.0f, 1.0f));
 
        uniform_viewports[flight_frame]->set_uniform(&viewport);
        
        vkCmdBindDescriptorSets(command_buffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                pipeline.layout(),
                                0,
                                1,
                                &descriptorsets[flight_frame],
                                0,
                                nullptr);
        
        /* Bind Vertices and Indices
         */
        VkBuffer vertex_buffers[] = {vertex_buffer->get_buffer()};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);
        vkCmdBindIndexBuffer(command_buffer,
                             index_buffer->get_buffer(),
                             0,
                             VK_INDEX_TYPE_UINT32);
        
        const uint32_t instanceCount = 1;
        const uint32_t firstIndex = 0;
        const int32_t  vertexOffset = 0;
        const uint32_t firstInstance = 0;
        vkCmdDrawIndexed(command_buffer,
                         index_buffer->get_count(),
                         instanceCount,
                         firstIndex,
                         vertexOffset,
                         firstInstance);
        
        /* =======================================================
         * End Command Buffer
         */
        pipeline.end_command_buffer(command_buffer, *frameindex);
    }
    
    vkDestroyDescriptorPool(device.logical_device(), descriptor_pool, nullptr);
    vkDestroyDescriptorSetLayout(device.logical_device(), descriptorset_layout, nullptr);
    
    texture->destroy(device.logical_device());
    pipeline.destroy();
    renderer.destroy();
    device.destroy();
}
