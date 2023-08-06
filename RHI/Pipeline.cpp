#include <RHI/Pipeline.h>

Pipeline::Pipeline(PipelineStates pipelineStates) : pipelineStates(pipelineStates)
{
}

IntrusivePtr<RenderPass> Pipeline::GetRenderPass()
{
    return renderPass;
}
