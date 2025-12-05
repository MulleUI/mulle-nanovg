// Minimal Vulkan test - just clear red and draw green square
// the point of this is just to see if any nvg + Vulkan code is working

#include <stdio.h>
#include <math.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define NANOVG_VULKAN_IMPLEMENTATION
#include <nanovg_vk.h>
#include "vulkan_util.h"

static void prepareFrame( VkDevice device, VkCommandBuffer cmdBuffer, FrameBuffers *fb)
{
   VkClearValue             clearValues[2];
   VkCommandBufferBeginInfo cmdBufInfo;
   VkRect2D                 scissor;
   VkRenderPassBeginInfo    rpBegin;
   VkResult                 res;
   VkViewport               viewport;
   
   res = vkAcquireNextImageKHR( device, fb->swap_chain, UINT64_MAX, 
                                fb->present_complete_semaphore[fb->current_frame],
                                VK_NULL_HANDLE, &fb->current_buffer);
   if( res == VK_ERROR_OUT_OF_DATE_KHR)
      return;
   
   memset( &cmdBufInfo, 0, sizeof( cmdBufInfo));
   cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
   vkBeginCommandBuffer( cmdBuffer, &cmdBufInfo);
   
   clearValues[0].color.float32[0]       = 1.0f;  // Red
   clearValues[0].color.float32[1]       = 0.0f;
   clearValues[0].color.float32[2]       = 0.0f;
   clearValues[0].color.float32[3]       = 1.0f;
   clearValues[1].depthStencil.depth     = 1.0f;
   clearValues[1].depthStencil.stencil   = 0;
   
   memset( &rpBegin, 0, sizeof( rpBegin));
   rpBegin.sType                         = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
   rpBegin.renderPass                    = fb->render_pass;
   rpBegin.framebuffer                   = fb->framebuffers[fb->current_buffer];
   rpBegin.renderArea.offset.x           = 0;
   rpBegin.renderArea.offset.y           = 0;
   rpBegin.renderArea.extent.width       = fb->buffer_size.width;
   rpBegin.renderArea.extent.height      = fb->buffer_size.height;
   rpBegin.clearValueCount               = 2;
   rpBegin.pClearValues                  = clearValues;
   
   vkCmdBeginRenderPass( cmdBuffer, &rpBegin, VK_SUBPASS_CONTENTS_INLINE);
   
   // Set viewport
   viewport.width    = (float)fb->buffer_size.width;
   viewport.height   = (float)fb->buffer_size.height;
   viewport.minDepth = 0.0f;
   viewport.maxDepth = 1.0f;
   viewport.x        = 0.0f;
   viewport.y        = 0.0f;
   vkCmdSetViewport( cmdBuffer, 0, 1, &viewport);
   
   // Set scissor
   scissor = rpBegin.renderArea;
   vkCmdSetScissor( cmdBuffer, 0, 1, &scissor);
}

static void submitFrame( VkDevice device, VkQueue queue, VkCommandBuffer cmdBuffer, FrameBuffers *fb)
{
   VkPipelineStageFlags waitStages;
   VkPresentInfoKHR     presentInfo;
   VkSubmitInfo         submitInfo;
   
   waitStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
   
   vkCmdEndRenderPass( cmdBuffer);
   vkEndCommandBuffer( cmdBuffer);
   
   memset( &submitInfo, 0, sizeof( submitInfo));
   submitInfo.sType                   = VK_STRUCTURE_TYPE_SUBMIT_INFO;
   submitInfo.waitSemaphoreCount      = 1;
   submitInfo.pWaitSemaphores         = &fb->present_complete_semaphore[fb->current_frame];
   submitInfo.pWaitDstStageMask       = &waitStages;
   submitInfo.commandBufferCount      = 1;
   submitInfo.pCommandBuffers         = &cmdBuffer;
   submitInfo.signalSemaphoreCount    = 1;
   submitInfo.pSignalSemaphores       = &fb->render_complete_semaphore[fb->current_frame];
   
   vkQueueSubmit( queue, 1, &submitInfo, fb->flight_fence[fb->current_frame]);
   
   memset( &presentInfo, 0, sizeof( presentInfo));
   presentInfo.sType                  = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
   presentInfo.waitSemaphoreCount     = 1;
   presentInfo.pWaitSemaphores        = &fb->render_complete_semaphore[fb->current_frame];
   presentInfo.swapchainCount         = 1;
   presentInfo.pSwapchains            = &fb->swap_chain;
   presentInfo.pImageIndices          = &fb->current_buffer;
   
   vkQueuePresentKHR( queue, &presentInfo);
   
   fb->current_frame = (fb->current_frame + 1) % fb->swapchain_image_count;
   fb->num_swaps++;
   
   if( fb->num_swaps >= fb->swapchain_image_count)
   {
      vkWaitForFences( device, 1, &fb->flight_fence[fb->current_frame], VK_TRUE, UINT64_MAX);
      vkResetFences( device, 1, &fb->flight_fence[fb->current_frame]);
   }
}

int main( void)
{
   FrameBuffers        fb;
   GLFWwindow          *window;
   NVGcontext          *vg;
   VKNVGCreateInfo     createInfo;
   VkCommandBuffer     *cmdBuffers;
   VkInstance          instance;
   VkNvgExt            extQuery;
   VkPhysicalDevice    gpu[32];
   VkQueue             queue;
   VkResult            res;
   VkSurfaceKHR        surface;
   VulkanDevice        *device;
   double              t;
   float               pxRatio;
   int                 fbHeight, fbWidth, frameCount, winHeight, winWidth;
   uint32_t            gpuCount, gpuIdx, j;

   printf( "Minimal Vulkan test...\n");
   
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
   window = glfwCreateWindow( 800, 600, "Minimal Vulkan Test", NULL, NULL);
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

   res = vkEnumeratePhysicalDevices( instance, &gpuCount, NULL);
   if( res != VK_SUCCESS || gpuCount < 1)
   {
      printf( "No Vulkan device found.\n");
      glfwTerminate();
      return( -1);
   }

   vkEnumeratePhysicalDevices( instance, &gpuCount, gpu);

   gpuIdx = 0;
   for( j = 0; j < gpuCount; j++)
   {
      uint32_t   qfc;
      
      vkGetPhysicalDeviceQueueFamilyProperties( gpu[j], &qfc, NULL);
      if( qfc >= 1)
      {
         gpuIdx = j;
         break;
      }
   }

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

   glfwGetWindowSize( window, &winWidth, &winHeight);
   glfwGetFramebufferSize( window, &fbWidth, &fbHeight);
   pxRatio = (float)fbWidth / (float)winWidth;

   vkGetDeviceQueue( device->device, device->graphicsQueueFamilyIndex, 0, &queue);

   fb         = createFrameBuffers( device, surface, queue, winWidth, winHeight, 0);
   cmdBuffers = createCmdBuffer( device->device, device->commandPool, fb.swapchain_image_count);

   memset( &createInfo, 0, sizeof( createInfo));
   createInfo.device                    = device->device;
   createInfo.gpu                       = device->gpu;
   createInfo.renderpass                = fb.render_pass;
   createInfo.cmdBuffer                 = cmdBuffers;
   createInfo.swapchainImageCount       = fb.swapchain_image_count;
   createInfo.currentFrame              = &fb.current_frame;
   createInfo.ext.dynamicState          = extQuery.dynamicState;
   createInfo.ext.colorBlendEquation    = extQuery.colorBlendEquation;
   createInfo.ext.colorWriteMask        = extQuery.colorWriteMask;

   vg = nvgCreateVk( createInfo, NVG_ANTIALIAS | NVG_STENCIL_STROKES, queue);
   if( !vg)
   {
      printf( "Could not init nanovg.\n");
      return( -1);
   }

   printf("Starting render loop...\n");
   glfwSetTime( 0);

   frameCount = 0;
   while( !glfwWindowShouldClose( window))
   {
      t = glfwGetTime();

      glfwGetWindowSize( window, &winWidth, &winHeight);
      glfwGetFramebufferSize( window, &fbWidth, &fbHeight);
      pxRatio = (float)fbWidth / (float)winWidth;

      prepareFrame( device->device, cmdBuffers[fb.current_frame], &fb);
      
      nvgBeginFrame( vg, winWidth, winHeight, pxRatio);

      // Draw green square
      nvgBeginPath( vg);
      nvgRect( vg, 100, 100, 200, 200);
      nvgFillColor( vg, nvgRGBA( 0, 255, 0, 255));
      nvgFill( vg);

      nvgEndFrame( vg);

      submitFrame( device->device, queue, cmdBuffers[fb.current_frame], &fb);
      
      glfwPollEvents();
   }

   vkDeviceWaitIdle( device->device);
   nvgDeleteVk( vg);
   destroyFrameBuffers( device, &fb, queue);
   free( cmdBuffers);
   destroyVulkanDevice( device);
   vkDestroySurfaceKHR( instance, surface, NULL);
   vkDestroyInstance( instance, NULL);
   glfwTerminate();
   
   return( 0);
}
