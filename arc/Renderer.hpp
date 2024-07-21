#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.h>

#include "Device.hpp"

#include <vector>

namespace ArcGraphics {

class Renderer
{
public:
    class Builder;

    Renderer();
    ~Renderer();

private:
    VkQueue m_graphics_queue;
    VkSwapchainKHR m_swap_chain;
    VkSurfaceFormatKHR m_swap_chain_surface_format;
    std::vector<VkImageView> m_swap_chain_image_views;
    VkRenderPass m_render_pass;
    VkDescriptorPool m_descriptor_pool;
    VkDescriptorSetLayout m_descriptor_layout;
    std::vector<VkDescriptorSet> m_descriptor_sets;
    VkPipelineLayout m_graphics_pipeline_layout;
    VkPipeline m_graphics_pipeline;
    std::vector<VkFramebuffer> m_swap_chain_framebuffers;
    bool m_swap_chain_framebuffer_resized{false};
    VkCommandPool m_command_pool;
};
    
class Renderer::Builder 
{
 public:
    Builder(const Device* device);
    ~Builder();

    [[nodiscard]]
    Renderer produce();
    void reset_builder();

    Builder& with_wanted_window_size(const uint32_t width,
                                     const uint32_t height);
    Builder& with_window_name(const std::string& name);
    Builder& with_window_flags(const uint32_t flags);

 private:
    const Device* m_device;
    std::string m_window_name;
    uint32_t m_window_width;
    uint32_t m_window_height;
    uint32_t m_window_flags;
};
    
}
