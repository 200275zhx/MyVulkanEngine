#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include <stdexcept>
#include <iostream>
#include <set>

#define VULKAN_PHYSICAL_DEVICE_INFO_CHECK
#define VK_CHECK(cmd)                                                         \
  do {                                                                        \
    VkResult _r = (cmd);                                                      \
    if (_r != VK_SUCCESS) {                                                   \
      throw std::runtime_error(                                               \
        std::string(#cmd) +                                                   \
        " failed at " + __FILE__ + ":" + std::to_string(__LINE__) +           \
        " (VkResult=" + std::to_string(_r) + ")"                              \
      );                                                                      \
    }                                                                         \
  } while (0)

namespace mve {
    // ---------- Vulkan Device ---------- //

    // All known Vulkan feature flag names (core 1.0 + 1.1–1.4)
    static const std::vector<std::string> ALL_VULKAN_FEATURE_NAMES = {
        // Core 1.0
        "robustBufferAccess","fullDrawIndexUint32","imageCubeArray","independentBlend","geometryShader",
        "tessellationShader","sampleRateShading","dualSrcBlend","logicOp","multiDrawIndirect",
        "drawIndirectFirstInstance","depthClamp","depthBiasClamp","fillModeNonSolid","depthBounds","wideLines",
        "largePoints","alphaToOne","multiViewport","samplerAnisotropy","textureCompressionETC2",
        "textureCompressionASTC_LDR","textureCompressionBC","occlusionQueryPrecise","pipelineStatisticsQuery",
        "vertexPipelineStoresAndAtomics","fragmentStoresAndAtomics","shaderTessellationAndGeometryPointSize",
        "shaderImageGatherExtended","shaderStorageImageExtendedFormats","shaderStorageImageMultisample",
        "shaderStorageImageReadWithoutFormat","shaderStorageImageWriteWithoutFormat",
        "shaderUniformBufferArrayDynamicIndexing","shaderSampledImageArrayDynamicIndexing",
        "shaderStorageBufferArrayDynamicIndexing","shaderStorageImageArrayDynamicIndexing",
        "shaderClipDistance","shaderCullDistance","shaderFloat64","shaderInt64","shaderInt16",
        "shaderResourceResidency","shaderResourceMinLod","sparseBinding","sparseResidencyBuffer",
        "sparseResidencyImage2D","sparseResidencyImage3D","sparseResidency2Samples","sparseResidency4Samples",
        "sparseResidency8Samples","sparseResidency16Samples","sparseResidencyAliased","variableMultisampleRate",
        "inheritedQueries",

        // Vulkan 1.1
        "storageBuffer16BitAccess","uniformAndStorageBuffer16BitAccess",
        "storagePushConstant16","storageInputOutput16","multiview",
        "multiviewGeometryShader","multiviewTessellationShader",
        "variablePointersStorageBuffer","variablePointers","protectedMemory",
        "samplerYcbcrConversion","shaderDrawParameters",

        // Vulkan 1.2
        "samplerMirrorClampToEdge","drawIndirectCount","storageBuffer8BitAccess",
        "uniformAndStorageBuffer8BitAccess","storagePushConstant8",
        "shaderBufferInt64Atomics","shaderSharedInt64Atomics",
        "shaderFloat16","shaderInt8","descriptorIndexing",
        "shaderInputAttachmentArrayDynamicIndexing",
        "shaderUniformTexelBufferArrayDynamicIndexing",
        "shaderStorageTexelBufferArrayDynamicIndexing",
        "shaderUniformBufferArrayNonUniformIndexing",
        "shaderSampledImageArrayNonUniformIndexing",
        "shaderStorageBufferArrayNonUniformIndexing",
        "shaderStorageImageArrayNonUniformIndexing",
        "shaderInputAttachmentArrayNonUniformIndexing",
        "descriptorBindingUniformBufferUpdateAfterBind",
        "descriptorBindingSampledImageUpdateAfterBind",
        "descriptorBindingStorageImageUpdateAfterBind",
        "descriptorBindingStorageBufferUpdateAfterBind",
        "descriptorBindingUniformTexelBufferUpdateAfterBind",
        "descriptorBindingStorageTexelBufferUpdateAfterBind",
        "descriptorBindingUpdateUnusedWhilePending",
        "descriptorBindingPartiallyBound","descriptorBindingVariableDescriptorCount",
        "runtimeDescriptorArray","samplerFilterMinmax","scalarBlockLayout",
        "imagelessFramebuffer","uniformBufferStandardLayout",
        "shaderSubgroupExtendedTypes","separateDepthStencilLayouts",
        "hostQueryReset","timelineSemaphore","bufferDeviceAddress",
        "bufferDeviceAddressCaptureReplay","bufferDeviceAddressMultiDevice",
        "vulkanMemoryModel","vulkanMemoryModelDeviceScope",
        "vulkanMemoryModelAvailabilityVisibilityChains",
        "shaderOutputViewportIndex","shaderOutputLayer",
        "subgroupBroadcastDynamicId",

        // Vulkan 1.3
        "robustImageAccess","inlineUniformBlock",
        "descriptorBindingInlineUniformBlockUpdateAfterBind",
        "pipelineCreationCacheControl","privateData",
        "shaderDemoteToHelperInvocation","shaderTerminateInvocation",
        "subgroupSizeControl","computeFullSubgroups","synchronization2",
        "textureCompressionASTC_HDR","shaderZeroInitializeWorkgroupMemory",
        "dynamicRendering","shaderIntegerDotProduct","maintenance4",

        // Vulkan 1.4
        "globalPriorityQuery","shaderSubgroupRotate",
        "shaderSubgroupRotateClustered","shaderFloatControls2",
        "shaderExpectAssume","rectangularLines","bresenhamLines",
        "smoothLines","stippledRectangularLines","stippledBresenhamLines",
        "stippledSmoothLines","vertexAttributeInstanceRateDivisor",
        "vertexAttributeInstanceRateZeroDivisor","indexTypeUint8",
        "dynamicRenderingLocalRead","maintenance5","maintenance6",
        "pipelineProtectedAccess","pipelineRobustness","hostImageCopy",
        "pushDescriptor"
    };

    struct AllFeatures {
        VkPhysicalDeviceFeatures2          core{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, nullptr, {}};
        VkPhysicalDeviceVulkan11Features   v11{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES, nullptr, {} };
        VkPhysicalDeviceVulkan12Features   v12{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES, nullptr, {} };
        VkPhysicalDeviceVulkan13Features   v13{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES, nullptr, {} };
        VkPhysicalDeviceVulkan14Features   v14{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES, nullptr, {} };
    };

    struct FeatureMap {
        const char* name;
        VkBool32 VkPhysicalDeviceFeatures::* core;
        VkBool32 VkPhysicalDeviceVulkan11Features::* v11;
        VkBool32 VkPhysicalDeviceVulkan12Features::* v12;
        VkBool32 VkPhysicalDeviceVulkan13Features::* v13;
        VkBool32 VkPhysicalDeviceVulkan14Features::* v14;
    };

    static const FeatureMap FEATURE_TABLE[] = {
        // ––– Core 1.0 –––
        { "robustBufferAccess",                     &VkPhysicalDeviceFeatures::robustBufferAccess,                      nullptr, nullptr, nullptr, nullptr },
        { "fullDrawIndexUint32",                    &VkPhysicalDeviceFeatures::fullDrawIndexUint32,                     nullptr, nullptr, nullptr, nullptr },
        { "imageCubeArray",                         &VkPhysicalDeviceFeatures::imageCubeArray,                          nullptr, nullptr, nullptr, nullptr },
        { "independentBlend",                       &VkPhysicalDeviceFeatures::independentBlend,                        nullptr, nullptr, nullptr, nullptr },
        { "geometryShader",                         &VkPhysicalDeviceFeatures::geometryShader,                          nullptr, nullptr, nullptr, nullptr },
        { "tessellationShader",                     &VkPhysicalDeviceFeatures::tessellationShader,                      nullptr, nullptr, nullptr, nullptr },
        { "sampleRateShading",                      &VkPhysicalDeviceFeatures::sampleRateShading,                       nullptr, nullptr, nullptr, nullptr },
        { "dualSrcBlend",                           &VkPhysicalDeviceFeatures::dualSrcBlend,                            nullptr, nullptr, nullptr, nullptr },
        { "logicOp",                                &VkPhysicalDeviceFeatures::logicOp,                                 nullptr, nullptr, nullptr, nullptr },
        { "multiDrawIndirect",                      &VkPhysicalDeviceFeatures::multiDrawIndirect,                       nullptr, nullptr, nullptr, nullptr },
        { "drawIndirectFirstInstance",              &VkPhysicalDeviceFeatures::drawIndirectFirstInstance,               nullptr, nullptr, nullptr, nullptr },
        { "depthClamp",                             &VkPhysicalDeviceFeatures::depthClamp,                              nullptr, nullptr, nullptr, nullptr },
        { "depthBiasClamp",                         &VkPhysicalDeviceFeatures::depthBiasClamp,                          nullptr, nullptr, nullptr, nullptr },
        { "fillModeNonSolid",                       &VkPhysicalDeviceFeatures::fillModeNonSolid,                        nullptr, nullptr, nullptr, nullptr },
        { "depthBounds",                            &VkPhysicalDeviceFeatures::depthBounds,                             nullptr, nullptr, nullptr, nullptr },
        { "wideLines",                              &VkPhysicalDeviceFeatures::wideLines,                               nullptr, nullptr, nullptr, nullptr },
        { "largePoints",                            &VkPhysicalDeviceFeatures::largePoints,                             nullptr, nullptr, nullptr, nullptr },
        { "alphaToOne",                             &VkPhysicalDeviceFeatures::alphaToOne,                              nullptr, nullptr, nullptr, nullptr },
        { "multiViewport",                          &VkPhysicalDeviceFeatures::multiViewport,                           nullptr, nullptr, nullptr, nullptr },
        { "samplerAnisotropy",                      &VkPhysicalDeviceFeatures::samplerAnisotropy,                       nullptr, nullptr, nullptr, nullptr },
        { "textureCompressionETC2",                 &VkPhysicalDeviceFeatures::textureCompressionETC2,                  nullptr, nullptr, nullptr, nullptr },
        { "textureCompressionASTC_LDR",             &VkPhysicalDeviceFeatures::textureCompressionASTC_LDR,              nullptr, nullptr, nullptr, nullptr },
        { "textureCompressionBC",                   &VkPhysicalDeviceFeatures::textureCompressionBC,                    nullptr, nullptr, nullptr, nullptr },
        { "occlusionQueryPrecise",                  &VkPhysicalDeviceFeatures::occlusionQueryPrecise,                   nullptr, nullptr, nullptr, nullptr },
        { "pipelineStatisticsQuery",                &VkPhysicalDeviceFeatures::pipelineStatisticsQuery,                 nullptr, nullptr, nullptr, nullptr },
        { "vertexPipelineStoresAndAtomics",         &VkPhysicalDeviceFeatures::vertexPipelineStoresAndAtomics,          nullptr, nullptr, nullptr, nullptr },
        { "fragmentStoresAndAtomics",               &VkPhysicalDeviceFeatures::fragmentStoresAndAtomics,                nullptr, nullptr, nullptr, nullptr },
        { "shaderTessellationAndGeometryPointSize", &VkPhysicalDeviceFeatures::shaderTessellationAndGeometryPointSize,  nullptr, nullptr, nullptr, nullptr },
        { "shaderImageGatherExtended",              &VkPhysicalDeviceFeatures::shaderImageGatherExtended,               nullptr, nullptr, nullptr, nullptr },
        { "shaderStorageImageExtendedFormats",      &VkPhysicalDeviceFeatures::shaderStorageImageExtendedFormats,       nullptr, nullptr, nullptr, nullptr },
        { "shaderStorageImageMultisample",          &VkPhysicalDeviceFeatures::shaderStorageImageMultisample,           nullptr, nullptr, nullptr, nullptr },
        { "shaderStorageImageReadWithoutFormat",    &VkPhysicalDeviceFeatures::shaderStorageImageReadWithoutFormat,     nullptr, nullptr, nullptr, nullptr },
        { "shaderStorageImageWriteWithoutFormat",   &VkPhysicalDeviceFeatures::shaderStorageImageWriteWithoutFormat,    nullptr, nullptr, nullptr, nullptr },
        { "shaderUniformBufferArrayDynamicIndexing",&VkPhysicalDeviceFeatures::shaderUniformBufferArrayDynamicIndexing, nullptr, nullptr, nullptr, nullptr },
        { "shaderSampledImageArrayDynamicIndexing", &VkPhysicalDeviceFeatures::shaderSampledImageArrayDynamicIndexing,  nullptr, nullptr, nullptr, nullptr },
        { "shaderStorageBufferArrayDynamicIndexing",&VkPhysicalDeviceFeatures::shaderStorageBufferArrayDynamicIndexing, nullptr, nullptr, nullptr, nullptr },
        { "shaderStorageImageArrayDynamicIndexing", &VkPhysicalDeviceFeatures::shaderStorageImageArrayDynamicIndexing,  nullptr, nullptr, nullptr, nullptr },
        { "shaderClipDistance",                     &VkPhysicalDeviceFeatures::shaderClipDistance,                      nullptr, nullptr, nullptr, nullptr },
        { "shaderCullDistance",                     &VkPhysicalDeviceFeatures::shaderCullDistance,                      nullptr, nullptr, nullptr, nullptr },
        { "shaderFloat64",                          &VkPhysicalDeviceFeatures::shaderFloat64,                           nullptr, nullptr, nullptr, nullptr },
        { "shaderInt64",                            &VkPhysicalDeviceFeatures::shaderInt64,                             nullptr, nullptr, nullptr, nullptr },
        { "shaderInt16",                            &VkPhysicalDeviceFeatures::shaderInt16,                             nullptr, nullptr, nullptr, nullptr },
        { "shaderResourceResidency",                &VkPhysicalDeviceFeatures::shaderResourceResidency,                 nullptr, nullptr, nullptr, nullptr },
        { "shaderResourceMinLod",                   &VkPhysicalDeviceFeatures::shaderResourceMinLod,                    nullptr, nullptr, nullptr, nullptr },
        { "sparseBinding",                          &VkPhysicalDeviceFeatures::sparseBinding,                           nullptr, nullptr, nullptr, nullptr },
        { "sparseResidencyBuffer",                  &VkPhysicalDeviceFeatures::sparseResidencyBuffer,                   nullptr, nullptr, nullptr, nullptr },
        { "sparseResidencyImage2D",                 &VkPhysicalDeviceFeatures::sparseResidencyImage2D,                  nullptr, nullptr, nullptr, nullptr },
        { "sparseResidencyImage3D",                 &VkPhysicalDeviceFeatures::sparseResidencyImage3D,                  nullptr, nullptr, nullptr, nullptr },
        { "sparseResidency2Samples",                &VkPhysicalDeviceFeatures::sparseResidency2Samples,                 nullptr, nullptr, nullptr, nullptr },
        { "sparseResidency4Samples",                &VkPhysicalDeviceFeatures::sparseResidency4Samples,                 nullptr, nullptr, nullptr, nullptr },
        { "sparseResidency8Samples",                &VkPhysicalDeviceFeatures::sparseResidency8Samples,                 nullptr, nullptr, nullptr, nullptr },
        { "sparseResidency16Samples",               &VkPhysicalDeviceFeatures::sparseResidency16Samples,                nullptr, nullptr, nullptr, nullptr },
        { "sparseResidencyAliased",                 &VkPhysicalDeviceFeatures::sparseResidencyAliased,                  nullptr, nullptr, nullptr, nullptr },
        { "variableMultisampleRate",                &VkPhysicalDeviceFeatures::variableMultisampleRate,                 nullptr, nullptr, nullptr, nullptr },
        { "inheritedQueries",                       &VkPhysicalDeviceFeatures::inheritedQueries,                        nullptr, nullptr, nullptr, nullptr },

        // ––– Vulkan 1.1 –––
        { "storageBuffer16BitAccess",             nullptr, &VkPhysicalDeviceVulkan11Features::storageBuffer16BitAccess,             nullptr, nullptr, nullptr },
        { "uniformAndStorageBuffer16BitAccess",   nullptr, &VkPhysicalDeviceVulkan11Features::uniformAndStorageBuffer16BitAccess,   nullptr, nullptr, nullptr },
        { "storagePushConstant16",                nullptr, &VkPhysicalDeviceVulkan11Features::storagePushConstant16,                nullptr, nullptr, nullptr },
        { "storageInputOutput16",                 nullptr, &VkPhysicalDeviceVulkan11Features::storageInputOutput16,                 nullptr, nullptr, nullptr },
        { "multiview",                            nullptr, &VkPhysicalDeviceVulkan11Features::multiview,                            nullptr, nullptr, nullptr },
        { "multiviewGeometryShader",              nullptr, &VkPhysicalDeviceVulkan11Features::multiviewGeometryShader,              nullptr, nullptr, nullptr },
        { "multiviewTessellationShader",          nullptr, &VkPhysicalDeviceVulkan11Features::multiviewTessellationShader,          nullptr, nullptr, nullptr },
        { "variablePointersStorageBuffer",        nullptr, &VkPhysicalDeviceVulkan11Features::variablePointersStorageBuffer,        nullptr, nullptr, nullptr },
        { "variablePointers",                     nullptr, &VkPhysicalDeviceVulkan11Features::variablePointers,                     nullptr, nullptr, nullptr },
        { "protectedMemory",                      nullptr, &VkPhysicalDeviceVulkan11Features::protectedMemory,                      nullptr, nullptr, nullptr },
        { "samplerYcbcrConversion",               nullptr, &VkPhysicalDeviceVulkan11Features::samplerYcbcrConversion,               nullptr, nullptr, nullptr },
        { "shaderDrawParameters",                 nullptr, &VkPhysicalDeviceVulkan11Features::shaderDrawParameters,                 nullptr, nullptr, nullptr },

        // ––– Vulkan 1.2 –––
        { "samplerMirrorClampToEdge",                           nullptr, nullptr, &VkPhysicalDeviceVulkan12Features::samplerMirrorClampToEdge,                          nullptr, nullptr },
        { "drawIndirectCount",                                  nullptr, nullptr, &VkPhysicalDeviceVulkan12Features::drawIndirectCount,                                 nullptr, nullptr },
        { "storageBuffer8BitAccess",                            nullptr, nullptr, &VkPhysicalDeviceVulkan12Features::storageBuffer8BitAccess,                           nullptr, nullptr },
        { "uniformAndStorageBuffer8BitAccess",                  nullptr, nullptr, &VkPhysicalDeviceVulkan12Features::uniformAndStorageBuffer8BitAccess,                 nullptr, nullptr },
        { "storagePushConstant8",                               nullptr, nullptr, &VkPhysicalDeviceVulkan12Features::storagePushConstant8,                              nullptr, nullptr },
        { "shaderBufferInt64Atomics",                           nullptr, nullptr, &VkPhysicalDeviceVulkan12Features::shaderBufferInt64Atomics,                          nullptr, nullptr },
        { "shaderSharedInt64Atomics",                           nullptr, nullptr, &VkPhysicalDeviceVulkan12Features::shaderSharedInt64Atomics,                          nullptr, nullptr },
        { "shaderFloat16",                                      nullptr, nullptr, &VkPhysicalDeviceVulkan12Features::shaderFloat16,                                     nullptr, nullptr },
        { "shaderInt8",                                         nullptr, nullptr, &VkPhysicalDeviceVulkan12Features::shaderInt8,                                        nullptr, nullptr },
        { "descriptorIndexing",                                 nullptr, nullptr, &VkPhysicalDeviceVulkan12Features::descriptorIndexing,                                nullptr, nullptr },
        { "shaderInputAttachmentArrayDynamicIndexing",          nullptr, nullptr, &VkPhysicalDeviceVulkan12Features::shaderInputAttachmentArrayDynamicIndexing,         nullptr, nullptr },
        { "shaderUniformTexelBufferArrayDynamicIndexing",       nullptr, nullptr, &VkPhysicalDeviceVulkan12Features::shaderUniformTexelBufferArrayDynamicIndexing,      nullptr, nullptr },
        { "shaderStorageTexelBufferArrayDynamicIndexing",       nullptr, nullptr, &VkPhysicalDeviceVulkan12Features::shaderStorageTexelBufferArrayDynamicIndexing,      nullptr, nullptr },
        { "shaderUniformBufferArrayNonUniformIndexing",         nullptr, nullptr, &VkPhysicalDeviceVulkan12Features::shaderUniformBufferArrayNonUniformIndexing,        nullptr, nullptr },
        { "shaderSampledImageArrayNonUniformIndexing",          nullptr, nullptr, &VkPhysicalDeviceVulkan12Features::shaderSampledImageArrayNonUniformIndexing,         nullptr, nullptr },
        { "shaderStorageBufferArrayNonUniformIndexing",         nullptr, nullptr, &VkPhysicalDeviceVulkan12Features::shaderStorageBufferArrayNonUniformIndexing,        nullptr, nullptr },
        { "shaderStorageImageArrayNonUniformIndexing",          nullptr, nullptr, &VkPhysicalDeviceVulkan12Features::shaderStorageImageArrayNonUniformIndexing,         nullptr, nullptr },
        { "shaderInputAttachmentArrayNonUniformIndexing",       nullptr, nullptr, &VkPhysicalDeviceVulkan12Features::shaderInputAttachmentArrayNonUniformIndexing,      nullptr, nullptr },
        { "descriptorBindingUniformBufferUpdateAfterBind",      nullptr, nullptr, &VkPhysicalDeviceVulkan12Features::descriptorBindingUniformBufferUpdateAfterBind,     nullptr, nullptr },
        { "descriptorBindingSampledImageUpdateAfterBind",       nullptr, nullptr, &VkPhysicalDeviceVulkan12Features::descriptorBindingSampledImageUpdateAfterBind,      nullptr, nullptr },
        { "descriptorBindingStorageImageUpdateAfterBind",       nullptr, nullptr, &VkPhysicalDeviceVulkan12Features::descriptorBindingStorageImageUpdateAfterBind,      nullptr, nullptr },
        { "descriptorBindingStorageBufferUpdateAfterBind",      nullptr, nullptr, &VkPhysicalDeviceVulkan12Features::descriptorBindingStorageBufferUpdateAfterBind,     nullptr, nullptr },
        { "descriptorBindingUniformTexelBufferUpdateAfterBind", nullptr, nullptr, &VkPhysicalDeviceVulkan12Features::descriptorBindingUniformTexelBufferUpdateAfterBind,nullptr, nullptr },
        { "descriptorBindingStorageTexelBufferUpdateAfterBind", nullptr, nullptr, &VkPhysicalDeviceVulkan12Features::descriptorBindingStorageTexelBufferUpdateAfterBind,nullptr, nullptr },
        { "descriptorBindingUpdateUnusedWhilePending",          nullptr, nullptr, &VkPhysicalDeviceVulkan12Features::descriptorBindingUpdateUnusedWhilePending,         nullptr, nullptr },
        { "descriptorBindingPartiallyBound",                    nullptr, nullptr, &VkPhysicalDeviceVulkan12Features::descriptorBindingPartiallyBound,                   nullptr, nullptr },
        { "descriptorBindingVariableDescriptorCount",           nullptr, nullptr, &VkPhysicalDeviceVulkan12Features::descriptorBindingVariableDescriptorCount,          nullptr, nullptr },
        { "runtimeDescriptorArray",                             nullptr, nullptr, &VkPhysicalDeviceVulkan12Features::runtimeDescriptorArray,                            nullptr, nullptr },
        { "samplerFilterMinmax",                                nullptr, nullptr, &VkPhysicalDeviceVulkan12Features::samplerFilterMinmax,                               nullptr, nullptr },
        { "scalarBlockLayout",                                  nullptr, nullptr, &VkPhysicalDeviceVulkan12Features::scalarBlockLayout,                                 nullptr, nullptr },
        { "imagelessFramebuffer",                               nullptr, nullptr, &VkPhysicalDeviceVulkan12Features::imagelessFramebuffer,                              nullptr, nullptr },
        { "uniformBufferStandardLayout",                        nullptr, nullptr, &VkPhysicalDeviceVulkan12Features::uniformBufferStandardLayout,                       nullptr, nullptr },
        { "shaderSubgroupExtendedTypes",                        nullptr, nullptr, &VkPhysicalDeviceVulkan12Features::shaderSubgroupExtendedTypes,                       nullptr, nullptr },
        { "separateDepthStencilLayouts",                        nullptr, nullptr, &VkPhysicalDeviceVulkan12Features::separateDepthStencilLayouts,                       nullptr, nullptr },
        { "hostQueryReset",                                     nullptr, nullptr, &VkPhysicalDeviceVulkan12Features::hostQueryReset,                                    nullptr, nullptr },
        { "timelineSemaphore",                                  nullptr, nullptr, &VkPhysicalDeviceVulkan12Features::timelineSemaphore,                                 nullptr, nullptr },
        { "bufferDeviceAddress",                                nullptr, nullptr, &VkPhysicalDeviceVulkan12Features::bufferDeviceAddress,                               nullptr, nullptr },
        { "bufferDeviceAddressCaptureReplay",                   nullptr, nullptr, &VkPhysicalDeviceVulkan12Features::bufferDeviceAddressCaptureReplay,                  nullptr, nullptr },
        { "bufferDeviceAddressMultiDevice",                     nullptr, nullptr, &VkPhysicalDeviceVulkan12Features::bufferDeviceAddressMultiDevice,                    nullptr, nullptr },
        { "vulkanMemoryModel",                                  nullptr, nullptr, &VkPhysicalDeviceVulkan12Features::vulkanMemoryModel,                                 nullptr, nullptr },
        { "vulkanMemoryModelDeviceScope",                       nullptr, nullptr, &VkPhysicalDeviceVulkan12Features::vulkanMemoryModelDeviceScope,                      nullptr, nullptr },
        { "vulkanMemoryModelAvailabilityVisibilityChains",      nullptr, nullptr, &VkPhysicalDeviceVulkan12Features::vulkanMemoryModelAvailabilityVisibilityChains,     nullptr, nullptr },
        { "shaderOutputViewportIndex",                          nullptr, nullptr, &VkPhysicalDeviceVulkan12Features::shaderOutputViewportIndex,                         nullptr, nullptr },
        { "shaderOutputLayer",                                  nullptr, nullptr, &VkPhysicalDeviceVulkan12Features::shaderOutputLayer,                                 nullptr, nullptr },
        { "subgroupBroadcastDynamicId",                         nullptr, nullptr, &VkPhysicalDeviceVulkan12Features::subgroupBroadcastDynamicId,                        nullptr, nullptr },

        // ––– Vulkan 1.3 –––
        { "robustImageAccess",                                  nullptr, nullptr, nullptr, &VkPhysicalDeviceVulkan13Features::robustImageAccess,                                    nullptr },
        { "inlineUniformBlock",                                 nullptr, nullptr, nullptr, &VkPhysicalDeviceVulkan13Features::inlineUniformBlock,                                   nullptr },
        { "descriptorBindingInlineUniformBlockUpdateAfterBind", nullptr, nullptr, nullptr, &VkPhysicalDeviceVulkan13Features::descriptorBindingInlineUniformBlockUpdateAfterBind,   nullptr },
        { "pipelineCreationCacheControl",                       nullptr, nullptr, nullptr, &VkPhysicalDeviceVulkan13Features::pipelineCreationCacheControl,                         nullptr },
        { "privateData",                                        nullptr, nullptr, nullptr, &VkPhysicalDeviceVulkan13Features::privateData,                                          nullptr },
        { "shaderDemoteToHelperInvocation",                     nullptr, nullptr, nullptr, &VkPhysicalDeviceVulkan13Features::shaderDemoteToHelperInvocation,                       nullptr },
        { "shaderTerminateInvocation",                          nullptr, nullptr, nullptr, &VkPhysicalDeviceVulkan13Features::shaderTerminateInvocation,                            nullptr },
        { "subgroupSizeControl",                                nullptr, nullptr, nullptr, &VkPhysicalDeviceVulkan13Features::subgroupSizeControl,                                  nullptr },
        { "computeFullSubgroups",                               nullptr, nullptr, nullptr, &VkPhysicalDeviceVulkan13Features::computeFullSubgroups,                                 nullptr },
        { "synchronization2",                                   nullptr, nullptr, nullptr, &VkPhysicalDeviceVulkan13Features::synchronization2,                                     nullptr },
        { "textureCompressionASTC_HDR",                         nullptr, nullptr, nullptr, &VkPhysicalDeviceVulkan13Features::textureCompressionASTC_HDR,                           nullptr },
        { "shaderZeroInitializeWorkgroupMemory",                nullptr, nullptr, nullptr, &VkPhysicalDeviceVulkan13Features::shaderZeroInitializeWorkgroupMemory,                  nullptr },
        { "dynamicRendering",                                   nullptr, nullptr, nullptr, &VkPhysicalDeviceVulkan13Features::dynamicRendering,                                     nullptr },
        { "shaderIntegerDotProduct",                            nullptr, nullptr, nullptr, &VkPhysicalDeviceVulkan13Features::shaderIntegerDotProduct,                              nullptr },
        { "maintenance4",                                       nullptr, nullptr, nullptr, &VkPhysicalDeviceVulkan13Features::maintenance4,                                         nullptr },

        // ––– Vulkan 1.4 –––
        { "globalPriorityQuery",                     nullptr, nullptr, nullptr, nullptr, &VkPhysicalDeviceVulkan14Features::globalPriorityQuery },
        { "shaderSubgroupRotate",                    nullptr, nullptr, nullptr, nullptr, &VkPhysicalDeviceVulkan14Features::shaderSubgroupRotate },
        { "shaderSubgroupRotateClustered",           nullptr, nullptr, nullptr, nullptr, &VkPhysicalDeviceVulkan14Features::shaderSubgroupRotateClustered },
        { "shaderFloatControls2",                    nullptr, nullptr, nullptr, nullptr, &VkPhysicalDeviceVulkan14Features::shaderFloatControls2 },
        { "shaderExpectAssume",                      nullptr, nullptr, nullptr, nullptr, &VkPhysicalDeviceVulkan14Features::shaderExpectAssume },
        { "rectangularLines",                        nullptr, nullptr, nullptr, nullptr, &VkPhysicalDeviceVulkan14Features::rectangularLines },
        { "bresenhamLines",                          nullptr, nullptr, nullptr, nullptr, &VkPhysicalDeviceVulkan14Features::bresenhamLines },
        { "smoothLines",                             nullptr, nullptr, nullptr, nullptr, &VkPhysicalDeviceVulkan14Features::smoothLines },
        { "stippledRectangularLines",                nullptr, nullptr, nullptr, nullptr, &VkPhysicalDeviceVulkan14Features::stippledRectangularLines },
        { "stippledBresenhamLines",                  nullptr, nullptr, nullptr, nullptr, &VkPhysicalDeviceVulkan14Features::stippledBresenhamLines },
        { "stippledSmoothLines",                     nullptr, nullptr, nullptr, nullptr, &VkPhysicalDeviceVulkan14Features::stippledSmoothLines },
        { "vertexAttributeInstanceRateDivisor",      nullptr, nullptr, nullptr, nullptr, &VkPhysicalDeviceVulkan14Features::vertexAttributeInstanceRateDivisor },
        { "vertexAttributeInstanceRateZeroDivisor",  nullptr, nullptr, nullptr, nullptr, &VkPhysicalDeviceVulkan14Features::vertexAttributeInstanceRateZeroDivisor },
        { "indexTypeUint8",                          nullptr, nullptr, nullptr, nullptr, &VkPhysicalDeviceVulkan14Features::indexTypeUint8 },
        { "dynamicRenderingLocalRead",               nullptr, nullptr, nullptr, nullptr, &VkPhysicalDeviceVulkan14Features::dynamicRenderingLocalRead },
        { "maintenance5",                            nullptr, nullptr, nullptr, nullptr, &VkPhysicalDeviceVulkan14Features::maintenance5 },
        { "maintenance6",                            nullptr, nullptr, nullptr, nullptr, &VkPhysicalDeviceVulkan14Features::maintenance6 },
        { "pipelineProtectedAccess",                 nullptr, nullptr, nullptr, nullptr, &VkPhysicalDeviceVulkan14Features::pipelineProtectedAccess },
        { "pipelineRobustness",                      nullptr, nullptr, nullptr, nullptr, &VkPhysicalDeviceVulkan14Features::pipelineRobustness },
        { "hostImageCopy",                           nullptr, nullptr, nullptr, nullptr, &VkPhysicalDeviceVulkan14Features::hostImageCopy },
        { "pushDescriptor",                          nullptr, nullptr, nullptr, nullptr, &VkPhysicalDeviceVulkan14Features::pushDescriptor },
    };

    struct QueueFamilyIndices {
        uint32_t graphicsFamilyIndex = UINT32_MAX;
        uint32_t presentFamilyIndex = UINT32_MAX;
        uint32_t computeFamilyIndex = UINT32_MAX;
        uint32_t transferFamilyIndex = UINT32_MAX;

        bool     graphicsFamilyFound = false;
        bool     presentFamilyFound = false;
        bool     computeFamilyFound = false;
        bool     transferFamilyFound = false;

        bool isGraphicsComplete() const { return graphicsFamilyFound && presentFamilyFound; }
        bool hasCompute()        const { return computeFamilyFound; }
        bool hasTransfer()       const { return transferFamilyFound; }
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    // Device enumerate & score
    std::vector<VkPhysicalDevice> EnumeratePhysicalDevices(VkInstance instance);
    int ScorePhysicalDevice(VkPhysicalDevice device);
    size_t GetDeviceVRAM_MB(VkPhysicalDevice device);

    // Check for support
    QueueFamilyIndices FindQueueFamilyIndices(VkPhysicalDevice device, VkSurfaceKHR surface = VK_NULL_HANDLE);
    bool CheckQueueFamilyMinimalSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
    SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface = VK_NULL_HANDLE);
    bool CheckExtensionSupport(VkPhysicalDevice device, const std::set<std::string>& requiredExtensions);
    bool CheckFeatureSupport(VkPhysicalDevice device, const std::set<std::string>& requiredFeatures);
    std::set<std::string> FilterRequiredFeatures(const std::set<std::string>& requiredFeatures);
    void GetPhysicalDeviceFeaturesName(const AllFeatures& vulkanFeatures, std::vector<std::string>* vulkanPhysicalDeviceFeaturesName);
    AllFeatures QueryAllFeatures(VkPhysicalDevice physicalDevice);
    AllFeatures BuildRequiredFeatureChain(AllFeatures avail, const std::vector<const char*>& requiredFeatureNames);
    bool HasFeature(const std::string& name, const AllFeatures& features);
}