#include "math/MipMap.hpp"

namespace mve {
    uint32_t calcNumMipLevels(uint32_t width, uint32_t height) {
        uint32_t levels = 0;
        uint32_t size = std::max(width, height);
        while (size > 0) {
            ++levels;
            size >>= 1;
        }
        return levels;
    }
}