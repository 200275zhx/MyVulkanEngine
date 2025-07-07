#include "render/Image.hpp"

namespace mve {

    Image::Image(Device& device,
        uint32_t width,
        uint32_t height,
        VkFormat format,
        VkImageUsageFlags usage,
        VkImageAspectFlags aspectFlags,
        VmaMemoryUsage memoryUsage,
        uint32_t mipLevels,
        uint32_t arrayLayers,
        VkImageCreateFlags flags)
        : device_(&device)
        , allocator_(device.getAllocator())
        , format_(format)
        , mipLevels_(mipLevels)
    {
        createImage(width, height, format, usage, memoryUsage, mipLevels, arrayLayers, flags);
        createImageView(format, aspectFlags, mipLevels, arrayLayers);
    }

    Image::~Image() {
        cleanup();
    }

    Image::Image(Image&& o) noexcept
        : device_(o.device_)
        , allocator_(o.allocator_)
        , image_(o.image_)
        , allocation_(o.allocation_)
        , view_(o.view_)
        , format_(o.format_)
        , mipLevels_(o.mipLevels_)
        , currentLayout_(o.currentLayout_)
    {
        o.image_ = VK_NULL_HANDLE;
        o.allocation_ = nullptr;
        o.view_ = VK_NULL_HANDLE;
    }

    Image& Image::operator=(Image&& o) noexcept {
        if (this != &o) {
            cleanup();
            device_ = o.device_;
            allocator_ = o.allocator_;
            image_ = o.image_;
            allocation_ = o.allocation_;
            view_ = o.view_;
            format_ = o.format_;
            mipLevels_ = o.mipLevels_;
            currentLayout_ = o.currentLayout_;

            o.image_ = VK_NULL_HANDLE;
            o.allocation_ = nullptr;
            o.view_ = VK_NULL_HANDLE;
        }
        return *this;
    }

    void Image::createImage(uint32_t width, uint32_t height,
        VkFormat format, VkImageUsageFlags usage,
        VmaMemoryUsage memoryUsage,
        uint32_t mipLevels, uint32_t arrayLayers,
        VkImageCreateFlags flags)
    {
        VkImageCreateInfo info{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
        info.flags = flags;
        info.imageType = VK_IMAGE_TYPE_2D;
        info.extent = { width, height, 1 };
        info.mipLevels = mipLevels;
        info.arrayLayers = arrayLayers;
        info.format = format;
        info.tiling = VK_IMAGE_TILING_OPTIMAL;
        info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        info.usage = usage;
        info.samples = VK_SAMPLE_COUNT_1_BIT;
        info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo allocInfo{};
        allocInfo.usage = memoryUsage;

        VK_CHECK(vmaCreateImage(allocator_, &info, &allocInfo, &image_, &allocation_, nullptr));
        device_->setObjectName(VK_OBJECT_TYPE_IMAGE, uint64_t(image_), "Image");
    }

    void Image::createImageView(VkFormat format,
        VkImageAspectFlags aspectFlags,
        uint32_t mipLevels, uint32_t arrayLayers)
    {
        VkImageViewCreateInfo viewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
        viewInfo.image = image_;
        viewInfo.viewType = (arrayLayers > 1)
            ? VK_IMAGE_VIEW_TYPE_2D_ARRAY
            : VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.components = {
            VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY
        };
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = mipLevels;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = arrayLayers;

        VK_CHECK(vkCreateImageView(device_->getDevice(), &viewInfo, nullptr, &view_));
        device_->setObjectName(VK_OBJECT_TYPE_IMAGE_VIEW, uint64_t(view_), "ImageView");
    }

    void Image::cleanup() {
        if (view_ != VK_NULL_HANDLE) {
            vkDestroyImageView(device_->getDevice(), view_, nullptr);
            view_ = VK_NULL_HANDLE;
        }
        if (image_ != VK_NULL_HANDLE) {
            vmaDestroyImage(allocator_, image_, allocation_);
            image_ = VK_NULL_HANDLE;
            allocation_ = nullptr;
        }
    }

} // namespace mve
