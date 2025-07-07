#pragma once

#include "render/Device.hpp"
#include <vulkan/vulkan.h>
#include <vector>

namespace mve {

    // Manages a Vulkan command pool and allocates command buffers
    class CommandBufferPool {
    public:
        // Create a transient, resettable command pool for the given queue family
        CommandBufferPool(Device& device,
            uint32_t queueFamilyIndex,
            VkCommandPoolCreateFlags flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

        // No copy, only movable
        CommandBufferPool(const CommandBufferPool&) = delete;
        CommandBufferPool& operator=(const CommandBufferPool&) = delete;
        CommandBufferPool(CommandBufferPool&& other) noexcept;
        CommandBufferPool& operator=(CommandBufferPool&& other) noexcept;

        ~CommandBufferPool();

        // Allocate a primary or secondary command buffer from this pool
        VkCommandBuffer allocateBuffer(VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

        // Reset and optionally release all command buffers
        void reset(VkCommandPoolResetFlags flags = 0);

        VkCommandPool            handle()           const { return pool_; }
        VkCommandPoolCreateFlags getCreateFlags()   const { return poolFlags_; }

    private:
        void destroy();

        Device* device_ = nullptr;
        VkCommandPool pool_ = VK_NULL_HANDLE;
        VkCommandPoolCreateFlags  poolFlags_;
    };

    // RAII wrapper around a single Vulkan command buffer
    class CommandBuffer {
    public:
        // Construct from an existing pool and allocated buffer handle
        CommandBuffer(Device& device, CommandBufferPool& pool, VkCommandBuffer cmdBuffer);

        // No copy, only movable
        CommandBuffer(const CommandBuffer&) = delete;
        CommandBuffer& operator=(const CommandBuffer&) = delete;
        CommandBuffer(CommandBuffer&& other) noexcept;
        CommandBuffer& operator=(CommandBuffer&& other) noexcept;

        ~CommandBuffer();

        // Begin recording; for secondary, provide inheritanceInfo
        void begin(VkCommandBufferUsageFlags usage = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
            const VkCommandBufferInheritanceInfo* inheritanceInfo = nullptr);

        // End recording
        void end();

        // Insert a pipeline barrier
        void pipelineBarrier(VkPipelineStageFlags srcStageMask,
            VkPipelineStageFlags dstStageMask,
            VkDependencyFlags dependencyFlags,
            const std::vector<VkMemoryBarrier>& memoryBarriers = {},
            const std::vector<VkBufferMemoryBarrier>& bufferBarriers = {},
            const std::vector<VkImageMemoryBarrier>& imageBarriers = {});

        // Submit to a queue, optionally wait/signal semaphores and fence
        void submit(VkQueue queue,
            VkSemaphore waitSemaphore = VK_NULL_HANDLE,
            VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VkSemaphore signalSemaphore = VK_NULL_HANDLE,
            VkFence fence = VK_NULL_HANDLE) const;

        // Reset this buffer (requires the pool was created with RESET_COMMAND_BUFFER_BIT)
        void reset(VkCommandBufferResetFlags flags = 0);

        // Access the raw handle
        VkCommandBuffer handle() const noexcept { return cmdBuffer_; }

    private:
        void destroy();

        Device* device_ = nullptr;
        VkCommandPool pool_ = VK_NULL_HANDLE;
        VkCommandPoolCreateFlags poolFlags_;
        VkCommandBuffer cmdBuffer_ = VK_NULL_HANDLE;
    };

} // namespace mve