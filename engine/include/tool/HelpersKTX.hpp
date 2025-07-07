#pragma once
#define CHANNEL_RGB 3
#define CHANNEL_RGBA 4

namespace mve {
    // Only support R, RG, RGB, RGBA image as input
    bool CompressTextureKTX2(const char* inputFilePath, const char* outputFilePath);
}