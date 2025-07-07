#pragma once

#include "render/Device.hpp"
#include "tool/HelpersVulkan.hpp"
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace mve {

    class Image {
    public:
        // Constructs an image + view wrapper.
        // - device: your Device wrapper holding VkDevice & VMA allocator
        // - width,height: dimensions
        // - format: image format
        // - usage: VkImageUsageFlags (e.g. VK_IMAGE_USAGE_SAMPLED_BIT|VK_IMAGE_USAGE_TRANSFER_DST_BIT)
        // - aspectFlags: aspect (e.g. VK_IMAGE_ASPECT_COLOR_BIT)
        // - memoryUsage: VMA_MEMORY_USAGE_*
        // - mipLevels, arrayLayers: as needed
        // - flags: any VkImageCreateFlags
        Image(Device& device,
            uint32_t width,
            uint32_t height,
            VkFormat format,
            VkImageUsageFlags usage,
            VkImageAspectFlags aspectFlags,
            VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY,
            uint32_t       mipLevels = 1,
            uint32_t       arrayLayers = 1,
            VkImageCreateFlags flags = 0);

        ~Image();

        // Move-only
        Image(Image&& other) noexcept;
        Image& operator=(Image&& other) noexcept;

        VkImage     getImage()    const { return image_; }
        VkImageView getView()     const { return view_; }
        VkFormat    getFormat()   const { return format_; }
        uint32_t    getMipLevels()const { return mipLevels_; }

    private:
        Device* device_;
        VmaAllocator       allocator_;
        VkImage            image_{ VK_NULL_HANDLE };
        VmaAllocation      allocation_{ nullptr };
        VkImageView        view_{ VK_NULL_HANDLE };
        VkFormat           format_;
        uint32_t           mipLevels_;
        VkImageLayout      currentLayout_{ VK_IMAGE_LAYOUT_UNDEFINED };

        void createImage(uint32_t width, uint32_t height,
            VkFormat format, VkImageUsageFlags usage,
            VmaMemoryUsage memoryUsage,
            uint32_t mipLevels, uint32_t arrayLayers,
            VkImageCreateFlags flags);

        void createImageView(VkFormat format,
            VkImageAspectFlags aspectFlags,
            uint32_t mipLevels, uint32_t arrayLayers);

        void cleanup();
    };

} // namespace mve
