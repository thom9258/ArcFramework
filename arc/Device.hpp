#pragma once
/** *******************************************************************
 * @file Device.hpp
 * @brief The Device Group needed to do anything in Vulkan.
 * @copyright
 * Copyright 2024 Thomas Alexgaard Jensen
 *********************************************************************/

#include "GlobalContext.hpp"
#include "Algorithm.hpp"

#include <vector>
#include <optional>

namespace ArcGraphics {

/**
 * @brief Devices & Vulkan Instance Manager.
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

    /**
     * @brief Destroy the devices and vulkan instance.
     */
    void destroy();

    ~Device() = default;

    /**
     * @brief Get the Vulkan Instance.
     */
    [[nodiscard]]
    const VkInstance& instance() const noexcept;

    /**
     * @brief Get the physical device.
     */
    [[nodiscard]]
    const VkPhysicalDevice& physical_device() const noexcept;

    /**
     * @brief Get the logical device.
     */
    [[nodiscard]]
    const VkDevice& logical_device() const noexcept;

    /**
     * @brief Get the rendering capabilities of the physical device.
     */
    [[nodiscard]]
    const DeviceRenderingCapabilities& capabilities() const noexcept;

private:
     /**
     * @brief Construct the Devices.
     * @note is internally handled by the builder.
     * @see ArcGraphics::Device::Builder
     */
    Device(const VkInstance instance,
           const VkPhysicalDevice physical_device,
           const VkDevice logical_device,
           const DeviceRenderingCapabilities capabilities);

    VkInstance m_instance;                      /// Vulkan instance
    VkPhysicalDevice m_physical_device;         /// physical device
    VkDevice m_logical_device;                  /// logical device
    DeviceRenderingCapabilities m_capabilities; /// rendering capabilities
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

    /**
     * @brief Builder Constructor ensures that the global context is initialized.
     * @see ArcGraphics::GlobalContext::Initialize
     */
    Builder();
    ~Builder() = default;

    /**
     * @brief Add a validation layer to the logical device for debugging.
     */
    Builder& add_validation_layers(const ValidationLayers layers);

    /**
     * @brief Add the khronos validation layer to the logical device for debugging.
     */
    Builder& add_khronos_validation_layer();

    /**
     * @brief Produce the Device.
     */
    Device produce();
    
private:
    ValidationLayers m_validation_layers{}; /// The validation layers to be enabled
};
 

} 
