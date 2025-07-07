#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <unordered_map>

namespace mve {

    class DescriptorLayoutCache {
    public:
        explicit DescriptorLayoutCache(VkDevice device);
        ~DescriptorLayoutCache();

        DescriptorLayoutCache(const DescriptorLayoutCache&) = delete;
        DescriptorLayoutCache& operator=(const DescriptorLayoutCache&) = delete;

        VkDescriptorSetLayout createOrGetLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings);

    private:
        struct LayoutInfo {
            std::vector<VkDescriptorSetLayoutBinding> bindings;

            bool operator==(const LayoutInfo& other) const;
            size_t hash() const;
        };

        struct LayoutInfoHasher {
            size_t operator()(const LayoutInfo& info) const {
                return info.hash();
            }
        };

        VkDevice device_;
        std::unordered_map<LayoutInfo, VkDescriptorSetLayout, LayoutInfoHasher> cache_;

        void destroyAll();
    };

} // namespace mve
