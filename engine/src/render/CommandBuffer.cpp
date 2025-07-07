#include "render/CommandBuffer.hpp"
#include "tool/HelpersVulkan.hpp"
#include <stdexcept>

namespace mve {

    // ---------- CommandBufferPool Implementation ---------- //
    CommandBufferPool::CommandBufferPool(Device& device, uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags)
        : device_(&device), poolFlags_(flags)
    {
        pool_ = device.createCommandPool(queueFamilyIndex, flags);
    }

    CommandBufferPool::CommandBufferPool(CommandBufferPool&& other) noexcept
        : device_(other.device_), pool_(other.pool_)
    {
        other.device_ = nullptr;
        other.pool_ = VK_NULL_HANDLE;
    }

    CommandBufferPool& CommandBufferPool::operator=(CommandBufferPool&& other) noexcept
    {
        if (this != &other) {
            destroy();
            device_ = other.device_;
            pool_ = other.pool_;
            other.device_ = nullptr;
            other.pool_ = VK_NULL_HANDLE;
        }
        return *this;
    }

    CommandBufferPool::~CommandBufferPool() {
        destroy();
    }

    void CommandBufferPool::destroy() {
        if (pool_ != VK_NULL_HANDLE) {
            vkDestroyCommandPool(device_->getDevice(), pool_, nullptr);
            pool_ = VK_NULL_HANDLE;
        }
    }

    VkCommandBuffer CommandBufferPool::allocateBuffer(VkCommandBufferLevel level) {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = pool_;
        allocInfo.level = level;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer cmd;
        VK_CHECK(vkAllocateCommandBuffers(device_->getDevice(), &allocInfo, &cmd));
        return cmd;
    }

    void CommandBufferPool::reset(VkCommandPoolResetFlags flags) {
        VK_CHECK(vkResetCommandPool(device_->getDevice(), pool_, flags));
    }


    // ---------- CommandBuffer Implementation ---------- //
    CommandBuffer::CommandBuffer(Device& device, CommandBufferPool& pool, VkCommandBuffer cmdBuffer)
        : device_(&device), pool_(pool.handle()), poolFlags_(pool.getCreateFlags()), cmdBuffer_(cmdBuffer) {
    }

    CommandBuffer::CommandBuffer(CommandBuffer&& other) noexcept
        : device_(other.device_), pool_(other.pool_), cmdBuffer_(other.cmdBuffer_)
    {
        other.device_ = nullptr;
        other.cmdBuffer_ = VK_NULL_HANDLE;
        other.pool_ = VK_NULL_HANDLE;
    }

    CommandBuffer& CommandBuffer::operator=(CommandBuffer&& other) noexcept
    {
        if (this != &other) {
            destroy();
            device_ = other.device_;
            pool_ = other.pool_;
            cmdBuffer_ = other.cmdBuffer_;
            other.device_ = nullptr;
            other.cmdBuffer_ = VK_NULL_HANDLE;
            other.pool_ = VK_NULL_HANDLE;
        }
        return *this;
    }

    CommandBuffer::~CommandBuffer() {
        destroy();
    }

    void CommandBuffer::destroy() {
        if (cmdBuffer_ != VK_NULL_HANDLE) {
            vkFreeCommandBuffers(device_->getDevice(), pool_, 1, &cmdBuffer_);
            cmdBuffer_ = VK_NULL_HANDLE;
        }
    }

    void CommandBuffer::begin(VkCommandBufferUsageFlags usage, const VkCommandBufferInheritanceInfo* inheritanceInfo) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = usage;
        beginInfo.pInheritanceInfo = inheritanceInfo;
        VK_CHECK(vkBeginCommandBuffer(cmdBuffer_, &beginInfo));
    }

    void CommandBuffer::end() {
        VK_CHECK(vkEndCommandBuffer(cmdBuffer_));
    }

    void CommandBuffer::pipelineBarrier(VkPipelineStageFlags srcStageMask,
        VkPipelineStageFlags dstStageMask,
        VkDependencyFlags dependencyFlags,
        const std::vector<VkMemoryBarrier>& memoryBarriers,
        const std::vector<VkBufferMemoryBarrier>& bufferBarriers,
        const std::vector<VkImageMemoryBarrier>& imageBarriers) {
        vkCmdPipelineBarrier(
            cmdBuffer_,
            srcStageMask,
            dstStageMask,
            dependencyFlags,
            static_cast<uint32_t>(memoryBarriers.size()), memoryBarriers.data(),
            static_cast<uint32_t>(bufferBarriers.size()), bufferBarriers.data(),
            static_cast<uint32_t>(imageBarriers.size()), imageBarriers.data());
    }

    void CommandBuffer::submit(VkQueue queue,
        VkSemaphore waitSemaphore,
        VkPipelineStageFlags waitStage,
        VkSemaphore signalSemaphore,
        VkFence fence) const {
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        if (waitSemaphore != VK_NULL_HANDLE) {
            submitInfo.waitSemaphoreCount = 1;
            submitInfo.pWaitSemaphores = &waitSemaphore;
            submitInfo.pWaitDstStageMask = &waitStage;
        }
        if (signalSemaphore != VK_NULL_HANDLE) {
            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores = &signalSemaphore;
        }
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmdBuffer_;

        VK_CHECK(vkQueueSubmit(queue, 1, &submitInfo, fence));
    }

    void CommandBuffer::reset(VkCommandBufferResetFlags flags) {
        if (!(poolFlags_ & VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT)) {
            throw std::runtime_error(
                "Cannot reset individual command buffer: "
                "pool was not created with VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT");
        }
        // vkResetCommandBuffer will clear out all recorded commands
        // Make sure your pool was created with RESET_COMMAND_BUFFER_BIT
        VK_CHECK(vkResetCommandBuffer(
            device_->getDevice(),
            cmdBuffer_,
            flags
        ));
    }

} // namespace mve
