#include <FrameGraph/Graph.h>
#include <RHI/RuntimeEntry.h>

#include <Engine/PixelEngine.h>
#include <Engine/Renderer.h>
#include <Engine/Camera.h>

#include <Engine/ImguiOverlay.h>

#include <glm/glm.hpp>
#include <spdlog/spdlog.h>

int main()
{
    spdlog::set_level(spdlog::level::debug);

    auto graph = Graph::ParseRenderPassJson("/Users/mario/Documents/GitHub/Pixel/Test/FrameGraph/mutipleRenderPass.json");
    graph->Topo();

    return 0;
}