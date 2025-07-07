#include "render/descriptor/DescriptorBuilder.hpp"
#include "render/descriptor/DescriptorAllocator.hpp"

namespace mve {

    DescriptorBuilder::DescriptorBuilder(
        VkDevice device,
        DescriptorAllocator& allocator,
        VkDescriptorSetLayout layout)
        : device_(device)
        , allocator_(allocator)
        , layout_(layout)
    {
    }

    DescriptorBuilder& DescriptorBuilder::bindBuffer(
        uint32_t binding,
        VkDescriptorType type,
        const VkDescriptorBufferInfo& bufferInfo)
    {
        bufferInfos_.push_back(bufferInfo);

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstBinding = binding;
        write.descriptorCount = 1;
        write.descriptorType = type;
        write.pBufferInfo = &bufferInfos_.back();

        writes_.push_back(write);
        return *this;
    }

    DescriptorBuilder& DescriptorBuilder::bindImage(
        uint32_t binding,
        VkDescriptorType type,
        const VkDescriptorImageInfo& imageInfo)
    {
        imageInfos_.push_back(imageInfo);

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstBinding = binding;
        write.descriptorCount = 1;
        write.descriptorType = type;
        write.pImageInfo = &imageInfos_.back();

        writes_.push_back(write);
        return *this;
    }

    VkResult DescriptorBuilder::build(VkDescriptorSet& outSet)
    {
        // Allocate via the allocator (handles pools, recycling, etc.)
        VkResult result = allocator_.allocate(layout_, outSet);
        if (result != VK_SUCCESS)
            return result;

        // Patch each write to target the newly allocated set
        for (auto& w : writes_)
            w.dstSet = outSet;

        // Submit all updates in one call
        vkUpdateDescriptorSets(
            device_,
            static_cast<uint32_t>(writes_.size()), writes_.data(),
            0, nullptr
        );

        return VK_SUCCESS;
    }

} // namespace mve
