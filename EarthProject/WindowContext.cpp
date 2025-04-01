#include "WindowContext.h"

std::shared_ptr<WindowContext> WindowContext::GetInstance() {
    static std::shared_ptr<WindowContext> instance(new WindowContext());
    return instance;
}
