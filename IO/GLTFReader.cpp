#include <IO/GLTFReader.h>

#include <tiny_gltf.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <spdlog/spdlog.h>

#include <RHI/ResourceBindingState.h>

std::vector<GLTFModel> GLTFReader::ReadFile(PixelEngine *engine, std::string file)
{
    std::vector<GLTFModel> gltfModels;

    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    tinygltf::Model model;
    auto result = loader.LoadASCIIFromFile(&model, &err, &warn, file);
    if (!result)
    {
        spdlog::info("tinygltf read failed: {}", err);
        return {};
    }

    for (auto mesh : model.meshes)
    {
        GLTFModel gltfModel;
        auto &vertexBuffer = gltfModel.vertexBuffer;
        auto &indexBuffer = gltfModel.indexBuffer;

        // read mesh vertex and indices
        for (auto primitive : mesh.primitives)
        {
            if (primitive.indices == -1)
            {
                continue;
            }

            // save old vertex offset for indices offset
            uint32_t vertexOffset = vertexBuffer.size();
            uint32_t indexOffset = indexBuffer.size();
            uint32_t indexCount = 0;
            uint32_t vertexCount = 0;

            const float *bufferPos = nullptr;
            const float *bufferNormals = nullptr;
            const float *bufferTexCoords = nullptr;
            const float *bufferTangents = nullptr;
            uint32_t numColorComponents;
            const uint16_t *bufferJoints = nullptr;
            const float *bufferWeights = nullptr;

            glm::vec3 posMin{};
            glm::vec3 posMax{};
            // read POSITION
            {
                if (primitive.attributes.count("POSITION") == 0)
                    continue;

                auto accessor = model.accessors[primitive.attributes["POSITION"]];
                auto bufferView = model.bufferViews[accessor.bufferView];
                bufferPos = (float *)&model.buffers[bufferView.buffer].data[bufferView.byteOffset + accessor.byteOffset];

                posMin = glm::vec3(accessor.minValues[0], accessor.minValues[1], accessor.minValues[2]);
                posMax = glm::vec3(accessor.maxValues[0], accessor.maxValues[1], accessor.maxValues[2]);

                vertexCount = accessor.count;
            }

            // read NORMAL
            if (primitive.attributes.count("NORMAL"))
            {
                auto accessor = model.accessors[primitive.attributes["NORMAL"]];
                auto bufferView = model.bufferViews[accessor.bufferView];
                bufferNormals = (float *)&model.buffers[bufferView.buffer].data[bufferView.byteOffset + accessor.byteOffset];
            }

            // read TEXCOORD_0
            if (primitive.attributes.count("TEXCOORD_0"))
            {
                auto accessor = model.accessors[primitive.attributes["TEXCOORD_0"]];
                auto bufferView = model.bufferViews[accessor.bufferView];
                bufferTexCoords = (float *)&model.buffers[bufferView.buffer].data[bufferView.byteOffset + accessor.byteOffset];
            }

            if (primitive.attributes.count("TANGENT"))
            {
                auto accessor = model.accessors[primitive.attributes["TANGENT"]];
                auto bufferView = model.bufferViews[accessor.bufferView];
                bufferTangents = (float *)&model.buffers[bufferView.buffer].data[bufferView.byteOffset + accessor.byteOffset];
            }

            // read skinning joints
            if (primitive.attributes.count("JOINTS_0"))
            {
                auto accessor = model.accessors[primitive.attributes["JOINTS_0"]];
                auto bufferView = model.bufferViews[accessor.bufferView];
                bufferJoints = (uint16_t *)&model.buffers[bufferView.buffer].data[bufferView.byteOffset + accessor.byteOffset];
            }

            if (primitive.attributes.count("WEIGHTS_0"))
            {
                auto accessor = model.accessors[primitive.attributes["WEIGHTS_0"]];
                auto bufferView = model.bufferViews[accessor.bufferView];
                bufferWeights = (float *)&model.buffers[bufferView.buffer].data[bufferView.byteOffset + accessor.byteOffset];
            }

            auto hasSkin = (bufferJoints && bufferWeights);

            for (uint32_t v = 0; v < vertexCount; v++)
            {
                GLTFVertex vert = {};
                vert.pos = glm::vec4(glm::make_vec3(&bufferPos[v * 3]), 1.0f);
                vert.normal = glm::normalize(glm::vec3(bufferNormals ? glm::make_vec3(&bufferNormals[v * 3]) : glm::vec3(0.0f)));
                vert.uv = bufferTexCoords ? glm::make_vec2(&bufferTexCoords[v * 2]) : glm::vec3(0.0f);
                vert.tangent = bufferTangents ? glm::vec4(glm::make_vec4(&bufferTangents[v * 4])) : glm::vec4(0.0f);
                vert.joint0 = hasSkin ? glm::vec4(glm::make_vec4(&bufferJoints[v * 4])) : glm::vec4(0.0f);
                vert.weight0 = hasSkin ? glm::make_vec4(&bufferWeights[v * 4]) : glm::vec4(0.0f);
                vertexBuffer.push_back(vert);
            }

            spdlog::info("{}", vertexBuffer.size());

            {
                // read indices
                auto &accessor = model.accessors[primitive.indices];
                auto &bufferView = model.bufferViews[accessor.bufferView];
                auto &buffer = model.buffers[bufferView.buffer];

                indexCount = accessor.count;

                switch (accessor.componentType)
                {
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
                {
                    uint32_t *buf = new uint32_t[accessor.count];
                    memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(uint32_t));
                    for (size_t index = 0; index < accessor.count; index++)
                    {
                        indexBuffer.push_back(buf[index] + vertexOffset);
                    }
                    delete[] buf;
                    break;
                }
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
                {
                    uint16_t *buf = new uint16_t[accessor.count];
                    memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(uint16_t));
                    for (size_t index = 0; index < accessor.count; index++)
                    {
                        indexBuffer.push_back(buf[index] + vertexOffset);
                    }
                    delete[] buf;
                    break;
                }
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
                {
                    uint8_t *buf = new uint8_t[accessor.count];
                    memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(uint8_t));
                    for (size_t index = 0; index < accessor.count; index++)
                    {
                        indexBuffer.push_back(buf[index] + vertexOffset);
                    }
                    delete[] buf;
                    break;
                }
                default:
                    throw;
                }
            }

            gltfModel.primitives.push_back(
                {.firstVertex = vertexOffset,
                 .vertexCount = vertexCount,
                 .firstIndex = indexOffset,
                 .indexCount = indexCount});
        }

        gltfModels.push_back(gltfModel);
    }

    return gltfModels;
}
