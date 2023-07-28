#include <FrameGraph/Graph.h>
#include <RHI/VulkanRuntime/RenderPass.h>

int main()
{
    auto graph = Graph::ParseRenderPassJson("C:/Users/Mario/Desktop/Pixel/deferred.json");
    auto renderpass = VulkanRenderPass(graph);
    renderpass.Build();
    return 0;
}