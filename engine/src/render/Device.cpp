#include "render/Device.hpp"
#include "tool/HelpersStd.hpp"
#include <cassert>

namespace mve {
    Device::Device(VkInstance instance,
        VkSurfaceKHR surface,
        const std::vector<const char*>& deviceExtensions,
        const std::vector<const char*>& deviceFeatures)
        : surface_(surface)
    {
        insertUniqueCStrings(device_extensions_, deviceExtensions);
        insertUniqueCStrings(device_features_, deviceFeatures);

        pickPhysicalDevice(instance);

        // cache memory properties
        vkGetPhysicalDeviceMemoryProperties(physical_device_, &mem_props_);
        // find all queue families (graphics/present/compute/transfer)
        queueIndices_ = FindQueueFamilyIndices(physical_device_, surface_);

        createLogicalDevice();
        initializeVMA(instance);

        // retrieve queues and create default command pools
        vkGetDeviceQueue(device_, queueIndices_.graphicsFamilyIndex, 0, &graphics_queue_);
        if (queueIndices_.presentFamilyFound) {
            vkGetDeviceQueue(device_, queueIndices_.presentFamilyIndex, 0, &present_queue_);
            graphics_command_pool_ = createCommandPool(
                queueIndices_.graphicsFamilyIndex, 
                VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | 
                VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
            );
        }
        if (queueIndices_.computeFamilyFound) {
            vkGetDeviceQueue(device_, queueIndices_.computeFamilyIndex, 0, &compute_queue_);
            compute_command_pool_ = createCommandPool(
                queueIndices_.computeFamilyIndex, 
                VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | 
                VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
            );
        }
        if (queueIndices_.transferFamilyFound) {
            vkGetDeviceQueue(device_, queueIndices_.transferFamilyIndex, 0, &transfer_queue_);
            transfer_command_pool_ = createCommandPool(
                queueIndices_.transferFamilyIndex, 
                VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | 
                VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
            );
            static_transfer_command_pool_ = createCommandPool(queueIndices_.transferFamilyIndex, 0);
        }
    }

    Device::~Device() {
        if (graphics_command_pool_ != VK_NULL_HANDLE) {
            vkDestroyCommandPool(device_, graphics_command_pool_, nullptr);
            graphics_command_pool_ = VK_NULL_HANDLE;
        }
        if (compute_command_pool_ != VK_NULL_HANDLE) {
            vkDestroyCommandPool(device_, compute_command_pool_, nullptr);
            compute_command_pool_ = VK_NULL_HANDLE;
        }
        if (transfer_command_pool_ != VK_NULL_HANDLE) {
            vkDestroyCommandPool(device_, transfer_command_pool_, nullptr);
            transfer_command_pool_ = VK_NULL_HANDLE;
        }
        if (static_transfer_command_pool_ != VK_NULL_HANDLE) {
            vkDestroyCommandPool(device_, static_transfer_command_pool_, nullptr);
            static_transfer_command_pool_ = VK_NULL_HANDLE;
        }
        vmaDestroyPool(allocator_, streaming_pool_);
        vmaDestroyPool(allocator_, static_resource_pool_);
        vmaDestroyPool(allocator_, staging_pool_);
        vmaDestroyAllocator(allocator_);
        if (device_ != VK_NULL_HANDLE) {
            vkDestroyDevice(device_, nullptr);
            device_ = VK_NULL_HANDLE;
        }
    }

    void Device::pickPhysicalDevice(VkInstance instance) {
        std::vector<VkPhysicalDevice> devices = EnumeratePhysicalDevices(instance);
        int bestScore = -1;
        for (auto& pd : devices) {
            // check required device-level extensions
            std::set<std::string> extSet(device_extensions_.begin(), device_extensions_.end());
            if (!CheckExtensionSupport(pd, extSet)) continue;
            // check feature support by name:
            std::set<std::string> featuresSet(device_features_.begin(), device_features_.end());
            if (!CheckFeatureSupport(pd, featuresSet)) continue;
            // check graphics + present queue family exist
            if (!CheckQueueFamilyMinimalSupport(pd, surface_)) continue;

            int score = ScorePhysicalDevice(pd);
            if (score > bestScore) {
                bestScore = score;
                physical_device_ = pd;
            }
        }
        if (bestScore < 0) throw std::runtime_error("Failed to find a suitable physical device");

        available_features_ = QueryAllFeatures(physical_device_);
    }

    void Device::createLogicalDevice() {
        // gather unique queue families
        std::set<uint32_t> uniqueFamilies = {
            queueIndices_.graphicsFamilyIndex
        };
        if (queueIndices_.presentFamilyFound)
            uniqueFamilies.insert(queueIndices_.presentFamilyIndex);
        if (queueIndices_.computeFamilyFound)
            uniqueFamilies.insert(queueIndices_.computeFamilyIndex);
        if (queueIndices_.transferFamilyFound)
            uniqueFamilies.insert(queueIndices_.transferFamilyIndex);

        // create queue infos
        std::vector<VkDeviceQueueCreateInfo> queueInfos;
        float priority = 1.0f;
        for (auto idx : uniqueFamilies) {
            VkDeviceQueueCreateInfo qi{};
            qi.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            qi.queueFamilyIndex = idx;
            qi.queueCount = 1;
            qi.pQueuePriorities = &priority;
            queueInfos.push_back(qi);
        }

        // build feature chain
        AllFeatures reqChain = BuildRequiredFeatureChain(available_features_, device_features_);

        // prepare the memoryPriority extension struct for VMA_ALLOCATOR_CREATE_EXT_MEMORY_PRIORITY_BIT
        VkPhysicalDeviceMemoryPriorityFeaturesEXT memPrio{};
        memPrio.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PRIORITY_FEATURES_EXT;
        memPrio.pNext = &reqChain.core;             // chain it *before* your core req since extension features should before native features
        memPrio.memoryPriority = VK_TRUE;

        // logical-device create
        VkDeviceCreateInfo ci{};
        ci.sType                    = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        ci.pNext                    = &memPrio;
        ci.pEnabledFeatures         = nullptr;
        ci.queueCreateInfoCount     = (uint32_t)queueInfos.size();
        ci.pQueueCreateInfos        = queueInfos.data();
        ci.enabledExtensionCount    = (uint32_t)device_extensions_.size();
        ci.ppEnabledExtensionNames  = device_extensions_.data();

        VK_CHECK(vkCreateDevice(physical_device_, &ci, nullptr, &device_));

#ifdef VULKAN_PHYSICAL_DEVICE_INFO_CHECK
        VkPhysicalDeviceProperties props{};
        vkGetPhysicalDeviceProperties(physical_device_, &props);
        std::cout << "\n// ---------- Logical Device Creation Succeed ---------- //\n\n"
            << "physical device name: " << props.deviceName << "\nphysical device id: "
            << props.deviceID << std::endl;
#endif
    }

    void Device::initializeVMA(VkInstance instance) {
        // Create VMA allocator
        VmaAllocatorCreateInfo vmaInfo = {};
        vmaInfo.physicalDevice = physical_device_;
        vmaInfo.device = device_;
        vmaInfo.instance = instance;
        vmaInfo.vulkanApiVersion = VK_API_VERSION_1_4;
        vmaInfo.flags = VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT
            | VMA_ALLOCATOR_CREATE_KHR_BIND_MEMORY2_BIT
            | VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT                            // need enable VK_EXT_memory_budget
            | VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT                        // need enable the bufferDeviceAddress feature in VkPhysicalDeviceBufferDeviceAddressFeatures
            | VMA_ALLOCATOR_CREATE_EXT_MEMORY_PRIORITY_BIT;                         // need enable VK_EXT_memory_priority and enable the memoryPriority feature in VkPhysicalDeviceMemoryPriorityFeaturesEXT
        vmaCreateAllocator(&vmaInfo, &allocator_);

        // Create memory pools
        VmaPoolCreateInfo poolInfo = {};
        poolInfo.flags = 0;
        poolInfo.maxBlockCount = 1;

        // 1) Per-frame staging uploads (host-visible, linear = free-at-once + ring)
        poolInfo.memoryTypeIndex = findMemoryType(~0u, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        poolInfo.blockSize = 64ull * 1024 * 1024;                       // 64 MB blocks
        poolInfo.flags = VMA_POOL_CREATE_LINEAR_ALGORITHM_BIT;          // gives ring/stack/free-at-once
        vmaCreatePool(allocator_, &poolInfo, &staging_pool_);

        // 2) Static resources (device-local, default best-fit algorithm)
        poolInfo.memoryTypeIndex = findMemoryType(~0u, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        poolInfo.blockSize = 256ull * 1024 * 1024;                      // 256 MB blocks
        poolInfo.flags = 0;                                             // default algorithm (best-fit)
        vmaCreatePool(allocator_, &poolInfo, &static_resource_pool_);

        // 3) Dynamic/streaming data (device-local, freeing allocations in FIFO order)
        poolInfo.memoryTypeIndex = findMemoryType(~0u, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        poolInfo.blockSize = 128ull * 1024 * 1024;                      // 128 MB blocks
        poolInfo.flags = VMA_POOL_CREATE_LINEAR_ALGORITHM_BIT;          // reuse as ring buffer
        vmaCreatePool(allocator_, &poolInfo, &streaming_pool_);
    }

    uint32_t Device::findMemoryType(uint32_t typeBits, VkMemoryPropertyFlags props) const {
        for (uint32_t i = 0; i < mem_props_.memoryTypeCount; ++i) {
            if ((typeBits & (1u << i)) && (mem_props_.memoryTypes[i].propertyFlags & props) == props) return i;
        }
        throw std::runtime_error("Failed to find suitable memory type");
    }

    VkBuffer Device::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage) const {
        VkBuffer       buffer;
        VkBufferCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        info.pNext = nullptr;
        info.size = size;
        info.usage = usage;
        info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        // VK_CHECK_ENABLE: heavy-weight resource-creation calls you do at load time or during streaming, not every frame
        VK_CHECK(vkCreateBuffer(device_, &info, nullptr, &buffer));
        return buffer;
    }

    VkDeviceMemory Device::allocateMemory(VkBuffer buffer, VkMemoryPropertyFlags props) const {
        VkMemoryRequirements req;
        vkGetBufferMemoryRequirements(device_, buffer, &req);

        VkMemoryAllocateInfo ai{};
        ai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        ai.allocationSize = req.size;
        ai.memoryTypeIndex = findMemoryType(req.memoryTypeBits, props);

        VkDeviceMemory mem;
        // VK_CHECK_ENABLE: heavy-weight resource-creation calls you do at load time or during streaming, not every frame
        VK_CHECK(vkAllocateMemory(device_, &ai, nullptr, &mem));
        vkBindBufferMemory(device_, buffer, mem, 0);
        return mem;
    }

    BufferWithMemory Device::createBufferWithMemory(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags props) const {
        VkBuffer       buf = createBuffer(size, usage);
        VkDeviceMemory mem = allocateMemory(buf, props);
        return { buf, mem };
    }

    BufferWithAllocation Device::createVMABuffer(VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags allocFlags) const {
        VkBufferCreateInfo        bufInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        bufInfo.size = size;
        bufInfo.usage = usage;
        bufInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo allocInfo{};
        allocInfo.usage = memoryUsage;
        allocInfo.flags = allocFlags;

        BufferWithAllocation bufAlloc;
        // VK_CHECK_ENABLE: heavy-weight resource-creation calls you do at load time or during streaming, not every frame
        VK_CHECK(vmaCreateBuffer(allocator_, &bufInfo, &allocInfo, &bufAlloc.buffer, &bufAlloc.allocation, nullptr));
        return bufAlloc;
    }

    VkCommandPool Device::createCommandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags) const {
        VkCommandPoolCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        info.queueFamilyIndex = queueFamilyIndex;
        info.flags = flags;

        VkCommandPool pool;
        VK_CHECK(vkCreateCommandPool(device_, &info, nullptr, &pool));
        return pool;
    }

    VkCommandBuffer Device::beginOneTimePrimaryCommands(VkCommandPool commandPool) const {
        VkCommandBufferAllocateInfo ai{};
        ai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        ai.commandPool = commandPool;
        ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        ai.commandBufferCount = 1;

        VkCommandBuffer cmd;
        // VK_CHECK_ENABLE: Allocate once (or in big batches) and then reuse/reset them each frame—don’t re-allocate every iteration
        VK_CHECK(vkAllocateCommandBuffers(device_, &ai, &cmd));

        VkCommandBufferBeginInfo bi{};
        bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        // VK_CHECK_ENABLE: Do occur per frame (or per work submission), but is very cheap. Could strip the VK_CHECK in release if have validated that path under debug.
        VK_CHECK(vkBeginCommandBuffer(cmd, &bi));
        return cmd;
    }

    void Device::endOneTimePrimaryCommands(VkCommandBuffer cmd, VkQueue submitQueue, VkCommandPool commandPool) const {
        // VK_CHECK_ENABLE: Do occur per frame (or per work submission), but is very cheap. Could strip the VK_CHECK in release if have validated that path under debug.
        VK_CHECK(vkEndCommandBuffer(cmd));

        // Create fence
        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = 0;
        VkFence fence;
        // VK_CHECK_ENABLE: Create fences up front and recycle them—don’t create/destroy per frame
        VK_CHECK(vkCreateFence(device_, &fenceInfo, nullptr, &fence));

        // Create submit
        VkSubmitInfo submit{};
        submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit.commandBufferCount = 1;
        submit.pCommandBuffers = &cmd;
        // VK_CHECK_ENABLE: Part of hot loop (alongside vkCmdDraw, vkQueuePresent). Can omit the VK_CHECK in release if is sure it never fails, but most engines still handle VK_ERROR_OUT_OF_DATE_KHR at least.
        VK_CHECK(vkQueueSubmit(submitQueue, 1, &submit, VK_NULL_HANDLE));
        // VK_CHECK_ENABLE: Usually wait on last-frame’s fence once per frame. It’s cheap but branchable; can strip the check in release if really need to.
        VK_CHECK(vkWaitForFences(device_, 1, &fence, VK_TRUE, UINT64_MAX));

        // Clean up
        vkDestroyFence(device_, fence, nullptr);
        vkFreeCommandBuffers(device_, commandPool, 1, &cmd);
    }

    void Device::setObjectName(VkObjectType objectType, uint64_t objectHandle, const std::string& name) const {
        auto func = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetDeviceProcAddr(device_, "vkSetDebugUtilsObjectNameEXT");
        if (func) {
            VkDebugUtilsObjectNameInfoEXT info{};
            info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
            info.objectType = objectType;
            info.objectHandle = objectHandle;
            info.pObjectName = name.c_str();
            func(device_, &info);
        }
    }

    void Device::beginDebugLabel(VkCommandBuffer cmd, const std::string& labelName) const {
        auto func = (PFN_vkCmdBeginDebugUtilsLabelEXT)vkGetDeviceProcAddr(device_, "vkCmdBeginDebugUtilsLabelEXT");
        if (func) {
            VkDebugUtilsLabelEXT info{};
            info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
            info.pLabelName = labelName.c_str();
            // optional: set a color
            info.color[0] = info.color[1] = info.color[2] = info.color[3] = 0.0f;
            func(cmd, &info);
        }
    }

    void Device::endDebugLabel(VkCommandBuffer cmd) const {
        auto func = (PFN_vkCmdEndDebugUtilsLabelEXT)vkGetDeviceProcAddr(device_, "vkCmdEndDebugUtilsLabelEXT");
        if (func) { func(cmd); }
    }
}