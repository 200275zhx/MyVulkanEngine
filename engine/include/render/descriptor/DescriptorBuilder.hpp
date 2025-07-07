#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace mve {

    class DescriptorAllocator;

    class DescriptorBuilder {
    public:
        DescriptorBuilder(
            VkDevice device,
            DescriptorAllocator& allocator,
            VkDescriptorSetLayout layout
        );

        DescriptorBuilder& bindBuffer(
            uint32_t binding,
            VkDescriptorType type,
            const VkDescriptorBufferInfo& bufferInfo
        );

        DescriptorBuilder& bindImage(
            uint32_t binding,
            VkDescriptorType type,
            const VkDescriptorImageInfo& imageInfo
        );

        // Allocates + updates the set
        VkResult build(VkDescriptorSet& outSet);

    private:
        VkDevice device_;
        DescriptorAllocator& allocator_;
        VkDescriptorSetLayout layout_;
        std::vector<VkWriteDescriptorSet>      writes_;
        std::vector<VkDescriptorBufferInfo>    bufferInfos_;
        std::vector<VkDescriptorImageInfo>     imageInfos_;
    };

} // namespace mve
