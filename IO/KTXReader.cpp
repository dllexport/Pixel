#include <IO/KTXReader.h>

#include <Engine/PixelEngine.h>

#include <ktx.h>

IntrusivePtr<Texture> KTXReader::ReadFile(PixelEngine *engine, std::string file)
{
    auto rhiRuntime = engine->GetRHIRuntime();

    ktxResult result;
    ktxTexture *ktxTexture;
    result = ktxTexture_CreateFromNamedFile(file.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktxTexture);
    auto config = Texture::Configuration();
    config.mipLevels = ktxTexture->numLevels;
    config.arrayLayers = ktxTexture->numLayers;
    auto ktxTextureData = ktxTexture_GetData(ktxTexture);
    auto ktxTextureSize = ktxTexture_GetDataSize(ktxTexture);

    auto uploadBuffer = rhiRuntime->CreateBuffer(Buffer::BUFFER_USAGE_TRANSFER_SRC_BIT, MemoryProperty::MEMORY_PROPERTY_HOST_VISIBLE_BIT | MemoryProperty::MEMORY_PROPERTY_HOST_COHERENT_BIT, ktxTextureSize);
    memcpy(uploadBuffer->Map(), ktxTextureData, ktxTextureSize);

    auto texture = rhiRuntime->CreateTexture(TextureFormat::FORMAT_R8G8B8A8_UNORM, Texture::IMAGE_USAGE_TRANSFER_DST_BIT | Texture::IMAGE_USAGE_SAMPLED_BIT, MemoryProperty::MEMORY_PROPERTY_DEVICE_LOCAL_BIT, {ktxTexture->baseWidth, ktxTexture->baseHeight, 1}, config);

    AuxiliaryExecutor::TransferConfig transferConfigs;
    uint32_t offset = 0;
    for (uint32_t i = 0; i < ktxTexture->numLevels; i++)
    {
        // Calculate offset into staging buffer for the current mip level
        ktx_size_t offset;
        KTX_error_code ret = ktxTexture_GetImageOffset(ktxTexture, i, 0, 0, &offset);
        assert(ret == KTX_SUCCESS);
        transferConfigs.mipmapBufferLevelOffsets.push_back(offset);
    }

    engine->GetAuxiliaryExecutor()->TransferResource(texture, uploadBuffer, transferConfigs);
    
    return texture;
}
