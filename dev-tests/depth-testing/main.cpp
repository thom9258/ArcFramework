#include <arc/Device.hpp>
#include <arc/Renderer.hpp>
#include <arc/RenderPipeline.hpp>
#include <arc/VertexBuffer.hpp>
#include <arc/IndexBuffer.hpp>
#include <arc/Texture.hpp>
#include <arc/SimpleGeometry.hpp>

#include <iostream>
#include <chrono>

std::string from_common_basepath(const std::string& path)
{
    return "../../common/" + path;
}

#define WIDTH 1200
#define HEIGHT 900

struct ViewPort {
    glm::mat4 view;
    glm::mat4 proj;
    glm::mat4 model;
};

VkPipelineVertexInputStateCreateInfo create_vertex_input_state()
{
    const auto binding = ArcGraphics::Vertex_PosTex::get_binding_description();
    const auto attribute = ArcGraphics::Vertex_PosTex::get_attribute_descriptions();

    VkPipelineVertexInputStateCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    info.vertexBindingDescriptionCount = 1;
    info.pVertexBindingDescriptions = &binding;
    info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute.size());
    info.pVertexAttributeDescriptions = attribute.data();
    return info;
}
 

std::tuple<std::vector<VkDescriptorSetLayoutBinding>, VkDescriptorSetLayout>
create_bindings_and_descriptorset_layout(const VkDevice& logical_device)
{
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
    auto status = vkCreateDescriptorSetLayout(logical_device,
                                              &descriptorset_layout_info,
                                              nullptr,
                                              &descriptorset_layout);

    if (status != VK_SUCCESS)
        throw std::runtime_error("failed to create descriptor set layout!");
    return {bindings, descriptorset_layout};
}


int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    
    auto device = ArcGraphics::Device::Builder()
        .add_khronos_validation_layer()
        .produce();
   
    auto renderer = ArcGraphics::Renderer::Builder(&device)
        .with_wanted_window_size(1200, 800)
        .with_window_name("Textures")
        .with_window_flags(SDL_WINDOW_BORDERLESS | SDL_WINDOW_SHOWN)
        .produce();
    
    const auto [bindings, descriptorset_layout] = create_bindings_and_descriptorset_layout(device.logical_device());

    const auto vert = ArcGraphics::read_shader_bytecode("../texture.vert.spv");
    const auto frag =
        //ArcGraphics::read_shader_bytecode("../texture_uvs.frag.spv");
        ArcGraphics::read_shader_bytecode("../texture.frag.spv");
    auto pipeline = 
        ArcGraphics::RenderPipeline::Builder(&device,
            &renderer,
            vert,
            frag,
            descriptorset_layout,
            ArcGraphics::Vertex_PosTex::get_binding_description(),
            ArcGraphics::Vertex_PosTex::get_attribute_descriptions()
            )
        .with_frames_in_flight(3)
        .with_use_alpha_blending(true)
        .with_clear_color(0.2f, 0.2f, 0.4f)
        .produce();
    
    const auto [vertices, indices] = ArcGraphics::create_unit_cube();

    auto vertex_buffer =
        ArcGraphics::VertexBuffer_PosTex::create_staging(device.physical_device(),
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

    //const auto image_path = from_common_basepath("red-brick-wall-512x512.png");
    const auto image_path = from_common_basepath("holey-cheese-1024x1024.png");
    //const auto image_path = from_common_basepath("statue.jpg");
    //const auto image_path = from_common_basepath("jinx_pixel_art.png");
    std::cout << "loading image from path: " << image_path << std::endl;
    auto image = ArcGraphics::Image::load_from_path(image_path);

    if (!image)
        throw std::runtime_error("Failed to load image from path!");
    
    auto texture = ArcGraphics::Texture::create_staging(device.physical_device(),
                                                        device.logical_device(),
                                                        pipeline.command_pool(),
                                                        renderer.graphics_queue(),
                                                        VK_FORMAT_R8G8B8A8_SRGB,
                                                        image.get());
    if (!texture)
        throw std::runtime_error("Failed to create texture from image!");
    
    /* ===================================================================
     * Create Descriptor Sets
     */
    std::vector<VkDescriptorPoolSize> descriptor_pool_sizes = 
        ArcGraphics::create_descriptor_pool_sizes(bindings,
                                                  pipeline.max_frames_in_flight());

    VkDescriptorPoolCreateInfo descriptor_pool_info{};
    descriptor_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptor_pool_info.poolSizeCount = static_cast<uint32_t>(descriptor_pool_sizes.size());
    descriptor_pool_info.pPoolSizes = descriptor_pool_sizes.data();
    descriptor_pool_info.maxSets = pipeline.max_frames_in_flight();

    VkDescriptorPool descriptor_pool{};
    auto status = vkCreateDescriptorPool(device.logical_device(),
                                         &descriptor_pool_info,
                                         nullptr,
                                         &descriptor_pool);
    if (status != VK_SUCCESS)
        throw std::runtime_error("Failed to create descriptor pool for uniform buffer!");

    std::vector<VkDescriptorSetLayout> descriptorset_layouts(pipeline.max_frames_in_flight(),
                                                             descriptorset_layout);
    
    VkDescriptorSetAllocateInfo descriptor_pool_alloc_info{};
    descriptor_pool_alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptor_pool_alloc_info.descriptorPool = descriptor_pool;
    descriptor_pool_alloc_info.descriptorSetCount = 
        static_cast<uint32_t>(descriptorset_layouts.size());
    descriptor_pool_alloc_info.pSetLayouts = descriptorset_layouts.data();

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
                                                               sizeof(ViewPort));
        uniform_viewports.push_back(std::shared_ptr<ArcGraphics::BasicUniformBuffer>(uniform.release()));
    }
    std::cout << "created uniform buffers" << std::endl;
    
    VkDescriptorImageInfo image_info{};
    image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    image_info.imageView = texture->view();
    image_info.sampler = texture->sampler();
    
    // TODO: This is very rigid and hardcoded, can it be made easier?
    std::array<VkWriteDescriptorSet, 2> descriptor_writes{};
    for (size_t i = 0; i < pipeline.max_frames_in_flight(); i++) {
        const auto buffer_info = uniform_viewports[i]->descriptor_buffer_info();
        descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[0].dstSet = descriptorsets[i];
        descriptor_writes[0].dstBinding = 0;
        descriptor_writes[0].dstArrayElement = 0;
        descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_writes[0].descriptorCount = 1;
        descriptor_writes[0].pBufferInfo = &buffer_info;
        descriptor_writes[0].pImageInfo = nullptr;
        descriptor_writes[0].pTexelBufferView = nullptr;
        
        //const auto image_info = uniform_images[i];
        descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[1].dstSet = descriptorsets[i];
        descriptor_writes[1].dstBinding = 1;
        descriptor_writes[1].dstArrayElement = 0;
        descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptor_writes[1].descriptorCount = 1;
        descriptor_writes[1].pBufferInfo = nullptr;
        descriptor_writes[1].pImageInfo = &image_info;
        descriptor_writes[1].pTexelBufferView = nullptr;
    
        vkUpdateDescriptorSets(device.logical_device(),
                               static_cast<uint32_t>(descriptor_writes.size()),
                               descriptor_writes.data(),
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
        ViewPort viewport{};
        viewport.view = view;
        viewport.proj = proj;
        viewport.model = glm::rotate(glm::mat4(1.0f),
                                     time * 0.5f * glm::radians(90.0f),
                                     glm::vec3(0.0f, 0.0f, 1.0f));


        vkCmdBindDescriptorSets(command_buffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                pipeline.layout(),
                                0,
                                1,
                                &descriptorsets[flight_frame],
                                0,
                                nullptr);

        uniform_viewports[flight_frame]->set_uniform(&viewport);
        
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

