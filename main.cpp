#include <FrameGraph/Graph.h>
#include <RHI/VulkanRuntime/RenderPass.h>
#include <RHI/RuntimeEntry.h>

int main()
{
    RuntimeEntry runtimeEntry(RuntimeEntry::Type::VULKAN);
    auto rhiRuntime = runtimeEntry.Create();

    auto graph = Graph::ParseRenderPassJson("C:/Users/Mario/Desktop/Pixel/deferred.json");
    auto renderpass = VulkanRenderPass(graph);
    renderpass.Build();
    return 0;
}