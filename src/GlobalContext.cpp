#include "../arc/GlobalContext.hpp"

namespace ArcGraphics {
    
std::unique_ptr<GlobalContextInstance> GlobalContext::instance = nullptr;
    
GlobalContextInstance::GlobalContextInstance()
{
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Vulkan_LoadLibrary(nullptr);
}

GlobalContextInstance::~GlobalContextInstance()
{
    SDL_Vulkan_UnloadLibrary();
    SDL_Quit();
}

void GlobalContext::Initialize() 
{
    if (!instance) instance = std::make_unique<GlobalContextInstance>();
} 

void GlobalContext::DeInitialize()
{
    if (instance) instance.reset();
}

}
