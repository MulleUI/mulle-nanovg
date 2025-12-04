// Minimal working Vulkan offscreen framebuffer demo with NanoVG
// Blue offscreen texture displayed on red background

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define NANOVG_VULKAN_IMPLEMENTATION
#include <nanovg_vk.h>

#define WIDTH 800
#define HEIGHT 600
#define OFFSCREEN_SIZE 256

typedef struct {
    VkInstance instance;
    VkPhysicalDevice gpu;
    VkDevice device;
    uint32_t queueFamily;
    VkQueue queue;
    VkSurfaceKHR surface;
    VkSwapchainKHR swapchain;
    VkImage *swapImages;
    VkImageView *swapViews;
    uint32_t imageCount;
    VkFormat swapFormat;
    VkRenderPass renderPass;
    VkFramebuffer *framebuffers;
    VkCommandPool cmdPool;
    VkCommandBuffer cmdBuf;
    VkSemaphore imageAvail, renderDone;
    VkFence fence;
    
    VkImage offscreenImg;
    VkDeviceMemory offscreenMem;
    VkImageView offscreenView;
    VkFramebuffer offscreenFB;
    VkRenderPass offscreenRP;
    VkSampler sampler;
    
    VkDescriptorSetLayout descLayout;
    VkDescriptorPool descPool;
    VkDescriptorSet descSet;
    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;
    
    NVGcontext *vg;
    NVGcontext *vgScreen;
    int offscreenImgId;
} VkCtx;

static uint32_t findMemType(VkPhysicalDevice gpu, uint32_t filter, VkMemoryPropertyFlags props) {
    VkPhysicalDeviceMemoryProperties mem;
    vkGetPhysicalDeviceMemoryProperties(gpu, &mem);
    for (uint32_t i = 0; i < mem.memoryTypeCount; i++) {
        if ((filter & (1 << i)) && (mem.memoryTypes[i].propertyFlags & props) == props)
            return i;
    }
    return 0;
}

static uint32_t findQueueFamily(VkPhysicalDevice gpu, VkSurfaceKHR surface) {
    uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &count, NULL);
    VkQueueFamilyProperties *props = malloc(count * sizeof(*props));
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &count, props);
    
    for (uint32_t i = 0; i < count; i++) {
        VkBool32 surfaceSupport = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(gpu, i, surface, &surfaceSupport);
        if ((props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && surfaceSupport) {
            free(props);
            return i;
        }
    }
    free(props);
    return 0;
}

static void initVulkan(VkCtx *ctx, GLFWwindow *window) {
    printf("Initializing Vulkan...\n");
    // Instance
    VkApplicationInfo appInfo = {0};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.apiVersion = VK_API_VERSION_1_0;
    
    uint32_t glfwExtCount = 0;
    const char **glfwExts = glfwGetRequiredInstanceExtensions(&glfwExtCount);
    
    VkInstanceCreateInfo instInfo = {0};
    instInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instInfo.pApplicationInfo = &appInfo;
    instInfo.enabledExtensionCount = glfwExtCount;
    instInfo.ppEnabledExtensionNames = glfwExts;
    
    VkResult res = vkCreateInstance(&instInfo, NULL, &ctx->instance);
    if (res != VK_SUCCESS) {
        printf("Failed to create instance: %d\n", res);
        return;
    }
    printf("Instance created\n");
    
    // Surface
    res = glfwCreateWindowSurface(ctx->instance, window, NULL, &ctx->surface);
    if (res != VK_SUCCESS) {
        printf("Failed to create surface: %d\n", res);
        return;
    }
    printf("Surface created\n");
    
    // Physical device
    uint32_t gpuCount = 0;
    vkEnumeratePhysicalDevices(ctx->instance, &gpuCount, NULL);
    printf("Found %d GPUs\n", gpuCount);
    VkPhysicalDevice *gpus = malloc(gpuCount * sizeof(VkPhysicalDevice));
    vkEnumeratePhysicalDevices(ctx->instance, &gpuCount, gpus);
    ctx->gpu = gpus[0];
    free(gpus);
    
    ctx->queueFamily = findQueueFamily(ctx->gpu, ctx->surface);
    printf("Queue family: %d\n", ctx->queueFamily);
    
    // Logical device
    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueInfo = {0};
    queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo.queueFamilyIndex = ctx->queueFamily;
    queueInfo.queueCount = 1;
    queueInfo.pQueuePriorities = &queuePriority;
    
    const char *devExts[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    
    VkDeviceCreateInfo devInfo = {0};
    devInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    devInfo.queueCreateInfoCount = 1;
    devInfo.pQueueCreateInfos = &queueInfo;
    devInfo.enabledExtensionCount = 1;
    devInfo.ppEnabledExtensionNames = devExts;
    
    res = vkCreateDevice(ctx->gpu, &devInfo, NULL, &ctx->device);
    if (res != VK_SUCCESS) {
        printf("Failed to create device: %d\n", res);
        return;
    }
    printf("Device created\n");
    vkGetDeviceQueue(ctx->device, ctx->queueFamily, 0, &ctx->queue);
    
    printf("Vulkan initialized\n");
}

static void createSwapchain(VkCtx *ctx) {
    VkSurfaceCapabilitiesKHR caps;
    VkResult res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(ctx->gpu, ctx->surface, &caps);
    if (res != VK_SUCCESS) {
        printf("Failed to get surface capabilities: %d\n", res);
        ctx->swapchain = VK_NULL_HANDLE;
        return;
    }
    printf("Surface extent: %dx%d, min images: %d, max images: %d\n", 
           caps.currentExtent.width, caps.currentExtent.height,
           caps.minImageCount, caps.maxImageCount);
    
    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(ctx->gpu, ctx->surface, &formatCount, NULL);
    printf("Found %d surface formats\n", formatCount);
    VkSurfaceFormatKHR *formats = malloc(formatCount * sizeof(*formats));
    vkGetPhysicalDeviceSurfaceFormatsKHR(ctx->gpu, ctx->surface, &formatCount, formats);
    ctx->swapFormat = formats[0].format;
    VkColorSpaceKHR colorSpace = formats[0].colorSpace;
    printf("Using format: %d, colorspace: %d\n", ctx->swapFormat, colorSpace);
    free(formats);
    
    uint32_t imageCount = caps.minImageCount + 1;
    if (caps.maxImageCount > 0 && imageCount > caps.maxImageCount) {
        imageCount = caps.maxImageCount;
    }
    
    VkExtent2D extent = caps.currentExtent;
    if (extent.width == (uint32_t)-1) {
        extent.width = WIDTH;
        extent.height = HEIGHT;
    }
    printf("Using extent: %dx%d\n", extent.width, extent.height);
    
    VkSwapchainCreateInfoKHR swapInfo = {0};
    swapInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapInfo.surface = ctx->surface;
    swapInfo.minImageCount = imageCount;
    swapInfo.imageFormat = ctx->swapFormat;
    swapInfo.imageColorSpace = colorSpace;
    swapInfo.imageExtent = extent;
    swapInfo.imageArrayLayers = 1;
    swapInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapInfo.preTransform = caps.currentTransform;
    swapInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    swapInfo.clipped = VK_TRUE;
    swapInfo.oldSwapchain = VK_NULL_HANDLE;
    
    res = vkCreateSwapchainKHR(ctx->device, &swapInfo, NULL, &ctx->swapchain);
    if (res != VK_SUCCESS) {
        printf("Failed to create swapchain: %d\n", res);
        ctx->swapchain = VK_NULL_HANDLE;
        return;
    }
    
    vkGetSwapchainImagesKHR(ctx->device, ctx->swapchain, &ctx->imageCount, NULL);
    ctx->swapImages = malloc(ctx->imageCount * sizeof(VkImage));
    ctx->swapViews = malloc(ctx->imageCount * sizeof(VkImageView));
    vkGetSwapchainImagesKHR(ctx->device, ctx->swapchain, &ctx->imageCount, ctx->swapImages);
    
    for (uint32_t i = 0; i < ctx->imageCount; i++) {
        VkImageViewCreateInfo viewInfo = {0};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = ctx->swapImages[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = ctx->swapFormat;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.layerCount = 1;
        vkCreateImageView(ctx->device, &viewInfo, NULL, &ctx->swapViews[i]);
    }
    
    printf("Swapchain created: %d images\n", ctx->imageCount);
}

static void createRenderPass(VkCtx *ctx) {
    VkAttachmentDescription attach = {0};
    attach.format = ctx->swapFormat;
    attach.samples = VK_SAMPLE_COUNT_1_BIT;
    attach.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    
    VkAttachmentReference colorRef = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    
    VkSubpassDescription subpass = {0};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorRef;
    
    VkRenderPassCreateInfo rpInfo = {0};
    rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rpInfo.attachmentCount = 1;
    rpInfo.pAttachments = &attach;
    rpInfo.subpassCount = 1;
    rpInfo.pSubpasses = &subpass;
    
    vkCreateRenderPass(ctx->device, &rpInfo, NULL, &ctx->renderPass);
    
    ctx->framebuffers = malloc(ctx->imageCount * sizeof(VkFramebuffer));
    for (uint32_t i = 0; i < ctx->imageCount; i++) {
        VkFramebufferCreateInfo fbInfo = {0};
        fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbInfo.renderPass = ctx->renderPass;
        fbInfo.attachmentCount = 1;
        fbInfo.pAttachments = &ctx->swapViews[i];
        fbInfo.width = WIDTH;
        fbInfo.height = HEIGHT;
        fbInfo.layers = 1;
        vkCreateFramebuffer(ctx->device, &fbInfo, NULL, &ctx->framebuffers[i]);
    }
}

static void createOffscreen(VkCtx *ctx) {
    // Create image
    VkImageCreateInfo imgInfo = {0};
    imgInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imgInfo.imageType = VK_IMAGE_TYPE_2D;
    imgInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imgInfo.extent.width = OFFSCREEN_SIZE;
    imgInfo.extent.height = OFFSCREEN_SIZE;
    imgInfo.extent.depth = 1;
    imgInfo.mipLevels = 1;
    imgInfo.arrayLayers = 1;
    imgInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imgInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imgInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imgInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    
    vkCreateImage(ctx->device, &imgInfo, NULL, &ctx->offscreenImg);
    
    VkMemoryRequirements memReq;
    vkGetImageMemoryRequirements(ctx->device, ctx->offscreenImg, &memReq);
    
    VkMemoryAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReq.size;
    allocInfo.memoryTypeIndex = findMemType(ctx->gpu, memReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    
    vkAllocateMemory(ctx->device, &allocInfo, NULL, &ctx->offscreenMem);
    vkBindImageMemory(ctx->device, ctx->offscreenImg, ctx->offscreenMem, 0);
    
    // Create image view
    VkImageViewCreateInfo viewInfo = {0};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = ctx->offscreenImg;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.layerCount = 1;
    
    vkCreateImageView(ctx->device, &viewInfo, NULL, &ctx->offscreenView);
    
    // Create render pass
    VkAttachmentDescription attach = {0};
    attach.format = VK_FORMAT_R8G8B8A8_UNORM;
    attach.samples = VK_SAMPLE_COUNT_1_BIT;
    attach.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attach.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    
    VkAttachmentReference colorRef = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    
    VkSubpassDescription subpass = {0};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorRef;
    
    VkRenderPassCreateInfo rpInfo = {0};
    rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rpInfo.attachmentCount = 1;
    rpInfo.pAttachments = &attach;
    rpInfo.subpassCount = 1;
    rpInfo.pSubpasses = &subpass;
    
    vkCreateRenderPass(ctx->device, &rpInfo, NULL, &ctx->offscreenRP);
    
    // Create framebuffer
    VkFramebufferCreateInfo fbInfo = {0};
    fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fbInfo.renderPass = ctx->offscreenRP;
    fbInfo.attachmentCount = 1;
    fbInfo.pAttachments = &ctx->offscreenView;
    fbInfo.width = OFFSCREEN_SIZE;
    fbInfo.height = OFFSCREEN_SIZE;
    fbInfo.layers = 1;
    
    vkCreateFramebuffer(ctx->device, &fbInfo, NULL, &ctx->offscreenFB);
    
    printf("Offscreen FBO created: %dx%d\n", OFFSCREEN_SIZE, OFFSCREEN_SIZE);
}

static void createSampler(VkCtx *ctx) {
    VkSamplerCreateInfo samplerInfo = {0};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    
    vkCreateSampler(ctx->device, &samplerInfo, NULL, &ctx->sampler);
}

static void createDescriptors(VkCtx *ctx) {
    // Descriptor set layout
    VkDescriptorSetLayoutBinding binding = {0};
    binding.binding = 0;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    binding.descriptorCount = 1;
    binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    
    VkDescriptorSetLayoutCreateInfo layoutInfo = {0};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &binding;
    
    vkCreateDescriptorSetLayout(ctx->device, &layoutInfo, NULL, &ctx->descLayout);
    
    // Descriptor pool
    VkDescriptorPoolSize poolSize = {0};
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = 1;
    
    VkDescriptorPoolCreateInfo poolInfo = {0};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = 1;
    
    vkCreateDescriptorPool(ctx->device, &poolInfo, NULL, &ctx->descPool);
    
    // Allocate descriptor set
    VkDescriptorSetAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = ctx->descPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &ctx->descLayout;
    
    vkAllocateDescriptorSets(ctx->device, &allocInfo, &ctx->descSet);
    
    // Update descriptor set
    VkDescriptorImageInfo imageInfo = {0};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = ctx->offscreenView;
    imageInfo.sampler = ctx->sampler;
    
    VkWriteDescriptorSet write = {0};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = ctx->descSet;
    write.dstBinding = 0;
    write.dstArrayElement = 0;
    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.descriptorCount = 1;
    write.pImageInfo = &imageInfo;
    
    vkUpdateDescriptorSets(ctx->device, 1, &write, 0, NULL);
}

static void createPipeline(VkCtx *ctx) {
    // Load shaders from files
    FILE *f = fopen("vert.spv", "rb");
    if (!f) {
        printf("Failed to open vert.spv\n");
        return;
    }
    fseek(f, 0, SEEK_END);
    long vertSize = ftell(f);
    fseek(f, 0, SEEK_SET);
    uint32_t *vertCode = malloc(vertSize);
    fread(vertCode, 1, vertSize, f);
    fclose(f);
    
    f = fopen("frag.spv", "rb");
    if (!f) {
        printf("Failed to open frag.spv\n");
        free(vertCode);
        return;
    }
    fseek(f, 0, SEEK_END);
    long fragSize = ftell(f);
    fseek(f, 0, SEEK_SET);
    uint32_t *fragCode = malloc(fragSize);
    fread(fragCode, 1, fragSize, f);
    fclose(f);
    
    VkShaderModuleCreateInfo vertInfo = {0};
    vertInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vertInfo.codeSize = vertSize;
    vertInfo.pCode = vertCode;
    
    VkShaderModule vertShader;
    VkResult res = vkCreateShaderModule(ctx->device, &vertInfo, NULL, &vertShader);
    if (res != VK_SUCCESS) {
        printf("Failed to create vertex shader: %d\n", res);
        free(vertCode);
        free(fragCode);
        return;
    }
    
    VkShaderModuleCreateInfo fragInfo = {0};
    fragInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    fragInfo.codeSize = fragSize;
    fragInfo.pCode = fragCode;
    
    VkShaderModule fragShader;
    res = vkCreateShaderModule(ctx->device, &fragInfo, NULL, &fragShader);
    if (res != VK_SUCCESS) {
        printf("Failed to create fragment shader: %d\n", res);
        vkDestroyShaderModule(ctx->device, vertShader, NULL);
        free(vertCode);
        free(fragCode);
        return;
    }
    
    VkPipelineShaderStageCreateInfo stages[2] = {0};
    stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    stages[0].module = vertShader;
    stages[0].pName = "main";
    
    stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    stages[1].module = fragShader;
    stages[1].pName = "main";
    
    VkVertexInputBindingDescription bindingDesc = {0};
    VkVertexInputAttributeDescription attrDesc = {0};
    
    VkPipelineVertexInputStateCreateInfo vertexInput = {0};
    vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInput.vertexBindingDescriptionCount = 0;
    vertexInput.vertexAttributeDescriptionCount = 0;
    
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {0};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    
    VkViewport viewport = {0, 0, WIDTH, HEIGHT, 0.0f, 1.0f};
    VkRect2D scissor = {{0, 0}, {WIDTH, HEIGHT}};
    
    VkPipelineViewportStateCreateInfo viewportState = {0};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;
    
    VkPipelineRasterizationStateCreateInfo rasterizer = {0};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    
    VkPipelineMultisampleStateCreateInfo multisampling = {0};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {0};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    
    VkPipelineColorBlendStateCreateInfo colorBlending = {0};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {0};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &ctx->descLayout;
    
    vkCreatePipelineLayout(ctx->device, &pipelineLayoutInfo, NULL, &ctx->pipelineLayout);
    
    VkGraphicsPipelineCreateInfo pipelineInfo = {0};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = stages;
    pipelineInfo.pVertexInputState = &vertexInput;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = ctx->pipelineLayout;
    pipelineInfo.renderPass = ctx->renderPass;
    pipelineInfo.subpass = 0;
    
    vkCreateGraphicsPipelines(ctx->device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &ctx->pipeline);
    
    if (ctx->pipeline == VK_NULL_HANDLE) {
        printf("Failed to create pipeline\n");
    }
    
    vkDestroyShaderModule(ctx->device, vertShader, NULL);
    vkDestroyShaderModule(ctx->device, fragShader, NULL);
    free(vertCode);
    free(fragCode);
}

static void createCommandBuffers(VkCtx *ctx) {
    VkCommandPoolCreateInfo poolInfo = {0};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = ctx->queueFamily;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    
    vkCreateCommandPool(ctx->device, &poolInfo, NULL, &ctx->cmdPool);
    
    VkCommandBufferAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = ctx->cmdPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;
    
    vkAllocateCommandBuffers(ctx->device, &allocInfo, &ctx->cmdBuf);
}

static void createSyncObjects(VkCtx *ctx) {
    VkSemaphoreCreateInfo semInfo = {0};
    semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    
    vkCreateSemaphore(ctx->device, &semInfo, NULL, &ctx->imageAvail);
    vkCreateSemaphore(ctx->device, &semInfo, NULL, &ctx->renderDone);
    
    VkFenceCreateInfo fenceInfo = {0};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    
    vkCreateFence(ctx->device, &fenceInfo, NULL, &ctx->fence);
}

static void renderOffscreen(VkCtx *ctx) {
    VkCommandBufferBeginInfo beginInfo = {0};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    
    vkBeginCommandBuffer(ctx->cmdBuf, &beginInfo);
    
    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};  // Opaque black
    
    VkRenderPassBeginInfo rpBegin = {0};
    rpBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpBegin.renderPass = ctx->offscreenRP;
    rpBegin.framebuffer = ctx->offscreenFB;
    rpBegin.renderArea.extent.width = OFFSCREEN_SIZE;
    rpBegin.renderArea.extent.height = OFFSCREEN_SIZE;
    rpBegin.clearValueCount = 1;
    rpBegin.pClearValues = &clearColor;
    
    vkCmdBeginRenderPass(ctx->cmdBuf, &rpBegin, VK_SUBPASS_CONTENTS_INLINE);
    
    // Set viewport and scissor for offscreen rendering
    VkViewport viewport = {0};
    viewport.width = (float)OFFSCREEN_SIZE;
    viewport.height = (float)OFFSCREEN_SIZE;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(ctx->cmdBuf, 0, 1, &viewport);
    
    VkRect2D scissor = {0};
    scissor.extent.width = OFFSCREEN_SIZE;
    scissor.extent.height = OFFSCREEN_SIZE;
    vkCmdSetScissor(ctx->cmdBuf, 0, 1, &scissor);
    
    // Use NanoVG to draw instead of Vulkan clear
    nvgBeginFrame(ctx->vg, OFFSCREEN_SIZE, OFFSCREEN_SIZE, 1.0f);
    
    nvgBeginPath(ctx->vg);
    nvgRect(ctx->vg, 0, 0, OFFSCREEN_SIZE, OFFSCREEN_SIZE);
    nvgFillColor(ctx->vg, nvgRGBA(0, 0, 255, 255));
    nvgFill(ctx->vg);
    
    nvgEndFrame(ctx->vg);
    
    vkCmdEndRenderPass(ctx->cmdBuf);
    
    vkEndCommandBuffer(ctx->cmdBuf);
    
    VkSubmitInfo submitInfo = {0};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &ctx->cmdBuf;
    
    vkQueueSubmit(ctx->queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(ctx->queue);
    
    printf("Rendered to offscreen with NanoVG (blue)\n");
}

static void drawFrame(VkCtx *ctx) {
    float t;
    
    vkWaitForFences(ctx->device, 1, &ctx->fence, VK_TRUE, UINT64_MAX);
    vkResetFences(ctx->device, 1, &ctx->fence);
    
    uint32_t imageIndex;
    vkAcquireNextImageKHR(ctx->device, ctx->swapchain, UINT64_MAX, ctx->imageAvail, VK_NULL_HANDLE, &imageIndex);
    
    vkResetCommandBuffer(ctx->cmdBuf, 0);
    
    VkCommandBufferBeginInfo beginInfo = {0};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    
    vkBeginCommandBuffer(ctx->cmdBuf, &beginInfo);
    
    // Render to offscreen first with animated color
    t = (float)glfwGetTime();
    float colorMix = (sinf(t * 2.0f) + 1.0f) * 0.5f;  // 0 to 1
    int r = (int)(colorMix * 255);
    int b = (int)((1.0f - colorMix) * 255);
    
    VkClearValue offscreenClear = {{{0.0f, 0.0f, 0.0f, 0.0f}}};
    
    VkRenderPassBeginInfo offscreenRpBegin = {0};
    offscreenRpBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    offscreenRpBegin.renderPass = ctx->offscreenRP;
    offscreenRpBegin.framebuffer = ctx->offscreenFB;
    offscreenRpBegin.renderArea.extent.width = OFFSCREEN_SIZE;
    offscreenRpBegin.renderArea.extent.height = OFFSCREEN_SIZE;
    offscreenRpBegin.clearValueCount = 1;
    offscreenRpBegin.pClearValues = &offscreenClear;
    
    vkCmdBeginRenderPass(ctx->cmdBuf, &offscreenRpBegin, VK_SUBPASS_CONTENTS_INLINE);
    
    VkViewport offscreenViewport = {0};
    offscreenViewport.width = OFFSCREEN_SIZE;
    offscreenViewport.height = OFFSCREEN_SIZE;
    offscreenViewport.maxDepth = 1.0f;
    vkCmdSetViewport(ctx->cmdBuf, 0, 1, &offscreenViewport);
    
    VkRect2D offscreenScissor = {0};
    offscreenScissor.extent.width = OFFSCREEN_SIZE;
    offscreenScissor.extent.height = OFFSCREEN_SIZE;
    vkCmdSetScissor(ctx->cmdBuf, 0, 1, &offscreenScissor);
    
    nvgBeginFrame(ctx->vg, OFFSCREEN_SIZE, OFFSCREEN_SIZE, 1.0f);
    nvgBeginPath(ctx->vg);
    nvgRect(ctx->vg, 0, 0, OFFSCREEN_SIZE, OFFSCREEN_SIZE);
    nvgFillColor(ctx->vg, nvgRGBA(r, 0, b, 255));
    nvgFill(ctx->vg);
    nvgEndFrame(ctx->vg);
    
    vkCmdEndRenderPass(ctx->cmdBuf);
    
    // Now render main screen
    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 0.0f}}};  // Transparent
    
    VkRenderPassBeginInfo rpBegin = {0};
    rpBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpBegin.renderPass = ctx->renderPass;
    rpBegin.framebuffer = ctx->framebuffers[imageIndex];
    rpBegin.renderArea.extent.width = WIDTH;
    rpBegin.renderArea.extent.height = HEIGHT;
    rpBegin.clearValueCount = 1;
    rpBegin.pClearValues = &clearColor;
    
    vkCmdBeginRenderPass(ctx->cmdBuf, &rpBegin, VK_SUBPASS_CONTENTS_INLINE);
    
    // Set viewport and scissor for main screen
    VkViewport viewport = {0};
    viewport.width = (float)WIDTH;
    viewport.height = (float)HEIGHT;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(ctx->cmdBuf, 0, 1, &viewport);
    
    VkRect2D scissor = {0};
    scissor.extent.width = WIDTH;
    scissor.extent.height = HEIGHT;
    vkCmdSetScissor(ctx->cmdBuf, 0, 1, &scissor);
    
    // Use NanoVG to paint screen yellow
    nvgBeginFrame(ctx->vgScreen, WIDTH, HEIGHT, 1.0f);
    
    // Yellow background
    nvgBeginPath(ctx->vgScreen);
    nvgRect(ctx->vgScreen, 0, 0, WIDTH, HEIGHT);
    nvgFillColor(ctx->vgScreen, nvgRGBA(255, 255, 0, 255));
    nvgFill(ctx->vgScreen);
    
    // Draw offscreen texture with movement
    t = (float)glfwGetTime();
    float x = 100 + sinf(t * 1.3f) * 50;
    float y = 100 + cosf(t * 1.7f) * 50;
    
    nvgBeginPath(ctx->vgScreen);
    nvgRect(ctx->vgScreen, x, y, OFFSCREEN_SIZE, OFFSCREEN_SIZE);
    NVGpaint imgPaint = nvgImagePattern(ctx->vgScreen, x, y, OFFSCREEN_SIZE, OFFSCREEN_SIZE, 0, ctx->offscreenImgId, 1.0f);
    nvgFillPaint(ctx->vgScreen, imgPaint);
    nvgFill(ctx->vgScreen);
    
    nvgEndFrame(ctx->vgScreen);
    
    vkCmdEndRenderPass(ctx->cmdBuf);
    
    vkEndCommandBuffer(ctx->cmdBuf);
    
    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submitInfo = {0};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &ctx->imageAvail;
    submitInfo.pWaitDstStageMask = &waitStage;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &ctx->cmdBuf;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &ctx->renderDone;
    
    vkQueueSubmit(ctx->queue, 1, &submitInfo, ctx->fence);
    
    VkPresentInfoKHR presentInfo = {0};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &ctx->renderDone;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &ctx->swapchain;
    presentInfo.pImageIndices = &imageIndex;
    
    vkQueuePresentKHR(ctx->queue, &presentInfo);
}

int main(void) {
    printf("Minimal Vulkan Offscreen FBO Demo\n");
    fflush(stdout);
    
    if (!glfwInit()) {
        printf("GLFW init failed\n");
        return -1;
    }
    printf("GLFW initialized\n");
    fflush(stdout);
    
    if (!glfwVulkanSupported()) {
        printf("Vulkan not supported\n");
        return -1;
    }
    printf("Vulkan supported\n");
    fflush(stdout);
    
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan FBO Demo", NULL, NULL);
    printf("Window created\n");
    fflush(stdout);
    
    VkCtx ctx = {0};
    
    initVulkan(&ctx, window);
    printf("Vulkan initialized\n");
    fflush(stdout);
    createSwapchain(&ctx);
    if (ctx.swapchain == VK_NULL_HANDLE) {
        printf("Swapchain creation failed, exiting\n");
        return -1;
    }
    printf("Swapchain created\n");
    fflush(stdout);
    createRenderPass(&ctx);
    printf("Render pass created\n");
    fflush(stdout);
    createOffscreen(&ctx);
    printf("Offscreen created\n");
    fflush(stdout);
    createSampler(&ctx);
    createDescriptors(&ctx);
    printf("Descriptors created\n");
    fflush(stdout);
    createPipeline(&ctx);
    printf("Pipeline created\n");
    fflush(stdout);
    createCommandBuffers(&ctx);
    printf("Command buffers created\n");
    fflush(stdout);
    createSyncObjects(&ctx);
    printf("Sync objects created\n");
    fflush(stdout);
    
    // Create NanoVG context - use offscreen render pass since we only render offscreen
    uint32_t currentFrame = 0;
    VKNVGCreateInfo nvgInfo = {0};
    nvgInfo.device = ctx.device;
    nvgInfo.gpu = ctx.gpu;
    nvgInfo.renderpass = ctx.offscreenRP;  // Offscreen render pass
    nvgInfo.cmdBuffer = &ctx.cmdBuf;
    nvgInfo.swapchainImageCount = 1;
    nvgInfo.currentFrame = &currentFrame;
    
    ctx.vg = nvgCreateVk(nvgInfo, NVG_ANTIALIAS | NVG_STENCIL_STROKES, ctx.queue);
    if (!ctx.vg) {
        printf("Failed to create NanoVG context\n");
        return -1;
    }
    printf("NanoVG offscreen context created\n");
    fflush(stdout);
    
    // Create second NanoVG context for main screen
    VKNVGCreateInfo nvgScreenInfo = {0};
    nvgScreenInfo.device = ctx.device;
    nvgScreenInfo.gpu = ctx.gpu;
    nvgScreenInfo.renderpass = ctx.renderPass;  // Main render pass
    nvgScreenInfo.cmdBuffer = &ctx.cmdBuf;
    nvgScreenInfo.swapchainImageCount = 1;
    nvgScreenInfo.currentFrame = &currentFrame;
    
    ctx.vgScreen = nvgCreateVk(nvgScreenInfo, NVG_ANTIALIAS | NVG_STENCIL_STROKES, ctx.queue);
    if (!ctx.vgScreen) {
        printf("Failed to create NanoVG screen context\n");
        return -1;
    }
    printf("NanoVG screen context created\n");
    fflush(stdout);
    
    // Create NanoVG image from offscreen texture
    ctx.offscreenImgId = vknvgCreateImageFromHandle(ctx.vgScreen, ctx.offscreenView, OFFSCREEN_SIZE, OFFSCREEN_SIZE, 0);
    printf("Created NanoVG image from offscreen texture, ID: %d\n", ctx.offscreenImgId);
    fflush(stdout);
    
    // Render once to offscreen
    renderOffscreen(&ctx);
    printf("Offscreen rendered\n");
    fflush(stdout);
    
    printf("Starting main loop (red screen - offscreen FBO created and rendered blue)\n");
    printf("Note: Textured quad display needs proper SPIR-V shaders\n");
    fflush(stdout);
    
    while (!glfwWindowShouldClose(window)) {
        drawFrame(&ctx);
        glfwPollEvents();
    }
    
    vkDeviceWaitIdle(ctx.device);
    
    // Cleanup
    nvgDeleteVk(ctx.vgScreen);
    nvgDeleteVk(ctx.vg);
    vkDestroyPipeline(ctx.device, ctx.pipeline, NULL);
    vkDestroyPipelineLayout(ctx.device, ctx.pipelineLayout, NULL);
    vkDestroyDescriptorPool(ctx.device, ctx.descPool, NULL);
    vkDestroyDescriptorSetLayout(ctx.device, ctx.descLayout, NULL);
    vkDestroySampler(ctx.device, ctx.sampler, NULL);
    vkDestroyFence(ctx.device, ctx.fence, NULL);
    vkDestroySemaphore(ctx.device, ctx.renderDone, NULL);
    vkDestroySemaphore(ctx.device, ctx.imageAvail, NULL);
    vkDestroyCommandPool(ctx.device, ctx.cmdPool, NULL);
    vkDestroyFramebuffer(ctx.device, ctx.offscreenFB, NULL);
    vkDestroyRenderPass(ctx.device, ctx.offscreenRP, NULL);
    vkDestroyImageView(ctx.device, ctx.offscreenView, NULL);
    vkFreeMemory(ctx.device, ctx.offscreenMem, NULL);
    vkDestroyImage(ctx.device, ctx.offscreenImg, NULL);
    for (uint32_t i = 0; i < ctx.imageCount; i++) {
        vkDestroyFramebuffer(ctx.device, ctx.framebuffers[i], NULL);
        vkDestroyImageView(ctx.device, ctx.swapViews[i], NULL);
    }
    free(ctx.framebuffers);
    free(ctx.swapViews);
    free(ctx.swapImages);
    vkDestroyRenderPass(ctx.device, ctx.renderPass, NULL);
    vkDestroySwapchainKHR(ctx.device, ctx.swapchain, NULL);
    vkDestroyDevice(ctx.device, NULL);
    vkDestroySurfaceKHR(ctx.instance, ctx.surface, NULL);
    vkDestroyInstance(ctx.instance, NULL);
    
    glfwDestroyWindow(window);
    glfwTerminate();
    
    printf("Done\n");
    return 0;
}

