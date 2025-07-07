#include "tool/HelpersVulkan.hpp"
#include <cassert>
#include <algorithm>

namespace mve {
    std::vector<VkPhysicalDevice> EnumeratePhysicalDevices(VkInstance instance) {
        uint32_t count = 0;
        vkEnumeratePhysicalDevices(instance, &count, nullptr);
        if (count == 0) throw std::runtime_error("No Vulkan-capable GPUs found");
        std::vector<VkPhysicalDevice> devices(count);
        vkEnumeratePhysicalDevices(instance, &count, devices.data());
        return devices;
    }

    int ScorePhysicalDevice(VkPhysicalDevice device) {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(device, &props);
        int score = 0;
        if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) score += 10000;   // Discrete device
        score += static_cast<int>(props.limits.maxImageDimension2D);                    // Max image dimension
        score += static_cast<int>(props.limits.maxSamplerAnisotropy * 100);             // Max sampler anisotropy
        score += static_cast<int>(GetDeviceVRAM_MB(device));                            // Max VRAM
#ifdef VULKAN_PHYSICAL_DEVICE_INFO_CHECK
        std::cout << "Device name: " << props.deviceName << " ID: " << props.deviceID << " Score: " << score << std::endl;
#endif
        return score;
    }

    size_t GetDeviceVRAM_MB(VkPhysicalDevice device) {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(device, &memProperties);

        size_t vramBytes = 0;
        for (uint32_t i = 0; i < memProperties.memoryHeapCount; ++i) {
            if (memProperties.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
                vramBytes += memProperties.memoryHeaps[i].size;
            }
        }
        return vramBytes / (1024 * 1024);
    }

    QueueFamilyIndices FindQueueFamilyIndices(VkPhysicalDevice device, VkSurfaceKHR surface) {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        for (uint32_t i = 0; i < queueFamilyCount; ++i) {
            const VkQueueFamilyProperties& queueFamily = queueFamilies[i];

            // Require both graphics and present on the same index
            if (!indices.graphicsFamilyFound && (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
                VkBool32 presentSupport = VK_FALSE;
                vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

                if (presentSupport) {
                    indices.graphicsFamilyIndex = i;
                    indices.presentFamilyIndex = i;
                    indices.graphicsFamilyFound = true;
                    indices.presentFamilyFound = true;
                }
            }
            // Compute support
            if (!indices.computeFamilyFound && (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)) {
                indices.computeFamilyIndex = i;
                indices.computeFamilyFound = true;
            }
            // Transfer support
            if (!indices.transferFamilyFound && (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT)) {
                indices.transferFamilyIndex = i;
                indices.transferFamilyFound = true;
            }
            if (indices.graphicsFamilyFound && indices.computeFamilyFound && indices.transferFamilyFound) break;
        }

        return indices;
    }

    bool CheckQueueFamilyMinimalSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        for (uint32_t i = 0; i < queueFamilyCount; ++i) {
            // Graphics support
            if ((queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) continue;

            // Present support
            VkBool32 presentSupport = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
            return (presentSupport == VK_TRUE);
        }
        return false;
    }

    SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
        assert(surface != VK_NULL_HANDLE && "Missing surface support for current physical device");
        SwapChainSupportDetails details;

        // Swap chain capability
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

        // Swap chain format
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
        }

        // Swap chain present modes
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }

    bool CheckExtensionSupport(VkPhysicalDevice device, const std::set<std::string>& requiredExtensions) {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

#ifdef VULKAN_PHYSICAL_DEVICE_INFO_CHECK
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(device, &props);
        std::cout << "\n//-Checking extensions: physical device " << props.deviceName << " id:" << props.deviceID << "-//\n";
        //std::cout << "Available extensions : ";
        //for (const auto& extension : availableExtensions) std::cout << extension.extensionName << ", ";
        //std::cout << "\nRequired extensions: ";
        //for (const auto& requiredExt : requiredExtensions) std::cout << requiredExt << ", ";
        //std::cout << std::endl;
#endif

        // Check that all required extensions are available
        std::set<std::string> missingExtensions = requiredExtensions;
        for (const auto& extension : availableExtensions) missingExtensions.erase(extension.extensionName);

#ifdef VULKAN_PHYSICAL_DEVICE_INFO_CHECK
        if (!missingExtensions.empty()) {
            std::cout << "Missing extensions: ";
            for (const auto& extension : missingExtensions) std::cout << extension << ", ";
            std::cout << std::endl;
        }
#endif

        return missingExtensions.empty();
    }

    bool CheckFeatureSupport(VkPhysicalDevice device, const std::set<std::string>& requiredFeatures) {
        std::set<std::string> requiredF = FilterRequiredFeatures(requiredFeatures);
        AllFeatures features = QueryAllFeatures(device);
        
        // Collect all features that are available on the physical device
        std::vector<std::string> availableFeatures;
        GetPhysicalDeviceFeaturesName(features, &availableFeatures);

#ifdef VULKAN_PHYSICAL_DEVICE_INFO_CHECK
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(device, &props);
        std::cout << "\n//-Checking features: physical device " << props.deviceName << " id:" << props.deviceID << "-//\n";
        //std::cout << "Available features: ";
        //for (const auto& feature : availableFeatures)  std::cout << feature << ", ";
        //std::cout << "\nRequired features: ";
        //for (const auto& feature : requiredF) std::cout << feature << ", ";
        //std::cout << std::endl;
#endif

        // Check that all required features are available
        std::set<std::string> missingFeatures = requiredF;
        for (const auto& feature : availableFeatures) missingFeatures.erase(feature);

#ifdef VULKAN_PHYSICAL_DEVICE_INFO_CHECK
        if (!missingFeatures.empty()) {
            std::cout << "Missing features: ";
            for (const auto& feature : missingFeatures) std::cout << feature << ", ";
            std::cout << std::endl;
        }
#endif

        return missingFeatures.empty();
    }

    std::set<std::string> FilterRequiredFeatures(const std::set<std::string>& requiredFeatures) {
        std::set<std::string> result;
        for (const std::string& f : requiredFeatures) {
            if (std::find(
                ALL_VULKAN_FEATURE_NAMES.begin(),
                ALL_VULKAN_FEATURE_NAMES.end(),
                f) != ALL_VULKAN_FEATURE_NAMES.end())
            {
                result.insert(f);
            }
            else std::cout << "Warning: required feature " << f << " ignored because it is not in the core feature 1.4-\n";
        }
        return result;
    }

    void GetPhysicalDeviceFeaturesName(const AllFeatures& vulkanFeatures, std::vector<std::string>* vulkanPhysicalDeviceFeaturesName) {
        for (const auto& name : ALL_VULKAN_FEATURE_NAMES) {
            if (HasFeature(name, vulkanFeatures)) {
                vulkanPhysicalDeviceFeaturesName->push_back(name);
            }
        }
    }

    AllFeatures QueryAllFeatures(VkPhysicalDevice physicalDevice) {
        // 1) Set up the head of the chain
        AllFeatures out;

        // 2) Link up the pNext chain
        out.core.pNext = &out.v11;
        out.v11.pNext = &out.v12;
        out.v12.pNext = &out.v13;
        out.v13.pNext = &out.v14;
        out.v14.pNext = nullptr;

        // 3) Query!
        vkGetPhysicalDeviceFeatures2(physicalDevice, &out.core);

        return out;
    }

    AllFeatures BuildRequiredFeatureChain(AllFeatures avail, const std::vector<const char*>& requiredFeatureNames) {
        // start with request all-FALSE
        AllFeatures req{};

        // chain request
        req.core.pNext = &req.v11;
        req.v11.pNext = &req.v12;
        req.v12.pNext = &req.v13;
        req.v13.pNext = &req.v14;
        req.v14.pNext = nullptr;

        // enable all requested features available
        for (const char* name : requiredFeatureNames) {
            const FeatureMap* it = std::find_if(
                std::begin(FEATURE_TABLE),
                std::end(FEATURE_TABLE),
                [&](const FeatureMap& e) { return std::strcmp(name, e.name) == 0; }
            );
            if (it == std::end(FEATURE_TABLE)) continue;  // unknown/typo

            // Core
            if (it->core && avail.core.features.*(it->core))
                req.core.features.*(it->core) = VK_TRUE;
            // Vulkan 1.1
            if (it->v11 && avail.v11.*(it->v11))
                req.v11.*(it->v11) = VK_TRUE;
            // Vulkan 1.2
            if (it->v12 && avail.v12.*(it->v12))
                req.v12.*(it->v12) = VK_TRUE;
            // Vulkan 1.3
            if (it->v13 && avail.v13.*(it->v13))
                req.v13.*(it->v13) = VK_TRUE;
            // Vulkan 1.4
            if (it->v14 && avail.v14.*(it->v14))
                req.v14.*(it->v14) = VK_TRUE;
        }

        return req;
    }

    // Query any single named feature across core + 1.1 - 1.4
    bool HasFeature(
        const std::string& name,
        const AllFeatures& features2)
    {
        // ––– Core 1.0 –––
        if (name == "robustBufferAccess")                           return features2.core.features.robustBufferAccess;
        if (name == "fullDrawIndexUint32")                          return features2.core.features.fullDrawIndexUint32;
        if (name == "imageCubeArray")                               return features2.core.features.imageCubeArray;
        if (name == "independentBlend")                             return features2.core.features.independentBlend;
        if (name == "geometryShader")                               return features2.core.features.geometryShader;
        if (name == "tessellationShader")                           return features2.core.features.tessellationShader;
        if (name == "sampleRateShading")                            return features2.core.features.sampleRateShading;
        if (name == "dualSrcBlend")                                 return features2.core.features.dualSrcBlend;
        if (name == "logicOp")                                      return features2.core.features.logicOp;
        if (name == "multiDrawIndirect")                            return features2.core.features.multiDrawIndirect;
        if (name == "drawIndirectFirstInstance")                    return features2.core.features.drawIndirectFirstInstance;
        if (name == "depthClamp")                                   return features2.core.features.depthClamp;
        if (name == "depthBiasClamp")                               return features2.core.features.depthBiasClamp;
        if (name == "fillModeNonSolid")                             return features2.core.features.fillModeNonSolid;
        if (name == "depthBounds")                                  return features2.core.features.depthBounds;
        if (name == "wideLines")                                    return features2.core.features.wideLines;
        if (name == "largePoints")                                  return features2.core.features.largePoints;
        if (name == "alphaToOne")                                   return features2.core.features.alphaToOne;
        if (name == "multiViewport")                                return features2.core.features.multiViewport;
        if (name == "samplerAnisotropy")                            return features2.core.features.samplerAnisotropy;
        if (name == "textureCompressionETC2")                       return features2.core.features.textureCompressionETC2;
        if (name == "textureCompressionASTC_LDR")                   return features2.core.features.textureCompressionASTC_LDR;
        if (name == "textureCompressionBC")                         return features2.core.features.textureCompressionBC;
        if (name == "occlusionQueryPrecise")                        return features2.core.features.occlusionQueryPrecise;
        if (name == "pipelineStatisticsQuery")                      return features2.core.features.pipelineStatisticsQuery;
        if (name == "vertexPipelineStoresAndAtomics")               return features2.core.features.vertexPipelineStoresAndAtomics;
        if (name == "fragmentStoresAndAtomics")                     return features2.core.features.fragmentStoresAndAtomics;
        if (name == "shaderTessellationAndGeometryPointSize")       return features2.core.features.shaderTessellationAndGeometryPointSize;
        if (name == "shaderImageGatherExtended")                    return features2.core.features.shaderImageGatherExtended;
        if (name == "shaderStorageImageExtendedFormats")            return features2.core.features.shaderStorageImageExtendedFormats;
        if (name == "shaderStorageImageMultisample")                return features2.core.features.shaderStorageImageMultisample;
        if (name == "shaderStorageImageReadWithoutFormat")          return features2.core.features.shaderStorageImageReadWithoutFormat;
        if (name == "shaderStorageImageWriteWithoutFormat")         return features2.core.features.shaderStorageImageWriteWithoutFormat;
        if (name == "shaderUniformBufferArrayDynamicIndexing")      return features2.core.features.shaderUniformBufferArrayDynamicIndexing;
        if (name == "shaderSampledImageArrayDynamicIndexing")       return features2.core.features.shaderSampledImageArrayDynamicIndexing;
        if (name == "shaderStorageBufferArrayDynamicIndexing")      return features2.core.features.shaderStorageBufferArrayDynamicIndexing;
        if (name == "shaderStorageImageArrayDynamicIndexing")       return features2.core.features.shaderStorageImageArrayDynamicIndexing;
        if (name == "shaderClipDistance")                           return features2.core.features.shaderClipDistance;
        if (name == "shaderCullDistance")                           return features2.core.features.shaderCullDistance;
        if (name == "shaderFloat64")                                return features2.core.features.shaderFloat64;
        if (name == "shaderInt64")                                  return features2.core.features.shaderInt64;
        if (name == "shaderInt16")                                  return features2.core.features.shaderInt16;
        if (name == "shaderResourceResidency")                      return features2.core.features.shaderResourceResidency;
        if (name == "shaderResourceMinLod")                         return features2.core.features.shaderResourceMinLod;
        if (name == "sparseBinding")                                return features2.core.features.sparseBinding;
        if (name == "sparseResidencyBuffer")                        return features2.core.features.sparseResidencyBuffer;
        if (name == "sparseResidencyImage2D")                       return features2.core.features.sparseResidencyImage2D;
        if (name == "sparseResidencyImage3D")                       return features2.core.features.sparseResidencyImage3D;
        if (name == "sparseResidency2Samples")                      return features2.core.features.sparseResidency2Samples;
        if (name == "sparseResidency4Samples")                      return features2.core.features.sparseResidency4Samples;
        if (name == "sparseResidency8Samples")                      return features2.core.features.sparseResidency8Samples;
        if (name == "sparseResidency16Samples")                     return features2.core.features.sparseResidency16Samples;
        if (name == "sparseResidencyAliased")                       return features2.core.features.sparseResidencyAliased;
        if (name == "variableMultisampleRate")                      return features2.core.features.variableMultisampleRate;
        if (name == "inheritedQueries")                             return features2.core.features.inheritedQueries;

        // ––– Vulkan 1.1 –––
        if (name == "storageBuffer16BitAccess")                    return features2.v11.storageBuffer16BitAccess;
        if (name == "uniformAndStorageBuffer16BitAccess")          return features2.v11.uniformAndStorageBuffer16BitAccess;
        if (name == "storagePushConstant16")                       return features2.v11.storagePushConstant16;
        if (name == "storageInputOutput16")                        return features2.v11.storageInputOutput16;
        if (name == "multiview")                                   return features2.v11.multiview;
        if (name == "multiviewGeometryShader")                     return features2.v11.multiviewGeometryShader;
        if (name == "multiviewTessellationShader")                 return features2.v11.multiviewTessellationShader;
        if (name == "variablePointersStorageBuffer")               return features2.v11.variablePointersStorageBuffer;
        if (name == "variablePointers")                            return features2.v11.variablePointers;
        if (name == "protectedMemory")                             return features2.v11.protectedMemory;
        if (name == "samplerYcbcrConversion")                      return features2.v11.samplerYcbcrConversion;
        if (name == "shaderDrawParameters")                        return features2.v11.shaderDrawParameters;

        // ––– Vulkan 1.2 –––
        if (name == "samplerMirrorClampToEdge")                    return features2.v12.samplerMirrorClampToEdge;
        if (name == "drawIndirectCount")                           return features2.v12.drawIndirectCount;
        if (name == "storageBuffer8BitAccess")                     return features2.v12.storageBuffer8BitAccess;
        if (name == "uniformAndStorageBuffer8BitAccess")           return features2.v12.uniformAndStorageBuffer8BitAccess;
        if (name == "storagePushConstant8")                        return features2.v12.storagePushConstant8;
        if (name == "shaderBufferInt64Atomics")                    return features2.v12.shaderBufferInt64Atomics;
        if (name == "shaderSharedInt64Atomics")                    return features2.v12.shaderSharedInt64Atomics;
        if (name == "shaderFloat16")                               return features2.v12.shaderFloat16;
        if (name == "shaderInt8")                                  return features2.v12.shaderInt8;
        if (name == "descriptorIndexing")                          return features2.v12.descriptorIndexing;
        if (name == "shaderInputAttachmentArrayDynamicIndexing")   return features2.v12.shaderInputAttachmentArrayDynamicIndexing;
        if (name == "shaderUniformTexelBufferArrayDynamicIndexing")return features2.v12.shaderUniformTexelBufferArrayDynamicIndexing;
        if (name == "shaderStorageTexelBufferArrayDynamicIndexing")return features2.v12.shaderStorageTexelBufferArrayDynamicIndexing;
        if (name == "shaderUniformBufferArrayNonUniformIndexing")  return features2.v12.shaderUniformBufferArrayNonUniformIndexing;
        if (name == "shaderSampledImageArrayNonUniformIndexing")   return features2.v12.shaderSampledImageArrayNonUniformIndexing;
        if (name == "shaderStorageBufferArrayNonUniformIndexing")  return features2.v12.shaderStorageBufferArrayNonUniformIndexing;
        if (name == "shaderStorageImageArrayNonUniformIndexing")   return features2.v12.shaderStorageImageArrayNonUniformIndexing;
        if (name == "shaderInputAttachmentArrayNonUniformIndexing")return features2.v12.shaderInputAttachmentArrayNonUniformIndexing;
        if (name == "descriptorBindingUniformBufferUpdateAfterBind")return features2.v12.descriptorBindingUniformBufferUpdateAfterBind;
        if (name == "descriptorBindingSampledImageUpdateAfterBind") return features2.v12.descriptorBindingSampledImageUpdateAfterBind;
        if (name == "descriptorBindingStorageImageUpdateAfterBind") return features2.v12.descriptorBindingStorageImageUpdateAfterBind;
        if (name == "descriptorBindingStorageBufferUpdateAfterBind")return features2.v12.descriptorBindingStorageBufferUpdateAfterBind;
        if (name == "descriptorBindingUniformTexelBufferUpdateAfterBind")return features2.v12.descriptorBindingUniformTexelBufferUpdateAfterBind;
        if (name == "descriptorBindingStorageTexelBufferUpdateAfterBind")return features2.v12.descriptorBindingStorageTexelBufferUpdateAfterBind;
        if (name == "descriptorBindingUpdateUnusedWhilePending")   return features2.v12.descriptorBindingUpdateUnusedWhilePending;
        if (name == "descriptorBindingPartiallyBound")             return features2.v12.descriptorBindingPartiallyBound;
        if (name == "descriptorBindingVariableDescriptorCount")    return features2.v12.descriptorBindingVariableDescriptorCount;
        if (name == "runtimeDescriptorArray")                      return features2.v12.runtimeDescriptorArray;
        if (name == "samplerFilterMinmax")                         return features2.v12.samplerFilterMinmax;
        if (name == "scalarBlockLayout")                           return features2.v12.scalarBlockLayout;
        if (name == "imagelessFramebuffer")                        return features2.v12.imagelessFramebuffer;
        if (name == "uniformBufferStandardLayout")                 return features2.v12.uniformBufferStandardLayout;
        if (name == "shaderSubgroupExtendedTypes")                 return features2.v12.shaderSubgroupExtendedTypes;
        if (name == "separateDepthStencilLayouts")                 return features2.v12.separateDepthStencilLayouts;
        if (name == "hostQueryReset")                              return features2.v12.hostQueryReset;
        if (name == "timelineSemaphore")                           return features2.v12.timelineSemaphore;
        if (name == "bufferDeviceAddress")                         return features2.v12.bufferDeviceAddress;
        if (name == "bufferDeviceAddressCaptureReplay")            return features2.v12.bufferDeviceAddressCaptureReplay;
        if (name == "bufferDeviceAddressMultiDevice")              return features2.v12.bufferDeviceAddressMultiDevice;
        if (name == "vulkanMemoryModel")                           return features2.v12.vulkanMemoryModel;
        if (name == "vulkanMemoryModelDeviceScope")                return features2.v12.vulkanMemoryModelDeviceScope;
        if (name == "vulkanMemoryModelAvailabilityVisibilityChains")return features2.v12.vulkanMemoryModelAvailabilityVisibilityChains;
        if (name == "shaderOutputViewportIndex")                   return features2.v12.shaderOutputViewportIndex;
        if (name == "shaderOutputLayer")                           return features2.v12.shaderOutputLayer;
        if (name == "subgroupBroadcastDynamicId")                  return features2.v12.subgroupBroadcastDynamicId;

        // ––– Vulkan 1.3 –––
        if (name == "robustImageAccess")                           return features2.v13.robustImageAccess;
        if (name == "inlineUniformBlock")                          return features2.v13.inlineUniformBlock;
        if (name == "descriptorBindingInlineUniformBlockUpdateAfterBind")return features2.v13.descriptorBindingInlineUniformBlockUpdateAfterBind;
        if (name == "pipelineCreationCacheControl")                return features2.v13.pipelineCreationCacheControl;
        if (name == "privateData")                                 return features2.v13.privateData;
        if (name == "shaderDemoteToHelperInvocation")              return features2.v13.shaderDemoteToHelperInvocation;
        if (name == "shaderTerminateInvocation")                   return features2.v13.shaderTerminateInvocation;
        if (name == "subgroupSizeControl")                         return features2.v13.subgroupSizeControl;
        if (name == "computeFullSubgroups")                        return features2.v13.computeFullSubgroups;
        if (name == "synchronization2")                            return features2.v13.synchronization2;
        if (name == "textureCompressionASTC_HDR")                  return features2.v13.textureCompressionASTC_HDR;
        if (name == "shaderZeroInitializeWorkgroupMemory")         return features2.v13.shaderZeroInitializeWorkgroupMemory;
        if (name == "dynamicRendering")                            return features2.v13.dynamicRendering;
        if (name == "shaderIntegerDotProduct")                     return features2.v13.shaderIntegerDotProduct;
        if (name == "maintenance4")                                return features2.v13.maintenance4;

        // ––– Vulkan 1.4 –––
        if (name == "globalPriorityQuery")                         return features2.v14.globalPriorityQuery;
        if (name == "shaderSubgroupRotate")                        return features2.v14.shaderSubgroupRotate;
        if (name == "shaderSubgroupRotateClustered")               return features2.v14.shaderSubgroupRotateClustered;
        if (name == "shaderFloatControls2")                        return features2.v14.shaderFloatControls2;
        if (name == "shaderExpectAssume")                          return features2.v14.shaderExpectAssume;
        if (name == "rectangularLines")                            return features2.v14.rectangularLines;
        if (name == "bresenhamLines")                              return features2.v14.bresenhamLines;
        if (name == "smoothLines")                                 return features2.v14.smoothLines;
        if (name == "stippledRectangularLines")                    return features2.v14.stippledRectangularLines;
        if (name == "stippledBresenhamLines")                      return features2.v14.stippledBresenhamLines;
        if (name == "stippledSmoothLines")                         return features2.v14.stippledSmoothLines;
        if (name == "vertexAttributeInstanceRateDivisor")          return features2.v14.vertexAttributeInstanceRateDivisor;
        if (name == "vertexAttributeInstanceRateZeroDivisor")      return features2.v14.vertexAttributeInstanceRateZeroDivisor;
        if (name == "indexTypeUint8")                              return features2.v14.indexTypeUint8;
        if (name == "dynamicRenderingLocalRead")                   return features2.v14.dynamicRenderingLocalRead;
        if (name == "maintenance5")                                return features2.v14.maintenance5;
        if (name == "maintenance6")                                return features2.v14.maintenance6;
        if (name == "pipelineProtectedAccess")                     return features2.v14.pipelineProtectedAccess;
        if (name == "pipelineRobustness")                          return features2.v14.pipelineRobustness;
        if (name == "hostImageCopy")                               return features2.v14.hostImageCopy;
        if (name == "pushDescriptor")                              return features2.v14.pushDescriptor;

        return false;
    }

}