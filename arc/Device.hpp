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

/**
 * @brief Devices & Context object.
 * A small collection of global objects, used almost everywhere in the renderer.
 * @todo restructure this class so that it is more explicitly called Devices,
 * and access is then just logical() or physical().
 * @todo VkInstance is a global instance thing, remove it and make it a singleton,
 * this would essencially allow having multiple Devices.
 */
class Device : private IsNotLvalueCopyable
{
public:
    class Builder;

    Device(const VkInstance instance,
           const VkPhysicalDevice physical_device,
           const VkDevice logical_device,
           const DeviceRenderingCapabilities capabilities);

    void destroy();

    ~Device() = default;

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
    
/**
 * @brief Device Builder.
 * The builder used to create the device context.
 * Facilitates adding different configurations such as validation layers.
 * @see ArcGraphics::Device
 *
 * @todo make it so a specified physical device can be forced to be used, instead
 * of always using internal algorithms to find the best one.
 * @todo make it so a a custom scoring algorithm for devices can be supplied.
 */
class Device::Builder : protected IsNotLvalueCopyable
{
public:
    Builder();
    ~Builder() = default;

    Builder& add_validation_layers(const ValidationLayers layers);
    Builder& add_khronos_validation_layer();

    Device produce();
    
private:
    ValidationLayers m_validation_layers{};
};
 

} 
