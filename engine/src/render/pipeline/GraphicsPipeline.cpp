// render/pipeline/GraphicsPipeline.cpp

#include "render/pipeline/GraphicsPipeline.hpp"
#include "render/Device.hpp"
#include "render/RenderPass.hpp"
#include "tool/HelpersVulkan.hpp"    // for VK_CHECK
#include <fstream>
#include <stdexcept>

namespace mve {

    GraphicsPipeline::GraphicsPipeline(
        Device& device,
        const std::string& vertSpirvPath,
        const std::string& fragSpirvPath,
        const PipelineConfig& config)
        : device_(&device), config_(config)
    {
        create(vertSpirvPath, fragSpirvPath);
    }

    GraphicsPipeline::~GraphicsPipeline() {
        if (pipeline_ != VK_NULL_HANDLE) vkDestroyPipeline(device_->getDevice(), pipeline_, nullptr);
    }

    GraphicsPipeline::GraphicsPipeline(GraphicsPipeline&& o) noexcept
        : device_(o.device_),
        pipeline_(o.pipeline_),
        config_(std::move(other.config_)),
        vertModule_(std::move(o.vertModule_)),
        fragModule_(std::move(o.fragModule_))
    {
        o.pipeline_ = VK_NULL_HANDLE;
        o.device_ = nullptr;
    }

    GraphicsPipeline& GraphicsPipeline::operator=(GraphicsPipeline&& other) noexcept {
        if (this == &other) return *this;
    
        // teardown our existing pipeline
        if (pipeline_ != VK_NULL_HANDLE && device_) {
            vkDestroyPipeline(device_->getDevice(), pipeline_, nullptr);
        }
    
        // steal
        device_   = other.device_;
        pipeline_ = other.pipeline_;
        config_   = std::move(other.config_);
        vertModule_ = std::move(other.vertModule_);
        fragModule_ = std::move(other.fragModule_);
    
        // leave other empty
        other.pipeline_ = VK_NULL_HANDLE;
        other.device_   = nullptr;
    
        return *this;
    }

    void GraphicsPipeline::bind(VkCommandBuffer cmd) const {
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_);
    }

    std::vector<char> GraphicsPipeline::readFile(const std::string& path) {
        std::ifstream file(path, std::ios::ate | std::ios::binary);
        if (!file.is_open()) throw std::runtime_error("Failed to open file: " + path);
        size_t size = (size_t)file.tellg();
        std::vector<char> buffer(size);
        file.seekg(0);
        file.read(buffer.data(), size);
        file.close();
        return buffer;
    }

    void GraphicsPipeline::create(const std::string& vsPath, const std::string& fsPath) {
        auto vertCode = readFile(vsPath);
        auto fragCode = readFile(fsPath);

        // Construct ShaderModule wrappers
        vertModule_ = { device_->getDevice(),
                        reinterpret_cast<const uint32_t*>(vertCode.data()),
                        vertCode.size() };
        fragModule_ = { device_->getDevice(),
                        reinterpret_cast<const uint32_t*>(fragCode.data()),
                        fragCode.size() };

        VkPipelineShaderStageCreateInfo vertStage{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
        vertStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertStage.module = vertModule_.get();
        vertStage.pName = "main";

        VkPipelineShaderStageCreateInfo fragStage{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
        fragStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragStage.module = fragModule_.get();
        fragStage.pName = "main";

        VkPipelineShaderStageCreateInfo stages[] = { vertStage, fragStage };

        // patch every CreateInfo’s counts & pointers from the vectors in config_
        auto& C = config_;
        C.vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        C.vertexInputInfo.vertexBindingDescriptionCount = uint32_t(C.bindingDescriptions.size());
        C.vertexInputInfo.pVertexBindingDescriptions = C.bindingDescriptions.data();
        C.vertexInputInfo.vertexAttributeDescriptionCount = uint32_t(C.attributeDescriptions.size());
        C.vertexInputInfo.pVertexAttributeDescriptions = C.attributeDescriptions.data();

        C.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        C.tessellationStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;

        C.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        C.viewportInfo.viewportCount = uint32_t(C.viewports.size());
        C.viewportInfo.pViewports = C.viewports.data();
        C.viewportInfo.scissorCount = uint32_t(C.scissors.size());
        C.viewportInfo.pScissors = C.scissors.data();

        C.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        C.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        C.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

        C.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        C.colorBlendInfo.attachmentCount = uint32_t(C.colorBlendAttachments.size());
        C.colorBlendInfo.pAttachments = C.colorBlendAttachments.data();

        C.dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        C.dynamicStateInfo.dynamicStateCount = uint32_t(C.dynamicStates.size());
        C.dynamicStateInfo.pDynamicStates = C.dynamicStates.data();

        // assemble the master VkGraphicsPipelineCreateInfo from C
        VkGraphicsPipelineCreateInfo pInfo{};
        pInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pInfo.flags = C.flags;
        pInfo.stageCount = 2;
        pInfo.pStages = stages;
        pInfo.pVertexInputState = &C.vertexInputInfo;
        pInfo.pInputAssemblyState = &C.inputAssemblyInfo;
        pInfo.pTessellationState = &C.tessellationStateInfo;
        pInfo.pViewportState = &C.viewportInfo;
        pInfo.pRasterizationState = &C.rasterizationInfo;
        pInfo.pMultisampleState = &C.multisampleInfo;
        pInfo.pDepthStencilState = &C.depthStencilInfo;
        pInfo.pColorBlendState = &C.colorBlendInfo;
        pInfo.pDynamicState = &C.dynamicStateInfo;
        pInfo.layout = C.layout;
        pInfo.renderPass = C.renderPass;
        pInfo.subpass = C.subpass;
        pInfo.basePipelineHandle = C.basePipelineHandle;
        pInfo.basePipelineIndex = C.basePipelineIndex;

        VK_CHECK(vkCreateGraphicsPipelines(
            device_->getDevice(),
            C.pipelineCache,
            1, &pInfo, nullptr,
            &pipeline_
        ));
    }

    void GraphicsPipeline::defaultConfig(PipelineConfig& c, VkExtent2D extent) {
        // — top-level —
        C.flags = 0;
        C.pipelineCache = VK_NULL_HANDLE;

        // — vertex input —
        C.bindingDescriptions.clear();
        C.attributeDescriptions.clear();
        C.vertexInputInfo = {};
        C.vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        // — input assembly —
        C.inputAssemblyInfo = {};
        C.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        C.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        C.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

        // — tessellation —
        C.tessellationStateInfo = {};
        C.tessellationStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
        C.tessellationStateInfo.patchControlPoints = 0;

        // — viewport & scissor —
        C.viewports = { VkViewport{0,0,(float)extent.width,(float)extent.height, 0,1} };
        C.scissors = { VkRect2D{{0,0}, extent} };
        C.viewportInfo = {};
        C.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        C.viewportInfo.viewportCount = 1;
        C.viewportInfo.pViewports = C.viewports.data();
        C.viewportInfo.scissorCount = 1;
        C.viewportInfo.pScissors = C.scissors.data();

        // — rasterization —
        C.rasterizationInfo = {};
        C.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        C.rasterizationInfo.depthClampEnable = VK_FALSE;
        C.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
        C.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
        C.rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
        C.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
        C.rasterizationInfo.depthBiasEnable = VK_FALSE;
        C.rasterizationInfo.lineWidth = 1.0f;

        // — multisample —
        C.multisampleInfo = {};
        C.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        C.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        C.multisampleInfo.sampleShadingEnable = VK_FALSE;
        C.multisampleInfo.minSampleShading = 1.0f;
        C.multisampleInfo.pSampleMask = nullptr;
        C.multisampleInfo.alphaToCoverageEnable = VK_FALSE;
        C.multisampleInfo.alphaToOneEnable = VK_FALSE;

        // — depth & stencil —
        C.depthStencilInfo = {};
        C.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        C.depthStencilInfo.depthTestEnable = VK_TRUE;
        C.depthStencilInfo.depthWriteEnable = VK_TRUE;
        C.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
        C.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
        C.depthStencilInfo.stencilTestEnable = VK_FALSE;

        // — color blend —
        C.colorBlendAttachments.clear();
        VkPipelineColorBlendAttachmentState att{};
        att.colorWriteMask = VK_COLOR_COMPONENT_R_BIT
            | VK_COLOR_COMPONENT_G_BIT
            | VK_COLOR_COMPONENT_B_BIT
            | VK_COLOR_COMPONENT_A_BIT;
        att.blendEnable = VK_FALSE;
        C.colorBlendAttachments.push_back(att);
        C.colorBlendInfo = {};
        C.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        C.colorBlendInfo.logicOpEnable = VK_FALSE;
        C.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;
        C.colorBlendInfo.attachmentCount = 1;
        C.colorBlendInfo.pAttachments = C.colorBlendAttachments.data();
        C.colorBlendInfo.blendConstants[0] = 0.0f;
        C.colorBlendInfo.blendConstants[1] = 0.0f;
        C.colorBlendInfo.blendConstants[2] = 0.0f;
        C.colorBlendInfo.blendConstants[3] = 0.0f;

        // — dynamic state —
        C.dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
        C.dynamicStateInfo = {};
        C.dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        C.dynamicStateInfo.dynamicStateCount = 2;
        C.dynamicStateInfo.pDynamicStates = C.dynamicStates.data();

        // — push constants, layout, render-pass & derivatives —
        C.pushConstantRanges.clear();
        C.layout = VK_NULL_HANDLE;
        C.renderPass = VK_NULL_HANDLE;
        C.subpass = 0;
        C.basePipelineHandle = VK_NULL_HANDLE;
        C.basePipelineIndex = -1;
    }

} // namespace mve
