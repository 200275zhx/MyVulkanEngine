#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#include "render/ShaderModule.hpp"

namespace mve {

    class Device;    // your Device wrapper
    class RenderPass; // your RenderPass wrapper

    class GraphicsPipeline {
    public:
        struct PipelineConfig {
            //—— Top-level flags & cache ——
            VkPipelineCreateFlags           flags = 0;
            VkPipelineCache                 pipelineCache = VK_NULL_HANDLE;

            //—— Vertex input ——
            std::vector<VkVertexInputBindingDescription>   bindingDescriptions{};
            std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
            VkPipelineVertexInputStateCreateInfo           vertexInputInfo{};

            //—— Input assembly ——
            VkPipelineInputAssemblyStateCreateInfo         inputAssemblyInfo{};

            //—— Tessellation (optional) ——
            VkPipelineTessellationStateCreateInfo          tessellationStateInfo{};

            //—— Viewport & scissor ——
            std::vector<VkViewport>                        viewports{};
            std::vector<VkRect2D>                          scissors{};
            VkPipelineViewportStateCreateInfo              viewportInfo{};

            //—— Rasterization ——
            VkPipelineRasterizationStateCreateInfo         rasterizationInfo{};

            //—— Multisample ——
            VkPipelineMultisampleStateCreateInfo           multisampleInfo{};

            //—— Depth & stencil ——
            VkPipelineDepthStencilStateCreateInfo          depthStencilInfo{};

            //—— Color blend ——
            std::vector<VkPipelineColorBlendAttachmentState>    colorBlendAttachments{};
            VkPipelineColorBlendStateCreateInfo                 colorBlendInfo{};

            //—— Dynamic state ——
            std::vector<VkDynamicState>                    dynamicStates{};
            VkPipelineDynamicStateCreateInfo               dynamicStateInfo{};

            //—— Push-constant ranges (for upstream layout/reflection) ——
            std::vector<VkPushConstantRange>               pushConstantRanges{};

            //—— Layout & render-pass binding ——
            VkPipelineLayout               layout = VK_NULL_HANDLE;
            VkRenderPass                   renderPass = VK_NULL_HANDLE;
            uint32_t                       subpass = 0;

            //—— Derivative pipeline support ——
            VkPipeline                     basePipelineHandle = VK_NULL_HANDLE;
            int32_t                        basePipelineIndex = -1;
        };

        GraphicsPipeline(
            Device& device,
            const std::string& vertSpirvPath,
            const std::string& fragSpirvPath,
            const PipelineConfig& config);
        ~GraphicsPipeline();

        GraphicsPipeline(const GraphicsPipeline&) = delete;
        GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;
        GraphicsPipeline(GraphicsPipeline&&) noexcept;
        GraphicsPipeline& operator=(GraphicsPipeline&&) noexcept;

        void bind(VkCommandBuffer cmd) const;

        // fills out Config with reasonable defaults for a given extent
        static void defaultConfig(PipelineConfig& c, VkExtent2D extent);

    private:
        std::vector<char> readFile(const std::string& path);
        void create(const std::string& vsPath, const std::string& fsPath);

        Device* device_;
        PipelineConfig         config_;
        VkPipeline     pipeline_{ VK_NULL_HANDLE };
        ShaderModule vertModule_{ VK_NULL_HANDLE };
        ShaderModule fragModule_{ VK_NULL_HANDLE };
    };

} // namespace mve
