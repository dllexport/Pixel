#include <RHI/RenderGroupExecutor.h>

void RenderGroupExecutor::SetSwapChain(IntrusivePtr<SwapChain> swapChain)
{
    this->swapChain = swapChain;
}