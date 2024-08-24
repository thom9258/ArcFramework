#include "SDLVulkan.hpp"
/*********************************************************************
 * @file GlobalContext.cpp
 * @brief The Global Vulkan and SDL Context manager.
 * @copyright
 * Copyright 2024 Thomas Alexgaard Jensen
 *********************************************************************/

#include "TypeTraits.hpp"

#include <memory>
#include <atomic>

namespace ArcGraphics {
    
/**
* @brief The globally-available context for the vulkan and SDL API's.
* @note This class *must* be the first one created.
*/
class GlobalContext 
    : public IsNotLvalueCopyable
    , public IsNotRvalueCopyable
{
    /**
    * @brief Construct the global context of Vulkan and SDL.
    */
    GlobalContext();

    /**
    * @brief Destroy the global context of Vulkan and SDL.
    * @note Destruction will only happen once the process is stopped. 
    */
    ~GlobalContext();
public:

    /**
    * @brief Initialize the singleton context.
    * The initialization is handled internally by the Device class when it is created.
    * @see ArcGraphics::Device
    */
    static void Initialize();

private:
    static std::unique_ptr<GlobalContext> m_context_instance; /// The singleton instance
};

} 
