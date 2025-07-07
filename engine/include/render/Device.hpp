#pragma once
#include "tool/HelpersVulkan.hpp"
#include <vk_mem_alloc.h>

namespace mve {
    // Helper struct for combined buffer+memory
    struct BufferWithMemory { // no need with vma
        VkBuffer       buffer;
        VkDeviceMemory memory;
    };
    struct BufferWithAllocation {
        VkBuffer         buffer;
        VmaAllocation    allocation;
    };

    class Device {
    public:
        // ctor/destructor
        Device(VkInstance instance,
            VkSurfaceKHR surface,
            const std::vector<const char*>& deviceExtensions,
            const std::vector<const char*>& deviceFeatures);
        ~Device();

        // No copy
        Device(const Device&) = delete;
        Device& operator=(const Device&) = delete;

        // memory & buffer helpers (no need with vma)
        uint32_t              findMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties) const;
        VkBuffer              createBuffer(VkDeviceSize size, VkBufferUsageFlags usage)           const;
        VkDeviceMemory        allocateMemory(VkBuffer buffer, VkMemoryPropertyFlags properties)   const;
        BufferWithMemory      createBufferWithMemory(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) const;
        // VMA-based buffer + memory creation
        BufferWithAllocation  createVMABuffer(VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags allocFlags = 0) const;

        // one-shot command helpers
        VkCommandPool         createCommandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags) const;
        VkCommandBuffer       beginOneTimePrimaryCommands(VkCommandPool commandPool) const;
        void                  endOneTimePrimaryCommands(VkCommandBuffer cmd, VkQueue submitQueue, VkCommandPool commandPool) const;
        VkCommandBuffer beginOneTimeTransferPrimaryCommands() const { return beginOneTimePrimaryCommands(transfer_command_pool_); }
        VkCommandBuffer beginOneTimeGraphicsPrimaryCommands() const { return beginOneTimePrimaryCommands(graphics_command_pool_); }
        VkCommandBuffer beginOneTimeComputePrimaryCommands () const { return beginOneTimePrimaryCommands(compute_command_pool_); }
        void endOneTimeTransferPrimaryCommands(VkCommandBuffer cmd) const { return endOneTimePrimaryCommands(cmd, transfer_queue_, transfer_command_pool_); }
        void endOneTimeGraphicsPrimaryCommands(VkCommandBuffer cmd) const { return endOneTimePrimaryCommands(cmd, graphics_queue_, graphics_command_pool_); }
        void endOneTimeComputePrimaryCommands (VkCommandBuffer cmd) const { return endOneTimePrimaryCommands(cmd, compute_queue_, compute_command_pool_); }

        // debug-utils (names & labels)
        void setObjectName(VkObjectType objectType, uint64_t objectHandle, const std::string& name) const;
        void beginDebugLabel(VkCommandBuffer cmd, const std::string& labelName)                     const;
        void endDebugLabel(VkCommandBuffer cmd)                                                     const;

        // accessors
        VkDevice                getDevice()                     const   { return device_; }
        VkSurfaceKHR            getSurface()                    const   { return surface_; }
        VkPhysicalDevice        getPhysicalDevice()             const   { return physical_device_; }
        VkQueue                 getGraphicsQueue()              const   { return graphics_queue_; }
        VkQueue                 getPresentQueue()               const   { return present_queue_; }
        VkQueue                 getComputeQueue()               const   { return compute_queue_; }
        VkQueue                 getTransferQueue()              const   { return transfer_queue_; }
        uint32_t                getGraphicsQueueFamilyIndex()   const   { return queueIndices_.graphicsFamilyIndex; }
        uint32_t                getPresentQueueFamilyIndex()    const   { return queueIndices_.presentFamilyIndex; }
        uint32_t                getComputeQueueFamilyIndex()    const   { return queueIndices_.computeFamilyIndex; }
        uint32_t                getTransferQueueFamilyIndex()   const   { return queueIndices_.transferFamilyIndex; }

    private:
        // internal setup
        void pickPhysicalDevice(VkInstance instance);
        void createLogicalDevice();
        void initializeVMA(VkInstance instance); // initialize allocator and vma pools

        // members
        VkSurfaceKHR                     surface_;
        VkPhysicalDevice                 physical_device_ = VK_NULL_HANDLE;
        VkDevice                         device_ = VK_NULL_HANDLE;
        QueueFamilyIndices               queueIndices_;
        VkQueue                          graphics_queue_ = VK_NULL_HANDLE;
        VkQueue                          present_queue_ = VK_NULL_HANDLE;
        VkQueue                          compute_queue_ = VK_NULL_HANDLE;
        VkQueue                          transfer_queue_ = VK_NULL_HANDLE;
        VkPhysicalDeviceMemoryProperties mem_props_{};
        std::vector<const char*>         device_extensions_ = { "VK_EXT_memory_budget", "VK_EXT_memory_priority",
                                                                "VK_EXT_hdr_metadata" };
        std::vector<const char*>         device_features_ = { "bufferDeviceAddress", "timelineSemaphore" };
        AllFeatures                      available_features_{};
        VkCommandPool                    graphics_command_pool_ = VK_NULL_HANDLE;
        VkCommandPool                    compute_command_pool_ = VK_NULL_HANDLE;
        VkCommandPool                    transfer_command_pool_ = VK_NULL_HANDLE;
        VkCommandPool                    static_transfer_command_pool_ = VK_NULL_HANDLE;
        VmaAllocator                     allocator_;
        VmaPool                          staging_pool_;         // per-frame uploads
        VmaPool                          static_resource_pool_;  // immutable, long-lived assets
        VmaPool                          streaming_pool_;       // medium-lived, frequently-updated data
    };
}