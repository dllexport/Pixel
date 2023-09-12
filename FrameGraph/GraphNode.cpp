#include <FrameGraph/GraphNode.h>

GraphNode::GraphNode(std::string name, Type type) : name(name), type(type) {}

void GraphNode::AddInput(GraphNode *from)
{
    inputs.push_back(from);
    from->outputs.push_back(this);
}

void GraphNode::AddOutput(GraphNode *to)
{
    outputs.push_back(to);
    to->inputs.push_back(this);
}

// get all outputs of outputs, filtered by type
std::vector<IntrusivePtr<GraphNode>> GraphNode::TraceAllOutputs(Type type, uint32_t traceLevel)
{
    if (traceLevel == 0)
    {
        return {};
    }

    std::vector<IntrusivePtr<GraphNode>> result;
    for (auto &output : outputs)
    {
        if (output->type == type)
        {
            result.push_back(output);
        }

        for (auto output2 : output->outputs)
        {
            auto nestedResult = output2->TraceAllOutputs(type, traceLevel - 1);
            result.insert(result.end(), nestedResult.begin(), nestedResult.end());
        }
    }
    return result;
}
