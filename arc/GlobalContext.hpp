#include "SDLVulkan.hpp"

#include "TypeTraits.hpp"

#include <memory>
#include <atomic>

namespace ArcGraphics {
    

struct GlobalContextInstance {
    GlobalContextInstance();
    ~GlobalContextInstance();
};
    
class GlobalContext : public IsNotLvalueCopyable, IsNotRvalueCopyable
{
public:
    static void Initialize();
    static void DeInitialize();
    static std::unique_ptr<GlobalContextInstance> instance;
};
    

} 
