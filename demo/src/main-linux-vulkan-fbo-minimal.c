// Minimal FBO test - blue offscreen texture on red background
#include <stdio.h>
#include <math.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define NANOVG_VULKAN_IMPLEMENTATION
#include <nanovg_vk.h>
#include <nanovg_vk_utils.h>
#include "vulkan_util.h"

static void prepareFrame(VkDevice device, VkCommandBuffer cmdBuffer, FrameBuffers *fb)
{
   VkClearValue clearValues[2];
   VkCommandBufferBeginInfo cmdBufInfo;
   VkRenderPassBeginInfo rpBegin;
   VkResult res;
   
   res = vkAcquireNextImageKHR(device, fb->swap_chain, UINT64_MAX, 
                               fb->present_complete_semaphore[fb->current_frame],
                               VK_NULL_HANDLE, &fb->current_buffer);
   if(res == VK_ERROR_OUT_OF_DATE_KHR)
      return;
   
   memset(&cmdBufInfo, 0, sizeof(cmdBufInfo));
   cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
   vkBeginCommandBuffer(cmdBuffer, &cmdBufInfo);
   
   clearValues[0].color.float32[0] = 1.0f;  // Red
   clearValues[0].color.float32[1] = 0.0f;
   clearValues[0].color.float32[2] = 0.0f;
   clearValues[0].color.float32[3] = 1.0f;
   clearValues[1].depthStencil.depth = 1.0f;
   clearValues[1].depthStencil.stencil = 0;
   
   memset(&rpBegin, 0, sizeof(rpBegin));
   rpBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
   rpBegin.renderPass = fb->render_pass;
   rpBegin.framebuffer = fb->framebuffers[fb->current_buffer];
   rpBegin.renderArea.offset.x = 0;
   rpBegin.renderArea.offset.y = 0;
   rpBegin.renderArea.extent.width = fb->buffer_size.width;
   rpBegin.renderArea.extent.height = fb->buffer_size.height;
   rpBegin.clearValueCount = 2;
   rpBegin.pClearValues = clearValues;
   
   vkCmdBeginRenderPass(cmdBuffer, &rpBegin, VK_SUBPASS_CONTENTS_INLINE);
   
   VkViewport viewport;
   viewport.width = (float)fb->buffer_size.width;
   viewport.height = (float)fb->buffer_size.height;
   viewport.minDepth = 0.0f;
   viewport.maxDepth = 1.0f;
   viewport.x = 0.0f;
   viewport.y = 0.0f;
   vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);
   
   VkRect2D scissor = rpBegin.renderArea;
   vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);
}

static void submitFrame(VkDevice device, VkQueue queue, VkCommandBuffer cmdBuffer, FrameBuffers *fb)
{
   VkPipelineStageFlags waitStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
   VkPresentInfoKHR presentInfo;
   VkSubmitInfo submitInfo;
   
   vkCmdEndRenderPass(cmdBuffer);
   vkEndCommandBuffer(cmdBuffer);
   
   memset(&submitInfo, 0, sizeof(submitInfo));
   submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
   submitInfo.waitSemaphoreCount = 1;
   submitInfo.pWaitSemaphores = &fb->present_complete_semaphore[fb->current_frame];
   submitInfo.pWaitDstStageMask = &waitStages;
   submitInfo.commandBufferCount = 1;
   submitInfo.pCommandBuffers = &cmdBuffer;
   submitInfo.signalSemaphoreCount = 1;
   submitInfo.pSignalSemaphores = &fb->render_complete_semaphore[fb->current_frame];
   
   vkQueueSubmit(queue, 1, &submitInfo, fb->flight_fence[fb->current_frame]);
   
   memset(&presentInfo, 0, sizeof(presentInfo));
   presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
   presentInfo.waitSemaphoreCount = 1;
   presentInfo.pWaitSemaphores = &fb->render_complete_semaphore[fb->current_frame];
   presentInfo.swapchainCount = 1;
   presentInfo.pSwapchains = &fb->swap_chain;
   presentInfo.pImageIndices = &fb->current_buffer;
   
   vkQueuePresentKHR(queue, &presentInfo);
   
   fb->current_frame = (fb->current_frame + 1) % fb->swapchain_image_count;
   fb->num_swaps++;
   
   if(fb->num_swaps >= fb->swapchain_image_count)
   {
      vkWaitForFences(device, 1, &fb->flight_fence[fb->current_frame], VK_TRUE, UINT64_MAX);
      vkResetFences(device, 1, &fb->flight_fence[fb->current_frame]);
   }
}

int main(void)
{
   FrameBuffers fb;
   GLFWwindow *window;
   NVGcontext *vg, *vgFbo;
   NVGframebuffer fbo;
   VKNVGCreateInfo createInfo;
   VkCommandBuffer *cmdBuffers, fboCmd;
   VkInstance instance;
   VkNvgExt extQuery;
   VkQueue queue;
   VkResult res;
   VkSurfaceKHR surface;
   VulkanDevice *device;
   double t;
   float pxRatio;
   int fbHeight, fbWidth, winHeight, winWidth;
   uint32_t gpuCount, gpuIdx, j;
   VkPhysicalDevice gpu[32];

   printf("Minimal FBO test...\n");
   
   if(!glfwInit())
   {
      printf("Failed to init GLFW.\n");
      return -1;
   }

   if(!glfwVulkanSupported())
   {
      printf("Vulkan not supported.\n");
      glfwTerminate();
      return -1;
   }

   glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
   window = glfwCreateWindow(800, 600, "Minimal FBO Test", NULL, NULL);
   if(!window)
   {
      glfwTerminate();
      return -1;
   }

   instance = createVkInstance(false);
   if(instance == VK_NULL_HANDLE)
   {
      printf("Failed to create Vulkan instance\n");
      glfwTerminate();
      return -1;
   }

   res = glfwCreateWindowSurface(instance, window, NULL, &surface);
   if(res != VK_SUCCESS)
   {
      printf("Failed to create surface\n");
      glfwTerminate();
      return -1;
   }

   gpuCount = 0;
   res = vkEnumeratePhysicalDevices(instance, &gpuCount, NULL);
   if(res != VK_SUCCESS || gpuCount < 1)
   {
      printf("No Vulkan device found.\n");
      glfwTerminate();
      return -1;
   }

   vkEnumeratePhysicalDevices(instance, &gpuCount, gpu);

   gpuIdx = 0;
   for(j = 0; j < gpuCount; j++)
   {
      uint32_t qfc = 0;
      vkGetPhysicalDeviceQueueFamilyProperties(gpu[j], &qfc, NULL);
      if(qfc >= 1)
      {
         gpuIdx = j;
         break;
      }
   }

   memset(&extQuery, 0, sizeof(extQuery));
   device = createVulkanDevice(gpu[gpuIdx], surface, &extQuery);
   if(device == NULL)
   {
      printf("Failed to create Vulkan device\n");
      vkDestroySurfaceKHR(instance, surface, NULL);
      vkDestroyInstance(instance, NULL);
      glfwTerminate();
      return -1;
   }

   glfwGetWindowSize(window, &winWidth, &winHeight);
   glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
   pxRatio = (float)fbWidth / (float)winWidth;

   vkGetDeviceQueue(device->device, device->graphicsQueueFamilyIndex, 0, &queue);

   fb = createFrameBuffers(device, surface, queue, winWidth, winHeight, 0);
   cmdBuffers = createCmdBuffer(device->device, device->commandPool, fb.swapchain_image_count);

   memset(&createInfo, 0, sizeof(createInfo));
   createInfo.device = device->device;
   createInfo.gpu = device->gpu;
   createInfo.renderpass = fb.render_pass;
   createInfo.cmdBuffer = cmdBuffers;
   createInfo.swapchainImageCount = fb.swapchain_image_count;
   createInfo.currentFrame = &fb.current_frame;
   createInfo.ext.dynamicState = extQuery.dynamicState;
   createInfo.ext.colorBlendEquation = extQuery.colorBlendEquation;
   createInfo.ext.colorWriteMask = extQuery.colorWriteMask;

   vg = nvgCreateVk(createInfo, NVG_ANTIALIAS | NVG_STENCIL_STROKES, queue);
   if(!vg)
   {
      printf("Could not init nanovg.\n");
      return -1;
   }

   // Create offscreen framebuffer
   fbo = vknvgCreateFramebuffer(vg, 200, 200, 0);
   if(!fbo)
   {
      printf("Could not create FBO.\n");
      return -1;
   }
   printf("FBO created, image ID: %d\n", (int)(intptr_t)fbo);

   // Allocate separate command buffer for FBO rendering
   VkCommandBufferAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
   allocInfo.commandPool = device->commandPool;
   allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
   allocInfo.commandBufferCount = 1;
   vkAllocateCommandBuffers(device->device, &allocInfo, &fboCmd);

   // Create separate NanoVG context for FBO rendering
   static uint32_t fboCurrentFrame = 0;
   VKNVGCreateInfo fboCreateInfo;
   VKNVGframebuffer *vkfb = (VKNVGframebuffer*)fbo;
   
   memset(&fboCreateInfo, 0, sizeof(fboCreateInfo));
   fboCreateInfo.device = device->device;
   fboCreateInfo.gpu = device->gpu;
   fboCreateInfo.renderpass = vkfb->renderPass;
   fboCreateInfo.cmdBuffer = &fboCmd;
   fboCreateInfo.swapchainImageCount = 1;
   fboCreateInfo.currentFrame = &fboCurrentFrame;
   fboCreateInfo.ext.dynamicState = extQuery.dynamicState;
   fboCreateInfo.ext.colorBlendEquation = extQuery.colorBlendEquation;
   fboCreateInfo.ext.colorWriteMask = extQuery.colorWriteMask;
   
   vgFbo = nvgCreateVk(fboCreateInfo, NVG_ANTIALIAS | NVG_STENCIL_STROKES, queue);
   if(!vgFbo)
   {
      printf("Could not create FBO NanoVG context.\n");
      return -1;
   }

   printf("Starting render loop...\n");
   glfwSetTime(0);

   while(!glfwWindowShouldClose(window))
   {
      t = glfwGetTime();

      glfwGetWindowSize(window, &winWidth, &winHeight);
      glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
      pxRatio = (float)fbWidth / (float)winWidth;

      // Render to offscreen FBO - fill with blue
      VKNVGframebuffer *vkfb = (VKNVGframebuffer*)fbo;
      
      VkCommandBufferBeginInfo fboBegin = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
      fboBegin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
      vkBeginCommandBuffer(fboCmd, &fboBegin);
      
      VkClearValue clearValues[2];
      clearValues[0].color.float32[0] = 0.0f;
      clearValues[0].color.float32[1] = 0.0f;
      clearValues[0].color.float32[2] = 0.0f;
      clearValues[0].color.float32[3] = 0.0f;
      clearValues[1].depthStencil.depth = 1.0f;
      clearValues[1].depthStencil.stencil = 0;
      
      VkRenderPassBeginInfo rpBegin = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
      rpBegin.renderPass = vkfb->renderPass;
      rpBegin.framebuffer = vkfb->framebuffer;
      rpBegin.renderArea.extent.width = 200;
      rpBegin.renderArea.extent.height = 200;
      rpBegin.clearValueCount = 2;
      rpBegin.pClearValues = clearValues;
      
      vkCmdBeginRenderPass(fboCmd, &rpBegin, VK_SUBPASS_CONTENTS_INLINE);
      
      VkViewport viewport = {0};
      viewport.width = 200.0f;
      viewport.height = 200.0f;
      viewport.maxDepth = 1.0f;
      vkCmdSetViewport(fboCmd, 0, 1, &viewport);
      
      VkRect2D scissor = {0};
      scissor.extent.width = 200;
      scissor.extent.height = 200;
      vkCmdSetScissor(fboCmd, 0, 1, &scissor);
      
      vknvgBindFramebuffer(vgFbo, fbo);
      nvgBeginFrame(vgFbo, 200, 200, 1.0f);
      
      nvgBeginPath(vgFbo);
      nvgRect(vgFbo, 0, 0, 200, 200);
      nvgFillColor(vgFbo, nvgRGBA(0, 0, 255, 255));
      nvgFill(vgFbo);
      
      nvgEndFrame(vgFbo);
      vknvgBindFramebuffer(vgFbo, NULL);
      
      vkCmdEndRenderPass(fboCmd);
      vkEndCommandBuffer(fboCmd);
      
      VkSubmitInfo fboSubmit = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
      fboSubmit.commandBufferCount = 1;
      fboSubmit.pCommandBuffers = &fboCmd;
      vkQueueSubmit(queue, 1, &fboSubmit, VK_NULL_HANDLE);
      vkQueueWaitIdle(queue);

      // Render to screen
      prepareFrame(device->device, cmdBuffers[fb.current_frame], &fb);
      
      nvgBeginFrame(vg, winWidth, winHeight, pxRatio);

      // Draw rectangle with FBO texture
      nvgBeginPath(vg);
      nvgRect(vg, 100, 100, 200, 200);
      nvgFillPaint(vg, nvgImagePattern(vg, 100, 100, 200, 200, 0, vkfb->nvgImageHandle, 1.0f));
      nvgFill(vg);

      nvgEndFrame(vg);

      submitFrame(device->device, queue, cmdBuffers[fb.current_frame], &fb);
      
      glfwPollEvents();
   }

   vkDeviceWaitIdle(device->device);
   vknvgDeleteFramebuffer(vg, fbo);
   nvgDeleteVk(vg);
   destroyFrameBuffers(device, &fb, queue);
   free(cmdBuffers);
   destroyVulkanDevice(device);
   vkDestroySurfaceKHR(instance, surface, NULL);
   vkDestroyInstance(instance, NULL);
   glfwTerminate();
   
   return 0;
}
