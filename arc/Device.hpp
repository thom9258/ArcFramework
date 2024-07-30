#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.h>

#include "TypeTraits.hpp"
#include "Algorithm.hpp"
#include "GlobalContext.hpp"

#include <vector>
#include <optional>

namespace ArcGraphics {
    
    
class Device : protected IsNotLvalueCopyable
{
public:
    class Builder;

    Device(const VkInstance instance,
           const VkPhysicalDevice physical_device,
           const VkDevice logical_device,
           const DeviceRenderingCapabilities capabilities);

    ~Device();

    [[nodiscard]]
    const VkInstance& instance() const noexcept;

    [[nodiscard]]
    const VkPhysicalDevice& physical_device() const noexcept;

    [[nodiscard]]
    const VkDevice& logical_device() const noexcept;

    [[nodiscard]]
    const DeviceRenderingCapabilities& capabilities() const noexcept;

private:
    VkInstance m_instance;
    VkPhysicalDevice m_physical_device;
    VkDevice m_logical_device;
    DeviceRenderingCapabilities m_capabilities;
};
    
class Device::Builder : protected IsNotLvalueCopyable
{
public:
    Builder();
    ~Builder();

    Builder& add_validation_layers(const ValidationLayers layers);
    Builder& add_khronos_validation_layer();

    std::shared_ptr<Device> produce();
    
private:
    void reset_builder();
    ValidationLayers m_validation_layers{};
};
 

} 
