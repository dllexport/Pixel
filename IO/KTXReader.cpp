#include <IO/KTXReader.h>

#include <ktx.h>

IntrusivePtr<Texture> KTXReader::ReadFile(RHIRuntime *rhiRuntime, std::string file)
{
    ktxResult result;
    ktxTexture *ktxTexture;
    result = ktxTexture_CreateFromNamedFile(file.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktxTexture);
    auto config = Texture::Configuration();
    auto texture = rhiRuntime->CreateTexture(TextureFormat::FORMAT_R8G8B8A8_UNORM, Texture::IMAGE_USAGE_TRANSFER_DST_BIT | Texture::IMAGE_USAGE_SAMPLED_BIT, MemoryProperty::MEMORY_PROPERTY_DEVICE_LOCAL_BIT, {ktxTexture->baseWidth, ktxTexture->baseHeight}, config);
    return texture;
}
