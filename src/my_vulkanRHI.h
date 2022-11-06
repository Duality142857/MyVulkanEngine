#pragma once
#include <vulkan/vulkan.h>
#include<GLFW/glfw3.h>
// #include<GLFW/glfw3.h>
#include<optional>
#include<vector>
#include"my_window.h"
#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif


struct QueueFamilyIndices 
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    std::optional<uint32_t> computeFamily;

    bool isComplete() 
    {
        return graphicsFamily.has_value() && presentFamily.has_value()&& computeFamily.has_value();
    }
};
struct SwapChainSupportDetails 
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class MyVulkanRHI
{
public:
    const int maxFramesInFlight=2;
    uint32_t imageCount=1;

    VkDebugUtilsMessengerEXT debugMessenger;
    MyWindow* mywindow;
    VkInstance instance;
    VkDevice device;
    VkPhysicalDevice physicalDevice=VK_NULL_HANDLE;
    VkSurfaceKHR surface;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkQueue computeQueue;
    VkSwapchainKHR swapChain;
    VkExtent2D swapChainExtent;
    VkExtent2D shadowBufferExtent{2048,2048};
    VkFormat imageFormat;

    std::vector<VkImage> images;
    std::vector<VkImageView> imageViews;

    std::vector<VkImage> depthImages;
    std::vector<VkDeviceMemory> depthImageMemorys;
    std::vector<VkImageView> depthImageViews;

    VkFramebuffer shadowFrameBuffer;
    VkImage shadowImage;
    VkImageView shadowImageView;
    VkDeviceMemory shadowImageMemory;
    VkSampler shadowImageSampler;

    VkRenderPass renderPass;
    VkRenderPass shadowRenderpass;
    // VkRenderPass skyboxPass;
    std::vector<VkFramebuffer> framebuffers;


    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;

    std::vector<VkFence> frameFences;

    QueueFamilyIndices queueFamilyIndices;
    VkCommandPool commandPool;
    const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};
    const std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

public:
    MyVulkanRHI(MyWindow* _mywindow):mywindow{_mywindow}
    {
        init();
    }
    void init();
    void cleanup();
    void createInstance();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createCommandPool();

    void createSwapChain();
    void createRenderpass();
    void createSyncObjects();
    void initSwapChain();

    void recreateSwapChain();

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    bool isDeviceSuitable(VkPhysicalDevice device);
    
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    
    bool checkValidationLayerSupport();

    std::vector<const char *> getRequiredExtensions();

    void setupDebugMessenger();

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) 
    {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }

    static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) 
    {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) 
        {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        } 
        else 
        {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) 
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(instance, debugMessenger, pAllocator);
        }
    }

    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    
    void copyBuffer2host(VkBuffer srcBuffer, void* data, VkDeviceSize size);

    void createImageWithInfo(VkImageCreateInfo& imageInfo, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

    void createImageViews(std::vector<VkImageView>& imageViews,std::vector<VkImage>& images, VkFormat imageFormat);

    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

    void createDataBuffer(void* vd, VkDeviceSize bufferSize, VkBuffer& buffer, VkDeviceMemory& memory, VkBufferUsageFlagBits bufferUsageFlagBit);

    void updateDataBuffer(void* vd,VkDeviceSize bufferSize, VkBuffer& buffer, VkDeviceMemory& memory);

    void createDepthResources();
    VkFormat findDepthFormat();
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

    void createFramebuffers();
    void createShadowRenderpass();

    void createShadowDepthBuffer();

    void clearSwapChain();
    void clearSyncObjects();



};