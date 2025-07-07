#include <vkformat_enum.h>
#include <gl_format.h>
#include <ktx.h>
#include <stb_image.h>
#include <stb_image_resize2.h>
#include <iostream>

#include "tool/HelpersKTX.hpp"
#include "tool/HelpersSTBI.hpp"
#include "math/MipMap.hpp"

namespace mve {
    bool CompressTextureKTX2(const char* inputFilePath, const char* outputFilePath) {
        int origW, origH;
        VkFormat channelFormat;
        stbir_pixel_layout pixelLayout;
        ktx_transcode_fmt_e transcodeFmt;
        uint8_t* pixels;
        int numChannel = GetNumChannel(inputFilePath);

        pixels = stbi_load(inputFilePath, &origW, &origH, nullptr, numChannel == 3 ? 4 : numChannel);
        if (!pixels) {
            fprintf(stderr, "Failed to load image %s\n", inputFilePath);
            return false;
        }

        // Set format
        switch (numChannel) {
        case 1:
            std::cout << "Compressing R channel image " << inputFilePath << " to BC4 ktx2" << std::endl;
            channelFormat = VK_FORMAT_R8_UNORM;
            pixelLayout = STBIR_1CHANNEL;
            transcodeFmt = KTX_TTF_BC4_R;
            break;

        case 2:
            std::cout << "Compressing RG channel image " << inputFilePath << " to BC5 ktx2" << std::endl;
            channelFormat = VK_FORMAT_R8G8_UNORM;
            pixelLayout = STBIR_2CHANNEL;
            transcodeFmt = KTX_TTF_BC5_RG;
            break;

        case 3:
            std::cout << "Converting RGB image " << inputFilePath << " to RGBA and compressing to BC7 ktx2" << std::endl;
            channelFormat = VK_FORMAT_R8G8B8A8_UNORM;
            pixelLayout = STBIR_RGBA;
            transcodeFmt = KTX_TTF_BC7_RGBA;
            break;

        case 4:
            std::cout << "Compressing RGBA channel image " << inputFilePath << " to BC7 ktx2" << std::endl;
            channelFormat = VK_FORMAT_R8G8B8A8_UNORM;
            pixelLayout = STBIR_RGBA;
            transcodeFmt = KTX_TTF_BC7_RGBA;
            break;

        default:
            fprintf(stderr, "Invalid channel number for image %s\n", inputFilePath);
            return false;
        }

        // Create a KTX2 texture for Basis compression
        const uint32_t numMipLevels = calcNumMipLevels(origW, origH);
        ktxTextureCreateInfo createInfo2 = {};
        createInfo2.vkFormat = channelFormat;
        createInfo2.baseWidth = origW;
        createInfo2.baseHeight = origH;
        createInfo2.baseDepth = 1;
        createInfo2.numDimensions = 2;
        createInfo2.numLevels = numMipLevels;
        createInfo2.numLayers = 1;
        createInfo2.numFaces = 1;
        createInfo2.generateMipmaps = KTX_FALSE;
        
        ktxTexture2* texture = nullptr;
        KTX_error_code result = ktxTexture2_Create(&createInfo2, KTX_TEXTURE_CREATE_ALLOC_STORAGE, &texture);
        if (result != KTX_SUCCESS) {
            fprintf(stderr, "ktxTexture2_Create failed (code %d)\n", result);
            stbi_image_free(pixels);
            return false;
        }

        // Upload and resize into each mip level
        int w = origW, h = origH;
        ktxTexture* base = reinterpret_cast<ktxTexture*>(texture);

        for (uint32_t level = 0; level < numMipLevels; ++level) {
            ktx_size_t offset;
            ktxTexture2_GetImageOffset(texture, level, 0, 0, &offset);

            uint8_t* dst = ktxTexture_GetData(base) + offset;
            stbir_resize_uint8_linear(pixels, origW, origH, 0, dst, w, h, 0, pixelLayout);

            h = h > 1 ? h >> 1 : 1;
            w = w > 1 ? w >> 1 : 1;
        }

        // Compress to Basis
        result = ktxTexture2_CompressBasis(texture, 255);
        if (result != KTX_SUCCESS) {
            fprintf(stderr, "ktxTexture2_CompressBasis failed (code %d)\n", result);
            ktxTexture_Destroy(base);
            stbi_image_free(pixels);
            return false;
        }

        // Transcode Basis to BC7
        result = ktxTexture2_TranscodeBasis(texture, transcodeFmt, 0);
        if (result != KTX_SUCCESS) {
            fprintf(stderr, "ktxTexture2_TranscodeBasis failed (code %d)\n", result);
            ktxTexture_Destroy(base);
            stbi_image_free(pixels);
            return false;
        }

        // Write out as .ktx2
        result = ktxTexture2_WriteToNamedFile(texture, outputFilePath);
        if (result != KTX_SUCCESS) {
            fprintf(stderr, "ktxTexture2_WriteToNamedFile failed (code %d)\n", result);
            ktxTexture_Destroy(base);
            stbi_image_free(pixels);
            return false;
        }

        // Cleanup
        ktxTexture_Destroy(base);
        stbi_image_free(pixels);

        std::cout << "Image compression to " << outputFilePath << " succeed" << std::endl;
        return true;
    }
}