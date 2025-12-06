// Vulkan ReadPixels test - render to FBO, read pixels, save to PNG
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define NANOVG_VULKAN_IMPLEMENTATION
#include <nanovg.h>
#include <nanovg_vk.h>
#include <nanovg_vk_utils.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "vulkan_util.h"

int main(void)
{
   GLFWwindow        *window;
   NVGcontext        *vg;
   NVGframebuffer    fb;
   VKNVGCreateInfo   createInfo;
   VkCommandBuffer   *cmdBuffers;
   VkCommandPool     cmdPool;
   VkInstance        instance;
   VkNvgExt          extQuery;
   VkQueue           queue;
   VkResult          res;
   VkSurfaceKHR      surface;
   VulkanDevice      *device;
   int               fbSize;
   unsigned char     *pixels;
   uint32_t          gpuCount, gpuIdx, j;
   VkPhysicalDevice  gpu[32];
   uint32_t          currentFrame = 0;

   fbSize = 512;

   if( !glfwInit())
   {
      printf( "Failed to init GLFW.\n");
      return( -1);
   }

   if( !glfwVulkanSupported())
   {
      printf( "Vulkan not supported.\n");
      glfwTerminate();
      return( -1);
   }

   glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API);
   glfwWindowHint( GLFW_VISIBLE, GLFW_FALSE);
   window = glfwCreateWindow( 100, 100, "ReadPixels Test", NULL, NULL);
   if( !window)
   {
      glfwTerminate();
      return( -1);
   }

   instance = createVkInstance( false);
   if( instance == VK_NULL_HANDLE)
   {
      printf( "Failed to create Vulkan instance\n");
      glfwTerminate();
      return( -1);
   }

   res = glfwCreateWindowSurface( instance, window, NULL, &surface);
   if( res != VK_SUCCESS)
   {
      printf( "Failed to create surface\n");
      glfwTerminate();
      return( -1);
   }

   gpuCount = 0;
   res = vkEnumeratePhysicalDevices( instance, &gpuCount, NULL);
   if( res != VK_SUCCESS || gpuCount < 1)
   {
      printf( "No Vulkan device found.\n");
      glfwTerminate();
      return( -1);
   }

   vkEnumeratePhysicalDevices( instance, &gpuCount, gpu);
   gpuIdx = 0;

   memset( &extQuery, 0, sizeof( extQuery));
   device = createVulkanDevice( gpu[gpuIdx], surface, &extQuery);
   if( device == NULL)
   {
      printf( "Failed to create Vulkan device\n");
      vkDestroySurfaceKHR( instance, surface, NULL);
      vkDestroyInstance( instance, NULL);
      glfwTerminate();
      return( -1);
   }

   vkGetDeviceQueue( device->device, device->graphicsQueueFamilyIndex, 0, &queue);
   cmdPool = createCmdPool( device);
   cmdBuffers = createCmdBuffer( device->device, cmdPool, 1);

   VkRenderPass renderPass = createRenderPass( device->device, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_D24_UNORM_S8_UINT);

   memset( &createInfo, 0, sizeof( createInfo));
   createInfo.gpu                  = gpu[gpuIdx];
   createInfo.device               = device->device;
   createInfo.renderpass           = renderPass;
   createInfo.cmdBuffer            = cmdBuffers;
   createInfo.swapchainImageCount  = 1;
   createInfo.currentFrame         = &currentFrame;
   createInfo.queueFamilyIndex     = device->graphicsQueueFamilyIndex;
   createInfo.allocator            = NULL;
   createInfo.ext                  = extQuery;

   vg = nvgCreateVk( createInfo, NVG_ANTIALIAS | NVG_STENCIL_STROKES, queue);
   if( !vg)
   {
      printf( "Could not init nanovg.\n");
      return( -1);
   }

   printf( "Creating %dx%d framebuffer...\n", fbSize, fbSize);
   fb = vknvgCreateFramebuffer( vg, fbSize, fbSize, 0);
   if( !fb)
   {
      printf( "Could not create FBO.\n");
      return( -1);
   }

   printf( "Rendering to framebuffer...\n");
   vknvgBindFramebuffer( vg, fb);
   
   // Begin command buffer
   VkCommandBufferBeginInfo beginInfo;
   memset(&beginInfo, 0, sizeof(beginInfo));
   beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
   beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
   vkBeginCommandBuffer(cmdBuffers[0], &beginInfo);
   
   // Begin render pass
   VKNVGframebuffer* vkfb = (VKNVGframebuffer*)fb;
   VkClearValue clearValues[2];
   clearValues[0].color.float32[0] = 0.2f;
   clearValues[0].color.float32[1] = 0.2f;
   clearValues[0].color.float32[2] = 0.3f;
   clearValues[0].color.float32[3] = 1.0f;
   clearValues[1].depthStencil.depth = 1.0f;
   clearValues[1].depthStencil.stencil = 0;
   
   VkRenderPassBeginInfo rpBegin;
   memset(&rpBegin, 0, sizeof(rpBegin));
   rpBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
   rpBegin.renderPass = vkfb->renderPass;
   rpBegin.framebuffer = vkfb->framebuffer;
   rpBegin.renderArea.extent.width = fbSize;
   rpBegin.renderArea.extent.height = fbSize;
   rpBegin.clearValueCount = 2;
   rpBegin.pClearValues = clearValues;
   vkCmdBeginRenderPass(cmdBuffers[0], &rpBegin, VK_SUBPASS_CONTENTS_INLINE);
   
   // Set viewport and scissor
   VkViewport viewport;
   viewport.x = 0.0f;
   viewport.y = 0.0f;
   viewport.width = (float)fbSize;
   viewport.height = (float)fbSize;
   viewport.minDepth = 0.0f;
   viewport.maxDepth = 1.0f;
   vkCmdSetViewport(cmdBuffers[0], 0, 1, &viewport);
   
   VkRect2D scissor;
   scissor.offset.x = 0;
   scissor.offset.y = 0;
   scissor.extent.width = fbSize;
   scissor.extent.height = fbSize;
   vkCmdSetScissor(cmdBuffers[0], 0, 1, &scissor);
   
   nvgBeginFrame( vg, fbSize, fbSize, 1.0f);

   nvgBeginPath( vg);
   nvgCircle( vg, fbSize/2, fbSize/2, 100);
   nvgFillColor( vg, nvgRGBA( 255, 192, 0, 255));
   nvgFill( vg);

   nvgBeginPath( vg);
   nvgRect( vg, 50, 50, 150, 150);
   nvgStrokeColor( vg, nvgRGBA( 255, 255, 255, 255));
   nvgStrokeWidth( vg, 5.0f);
   nvgStroke( vg);

   nvgEndFrame( vg);
   
   // End render pass and command buffer
   vkCmdEndRenderPass(cmdBuffers[0]);
   vkEndCommandBuffer(cmdBuffers[0]);
   
   // Submit command buffer
   VkSubmitInfo submitInfo;
   memset(&submitInfo, 0, sizeof(submitInfo));
   submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
   submitInfo.commandBufferCount = 1;
   submitInfo.pCommandBuffers = &cmdBuffers[0];
   vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
   vkQueueWaitIdle(queue);

   printf( "Reading pixels...\n");
   pixels = (unsigned char*)malloc( fbSize * fbSize * 4);
   vknvgReadPixels( vg, fb, 0, 0, fbSize, fbSize, pixels);

   printf( "Saving to vulkan-output.png...\n");
   stbi_write_png( "vulkan-output.png", fbSize, fbSize, 4, pixels, fbSize * 4);

   free( pixels);
   vknvgDeleteFramebuffer( vg, fb);
   nvgDeleteVk( vg);
   destroyVulkanDevice( device);
   vkDestroySurfaceKHR( instance, surface, NULL);
   vkDestroyInstance( instance, NULL);
   glfwTerminate();
   
   printf( "Done! Check vulkan-output.png\n");
   return( 0);
}
