#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
//#include <SDL2/SDL_events.h>
//#include <SDL2/SDL_syswm.h>
#include <vulkan/vulkan.h>

#ifdef _WIN32
    #pragma comment(linker, "/subsystem:windows")
    #define VK_USE_PLATFORM_WIN32_KHR
    #define PLATFORM_SURFACE_EXTENSION_NAME VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#endif

#include <memory>
#include <atomic>
#include <vector>
#include <string>

namespace arc {

    
class GraphicsContext {
public:
    using ValidationLayers = std::vector<const char*>;
    using DeviceExtensions = std::vector<const char*>;
    GraphicsContext() = default;
    ~GraphicsContext();
    GraphicsContext(GraphicsContext&&) = delete;
    GraphicsContext(const GraphicsContext&) = delete;
    GraphicsContext& operator=(GraphicsContext&&) = delete;
    GraphicsContext& operator=(const GraphicsContext&) = delete;

    [[nodiscard]]
    static std::unique_ptr<GraphicsContext> create(uint32_t width,
                                                   uint32_t height,
                                                   const ValidationLayers validation_layers,
                                                   const DeviceExtensions device_extensions
                                                   );

private:
    VkInstance m_instance;
    VkPhysicalDevice m_physical_device{VK_NULL_HANDLE};
    VkDevice m_logical_device{VK_NULL_HANDLE};
    VkQueue m_graphics_queue;
    SDL_Window* m_window{nullptr};
    VkSurfaceKHR m_window_surface;
};


}
