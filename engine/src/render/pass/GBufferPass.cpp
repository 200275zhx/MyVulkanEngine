// GBufferPass.cpp
#include "render/pass/GBufferPass.hpp"
#include <array>

namespace mve {

    GBufferPass::GBufferPass(Device& device,
        VkExtent2D extent,
        const std::vector<std::vector<VkImageView>>& views,
        const std::vector<VkFence>& inFlightFences,
        uint32_t currentFrame,
        VkSampleCountFlagBits msaaSamples)
        : device_(&device)
        , renderPass_(
            [&]() {
                RenderPass::Builder b;

                // explicit layout transitions
                VkSubpassDependency d1{ VK_SUBPASS_EXTERNAL, 0,
                                        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                        0,
                                        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT };
                b.addDependency(d1);
                VkSubpassDependency d2{ 0, VK_SUBPASS_EXTERNAL,
                                        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                        VK_ACCESS_SHADER_READ_BIT };
                b.addDependency(d2);

                // position
                VkAttachmentDescription posDesc{ };
                posDesc.format = VK_FORMAT_R16G16B16A16_SFLOAT;
                posDesc.samples = msaaSamples;
                posDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                posDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                posDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                posDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                posDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                posDesc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                posAtt_ = b.addAttachment(AttachmentRole::GBufferPosition, posDesc)
                    .getAttachmentIndex(AttachmentRole::GBufferPosition);

                // normal
                VkAttachmentDescription normDesc = posDesc;
                normAtt_ = b.addAttachment(AttachmentRole::GBufferNormal, normDesc)
                    .getAttachmentIndex(AttachmentRole::GBufferNormal);

                // albedo
                VkAttachmentDescription albDesc = posDesc;
                albDesc.format = VK_FORMAT_B8G8R8A8_UNORM;
                albedoAtt_ = b.addAttachment(AttachmentRole::GBufferAlbedo, albDesc)
                    .getAttachmentIndex(AttachmentRole::GBufferAlbedo);

                // depth
                VkAttachmentDescription depthDesc{ };
                depthDesc.format = VK_FORMAT_D32_SFLOAT;
                depthDesc.samples = msaaSamples;
                depthDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                depthDesc.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                depthDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                depthDesc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                depthAtt_ = b.addAttachment(AttachmentRole::DepthStencil, depthDesc)
                    .getAttachmentIndex(AttachmentRole::DepthStencil);

                // single subpass
                SubpassInfo sp;
                sp.bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
                sp.colorAttachments = { AttachmentRole::GBufferPosition,
                                            AttachmentRole::GBufferNormal,
                                            AttachmentRole::GBufferAlbedo };
                sp.depthStencilAttachment = AttachmentRole::DepthStencil;
                b.addSubpass(sp);

                return b.build(device, extent, views, inFlightFences, currentFrame);
            }())
        , msaaSamples_(msaaSamples)
        , pipelineLayout_(VK_NULL_HANDLE)
    {
        // dynamic clear-values
        clearValues_.resize(renderPass_.getCreateInfo().attachments.size());
        clearValues_[posAtt_].color = { {0,0,0,0} };
        clearValues_[normAtt_].color = { {0,0,0,0} };
        clearValues_[albedoAtt_].color = { {0,0,0,0} };
        clearValues_[depthAtt_].depthStencil = { 1.0f, 0 };

        buildPipeline();
    }

    GBufferPass::~GBufferPass() {
        if (pipelineLayout_ != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(device_->getDevice(), pipelineLayout_, nullptr);
        }
    }

    void GBufferPass::buildPipeline() {
        // pipeline-layout for frame UBO + material sets
        std::array<VkDescriptorSetLayout, 2> layouts = {
            /* FRAME_UBO_LAYOUT */, /* MATERIAL_LAYOUT */
        };
        VkPipelineLayoutCreateInfo plc{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
        plc.setLayoutCount = (uint32_t)layouts.size();
        plc.pSetLayouts = layouts.data();
        plc.pushConstantRangeCount = 0;
        plc.pPushConstantRanges = nullptr;
        VK_CHECK(vkCreatePipelineLayout(device_->getDevice(), &plc, nullptr, &pipelineLayout_));

        auto extent = renderPass_.getExtent();
        PipelineConfig cfg{};
        GraphicsPipeline::defaultConfig(cfg, extent);
        cfg.layout = pipelineLayout_;
        cfg.renderPass = renderPass_.getHandle();
        cfg.subpass = 0;

        pipeline_ = GraphicsPipeline(
            *device_,
            PROJECT_ROOT_DIR "/shaders/spir-v/gbuffer.vert.spv",
            PROJECT_ROOT_DIR "/shaders/spir-v/gbuffer.frag.spv",
            cfg
        );
    }

    void GBufferPass::recreate(VkExtent2D extent,
        const std::vector<std::vector<VkImageView>>& views)
    {
        renderPass_.recreate(views, extent);
        clearValues_.resize(renderPass_.getCreateInfo().attachments.size());
        clearValues_[posAtt_].color = { {0,0,0,0} };
        clearValues_[normAtt_].color = { {0,0,0,0} };
        clearValues_[albedoAtt_].color = { {0,0,0,0} };
        clearValues_[depthAtt_].depthStencil = { 1.0f, 0 };

        // rebuild pipeline (new extent / sample count)
        if (pipelineLayout_ != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(device_->getDevice(), pipelineLayout_, nullptr);
            pipelineLayout_ = VK_NULL_HANDLE;
        }
        buildPipeline();
    }

    void GBufferPass::begin(VkCommandBuffer cmd, uint32_t imageIndex) {
        device_->beginDebugLabel(cmd, "G-Buffer Pass");
        renderPass_.begin(cmd, imageIndex, clearValues_, VK_SUBPASS_CONTENTS_INLINE);
    }

    void GBufferPass::end(VkCommandBuffer cmd) {
        renderPass_.end(cmd);
        device_->endDebugLabel(cmd);
    }
} // namespace mve