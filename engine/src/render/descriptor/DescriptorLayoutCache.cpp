#include "render/descriptor/DescriptorLayoutCache.hpp"
#include "tool/HelpersVulkan.hpp"
#include <algorithm>

namespace mve {

    DescriptorLayoutCache::DescriptorLayoutCache(VkDevice device)
        : device_(device) {
    }

    DescriptorLayoutCache::~DescriptorLayoutCache() {
        destroyAll();
    }

    VkDescriptorSetLayout DescriptorLayoutCache::createOrGetLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindingsInput) {
        LayoutInfo info;
        info.bindings = bindingsInput;

        // Ensure stable ordering for hashing
        std::sort(info.bindings.begin(), info.bindings.end(),
            [](const VkDescriptorSetLayoutBinding& a, const VkDescriptorSetLayoutBinding& b) {
                return a.binding < b.binding;
            });

        auto it = cache_.find(info);
        if (it != cache_.end())
            return it->second;

        VkDescriptorSetLayoutCreateInfo ci{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
        ci.bindingCount = static_cast<uint32_t>(info.bindings.size());
        ci.pBindings = info.bindings.data();

        VkDescriptorSetLayout layout;
        VK_CHECK(vkCreateDescriptorSetLayout(device_, &ci, nullptr, &layout));
        cache_[info] = layout;
        return layout;
    }

    void DescriptorLayoutCache::destroyAll() {
        for (auto& [_, layout] : cache_) {
            vkDestroyDescriptorSetLayout(device_, layout, nullptr);
        }
        cache_.clear();
    }

    bool DescriptorLayoutCache::LayoutInfo::operator==(const LayoutInfo& other) const {
        if (bindings.size() != other.bindings.size()) return false;
        for (size_t i = 0; i < bindings.size(); ++i) {
            const auto& a = bindings[i];
            const auto& b = other.bindings[i];
            if (a.binding != b.binding || a.descriptorType != b.descriptorType ||
                a.descriptorCount != b.descriptorCount || a.stageFlags != b.stageFlags) {
                return false;
            }
        }
        return true;
    }

    size_t DescriptorLayoutCache::LayoutInfo::hash() const {
        size_t result = bindings.size();
        for (const auto& b : bindings) {
            size_t h = b.binding ^ (b.descriptorType << 8) ^
                (b.descriptorCount << 16) ^ (b.stageFlags << 24);
            result ^= std::hash<size_t>()(h);
        }
        return result;
    }

} // namespace mve
