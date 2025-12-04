// macOS Vulkan demo - MoltenVK backend
#include "demo.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define NANOVG_VULKAN_IMPLEMENTATION
#include <nanovg_vk.h>
#include "vulkan_util.h"


static void prepareFrame( VkDevice device, VkCommandBuffer cmdBuffer, FrameBuffers *fb)
{
   VkClearValue              clearValues[2];
   VkCommandBufferBeginInfo  cmdBufInfo;
   VkRect2D                  scissor;
   VkRenderPassBeginInfo     rpBegin;
   VkResult                  res;
   VkViewport                viewport;
   
   // Acquire next swapchain image
   res = vkAcquireNextImageKHR( device, fb->swap_chain, UINT64_MAX, 
                                fb->present_complete_semaphore[fb->current_frame],
                                VK_NULL_HANDLE, &fb->current_buffer);
   if( res == VK_ERROR_OUT_OF_DATE_KHR)
      return;
   
   // Begin command buffer
   memset( &cmdBufInfo, 0, sizeof( cmdBufInfo));
   cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
   vkBeginCommandBuffer( cmdBuffer, &cmdBufInfo);
   
   // Clear values
   clearValues[0].color.float32[0] = 0.3f;
   clearValues[0].color.float32[1] = 0.3f;
   clearValues[0].color.float32[2] = 0.32f;
   clearValues[0].color.float32[3] = 1.0f;
   clearValues[1].depthStencil.depth = 1.0f;
   clearValues[1].depthStencil.stencil = 0;
   
   // Begin render pass
   memset( &rpBegin, 0, sizeof( rpBegin));
   rpBegin.sType                    = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
   rpBegin.renderPass               = fb->render_pass;
   rpBegin.framebuffer              = fb->framebuffers[fb->current_buffer];
   rpBegin.renderArea.offset.x      = 0;
   rpBegin.renderArea.offset.y      = 0;
   rpBegin.renderArea.extent.width  = fb->buffer_size.width;
   rpBegin.renderArea.extent.height = fb->buffer_size.height;
   rpBegin.clearValueCount          = 2;
   rpBegin.pClearValues             = clearValues;
   
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


static void submitFrame( VkDevice device, VkQueue graphicsQueue, VkQueue presentQueue, 
                         VkCommandBuffer cmdBuffer, FrameBuffers *fb)
{
   VkImageMemoryBarrier   imageBarrier;
   VkPipelineStageFlags   pipeStageFlags;
   VkPresentInfoKHR       present;
   VkResult               res;
   VkSubmitInfo           submitInfo;
   
   vkCmdEndRenderPass( cmdBuffer);
   
   // Image barrier for presentation
   memset( &imageBarrier, 0, sizeof( imageBarrier));
   imageBarrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
   imageBarrier.srcAccessMask                   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
   imageBarrier.dstAccessMask                   = 0;
   imageBarrier.oldLayout                       = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
   imageBarrier.newLayout                       = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
   imageBarrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
   imageBarrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
   imageBarrier.image                           = fb->swap_chain_buffers[fb->current_buffer].image;
   imageBarrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
   imageBarrier.subresourceRange.baseMipLevel   = 0;
   imageBarrier.subresourceRange.levelCount     = 1;
   imageBarrier.subresourceRange.baseArrayLayer = 0;
   imageBarrier.subresourceRange.layerCount     = 1;
   
   vkCmdPipelineBarrier( cmdBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 
                        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, NULL, 0, NULL, 1, &imageBarrier);
   
   vkEndCommandBuffer( cmdBuffer);
   
   // Submit command buffer
   pipeStageFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
   memset( &submitInfo, 0, sizeof( submitInfo));
   submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
   submitInfo.waitSemaphoreCount   = 1;
   submitInfo.pWaitSemaphores      = &fb->present_complete_semaphore[fb->current_frame];
   submitInfo.pWaitDstStageMask    = &pipeStageFlags;
   submitInfo.commandBufferCount   = 1;
   submitInfo.pCommandBuffers      = &cmdBuffer;
   submitInfo.signalSemaphoreCount = 1;
   submitInfo.pSignalSemaphores    = &fb->render_complete_semaphore[fb->current_frame];
   
   vkQueueSubmit( presentQueue, 1, &submitInfo, fb->flight_fence[fb->current_frame]);
   
   // Present
   memset( &present, 0, sizeof( present));
   present.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
   present.swapchainCount     = 1;
   present.pSwapchains        = &fb->swap_chain;
   present.pImageIndices      = &fb->current_buffer;
   present.waitSemaphoreCount = 1;
   present.pWaitSemaphores    = &fb->render_complete_semaphore[fb->current_frame];
   
   res = vkQueuePresentKHR( presentQueue, &present);
   if( res == VK_ERROR_OUT_OF_DATE_KHR)
   {
      vkQueueWaitIdle( graphicsQueue);
      vkQueueWaitIdle( presentQueue);
      return;
   }
   
   fb->current_frame = (fb->current_frame + 1) % fb->swapchain_image_count;
   fb->num_swaps++;
   
   if( fb->num_swaps >= fb->swapchain_image_count)
   {
      vkWaitForFences( device, 1, &fb->flight_fence[fb->current_frame], VK_TRUE, UINT64_MAX);
      vkResetFences( device, 1, &fb->flight_fence[fb->current_frame]);
   }
}


int main( int argc, char** argv)
{
   DemoData          data;
   FrameBuffers      fb;
   GLFWwindow        *window;
   NVGcontext        *vg;
   VKNVGCreateInfo   createInfo;
   VkCommandBuffer   *cmdBuffer;
   VkInstance        instance;
   VkNvgExt          extQuery;
   VkQueue           executionQueue;
   VkQueue           presentQueue;
   VkResult          res;
   VkSurfaceKHR      surface;
   VulkanDevice      *device;
   double            dt, mx, my, prevt, t;
   float             pxRatio;
   int               fbHeight, fbWidth, winHeight, winWidth;
   uint32_t          gpuCount, gpuIdx, i;
   VkPhysicalDevice  gpu[32];
   
   printf( "Starting Vulkan demo...\n");
   fflush( stdout);
   
   vg       = NULL;
   prevt    = 0;
   device   = NULL;
   memset( &fb, 0, sizeof( fb));
   memset( &extQuery, 0, sizeof( extQuery));
   
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
   window = glfwCreateWindow( 1000, 600, "NanoVG Demo - MoltenVK", NULL, NULL);
   if( !window)
   {
      glfwTerminate();
      return( -1);
   }
   
   // Create Vulkan instance
   instance = createVkInstance( false);
   if( instance == VK_NULL_HANDLE)
   {
      printf( "Failed to create Vulkan instance. Is Vulkan installed?\n");
      glfwTerminate();
      return( -1);
   }
   printf( "Vulkan instance created\n");
   
   // Create surface
   res = glfwCreateWindowSurface( instance, window, NULL, &surface);
   if( res != VK_SUCCESS)
   {
      printf( "Failed to create window surface.\n");
      glfwTerminate();
      return( -1);
   }
   
   // Enumerate physical devices
   gpuCount = 0;
   res = vkEnumeratePhysicalDevices( instance, &gpuCount, NULL);
   if( res != VK_SUCCESS || gpuCount < 1)
   {
      printf( "No Vulkan device found.\n");
      glfwTerminate();
      return( -1);
   }
   
   vkEnumeratePhysicalDevices( instance, &gpuCount, gpu);
   
   // Select GPU with graphics queue
   gpuIdx = 0;
   for( i = 0; i < gpuCount; i++)
   {
      uint32_t   qfc;
      
      qfc = 0;
      vkGetPhysicalDeviceQueueFamilyProperties( gpu[i], &qfc, NULL);
      if( qfc >= 1)
      {
         gpuIdx = i;
         break;
      }
   }
   
   printf( "Using GPU device %u\n", gpuIdx);
   
   // Create logical device
   device = createVulkanDevice( gpu[gpuIdx], surface, &extQuery);
   if( device == NULL)
   {
      printf( "Failed to create Vulkan device\n");
      vkDestroySurfaceKHR( instance, surface, NULL);
      vkDestroyInstance( instance, NULL);
      glfwTerminate();
      return( -1);
   }
   printf( "Vulkan device created\n");
   
   // Get window size
   glfwGetWindowSize( window, &winWidth, &winHeight);
   
   // Get queues
   vkGetDeviceQueue( device->device, device->graphicsQueueFamilyIndex, 0, &executionQueue);
   vkGetDeviceQueue( device->device, device->graphicsQueueFamilyIndex, 0, &presentQueue);
   
   // Create framebuffers
   fb = createFrameBuffers( device, surface, executionQueue, winWidth, winHeight, 0);
   
   // Create command buffers
   cmdBuffer = createCmdBuffer( device->device, device->commandPool, fb.swapchain_image_count);
   
   // Create NanoVG context
   memset( &createInfo, 0, sizeof( createInfo));
   createInfo.device                = device->device;
   createInfo.gpu                   = device->gpu;
   createInfo.renderpass            = fb.render_pass;
   createInfo.cmdBuffer             = cmdBuffer;
   createInfo.swapchainImageCount   = fb.swapchain_image_count;
   createInfo.currentFrame          = &fb.current_frame;
   createInfo.ext.dynamicState      = extQuery.dynamicState;
   createInfo.ext.colorBlendEquation = extQuery.colorBlendEquation;
   createInfo.ext.colorWriteMask    = extQuery.colorWriteMask;
   
   vg = nvgCreateVk( createInfo, NVG_ANTIALIAS | NVG_STENCIL_STROKES, executionQueue);
   if( vg == NULL)
   {
      printf( "Could not init nanovg.\n");
      return( -1);
   }
   printf( "NanoVG context created successfully\n");
   
   if( loadDemoData( vg, &data) == -1)
   {
      printf( "Could not load demo data.\n");
      return( -1);
   }
   
   glfwSetTime( 0);
   prevt = glfwGetTime();
   
   while( !glfwWindowShouldClose( window))
   {
      t  = glfwGetTime();
      dt = t - prevt;
      prevt = t;
      
      glfwGetCursorPos( window, &mx, &my);
      glfwGetWindowSize( window, &winWidth, &winHeight);
      glfwGetFramebufferSize( window, &fbWidth, &fbHeight);
      pxRatio = (float)fbWidth / (float)winWidth;
      
      // Prepare frame
      prepareFrame( device->device, cmdBuffer[fb.current_frame], &fb);
      
      // Render with NanoVG
      nvgBeginFrame( vg, winWidth, winHeight, pxRatio);
      renderDemo( vg, mx, my, winWidth, winHeight, t, 0, &data);
      nvgEndFrame( vg);
      
      // Submit frame
      submitFrame( device->device, executionQueue, presentQueue, cmdBuffer[fb.current_frame], &fb);
      
      glfwPollEvents();
   }
   
   vkDeviceWaitIdle( device->device);
   
   freeDemoData( vg, &data);
   nvgDeleteVk( vg);
   
   destroyFrameBuffers( device, &fb, executionQueue);
   destroyVulkanDevice( device);
   vkDestroySurfaceKHR( instance, surface, NULL);
   vkDestroyInstance( instance, NULL);
   
   glfwTerminate();
   return( 0);
}
