// Linux Vulkan FBO demo - Vulkan backend with render-to-texture
#include "demo.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define NANOVG_VULKAN_IMPLEMENTATION
#include <nanovg_vk.h>
#include <nanovg_vk_utils.h>
#include "vulkan_util.h"

static void renderPattern(NVGcontext* vgFbo, VKNVGframebuffer *vkfb, VkCommandBuffer cmdBuf,
                          int fboWidth, int fboHeight, float t, float pxRatio, DemoData* data)
{
   VkCommandBufferBeginInfo beginInfo;
   VkRenderPassBeginInfo    rpBegin;
   VkClearValue             clearValues[2];
   VkViewport               viewport;
   VkRect2D                 scissor;
   float                    cx, cy, r, s, sr;
   int                      ph, pw, winHeight, winWidth, x, y;

   s         = 20.0f;
   sr        = (cosf( t) + 1) * 0.5f;
   r         = s * 0.6f * (0.2f + 0.8f * sr);
   winWidth  = (int) (fboWidth / pxRatio);
   winHeight = (int) (fboHeight / pxRatio);

   memset( &beginInfo, 0, sizeof( beginInfo));
   beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
   beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
   vkBeginCommandBuffer( cmdBuf, &beginInfo);

   clearValues[0].color.float32[0] = 0.0f;
   clearValues[0].color.float32[1] = 0.0f;
   clearValues[0].color.float32[2] = 0.0f;
   clearValues[0].color.float32[3] = 0.0f;
   clearValues[1].depthStencil.depth   = 1.0f;
   clearValues[1].depthStencil.stencil = 0;

   memset( &rpBegin, 0, sizeof( rpBegin));
   rpBegin.sType                    = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
   rpBegin.renderPass               = vkfb->renderPass;
   rpBegin.framebuffer              = vkfb->framebuffer;
   rpBegin.renderArea.extent.width  = fboWidth;
   rpBegin.renderArea.extent.height = fboHeight;
   rpBegin.clearValueCount          = 2;
   rpBegin.pClearValues             = clearValues;

   vkCmdBeginRenderPass( cmdBuf, &rpBegin, VK_SUBPASS_CONTENTS_INLINE);

   memset( &viewport, 0, sizeof( viewport));
   viewport.width    = (float) fboWidth;
   viewport.height   = (float) fboHeight;
   viewport.minDepth = 0.0f;
   viewport.maxDepth = 1.0f;
   vkCmdSetViewport( cmdBuf, 0, 1, &viewport);

   memset( &scissor, 0, sizeof( scissor));
   scissor.extent.width  = fboWidth;
   scissor.extent.height = fboHeight;
   vkCmdSetScissor( cmdBuf, 0, 1, &scissor);

   nvgBeginFrame( vgFbo, winWidth, winHeight, pxRatio);

   pw = (int) ceilf( winWidth / s);
   ph = (int) ceilf( winHeight / s);

   nvgBeginPath( vgFbo);
   for( y = 0; y < ph; y++)
   {
      for( x = 0; x < pw; x++)
      {
         cx = (x + 0.5f) * s;
         cy = (y + 0.5f) * s;
         nvgCircle( vgFbo, cx, cy, r);
      }
   }
   nvgFillColor( vgFbo, nvgRGBA( 220, 160, 0, 200));
   nvgFill( vgFbo);

   nvgEndFrame( vgFbo);

   vkCmdEndRenderPass( cmdBuf);
   vkEndCommandBuffer( cmdBuf);
}

static void prepareFrame( VkDevice device, VkCommandBuffer cmdBuffer, FrameBuffers *fb)
{
   VkClearValue              clearValues[2];
   VkCommandBufferBeginInfo  cmdBufInfo;
   VkRenderPassBeginInfo     rpBegin;
   VkResult                  res;
   
   res = vkAcquireNextImageKHR( device, fb->swap_chain, UINT64_MAX, 
                                fb->present_complete_semaphore[fb->current_frame],
                                VK_NULL_HANDLE, &fb->current_buffer);
   if( res == VK_ERROR_OUT_OF_DATE_KHR)
      return;
   
   memset( &cmdBufInfo, 0, sizeof( cmdBufInfo));
   cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
   vkBeginCommandBuffer( cmdBuffer, &cmdBufInfo);
   
   clearValues[0].color.float32[0] = 0.3f;
   clearValues[0].color.float32[1] = 0.3f;
   clearValues[0].color.float32[2] = 0.32f;
   clearValues[0].color.float32[3] = 1.0f;
   clearValues[1].depthStencil.depth = 1.0f;
   clearValues[1].depthStencil.stencil = 0;
   
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
   VkViewport viewport;
   viewport.width = (float)fb->buffer_size.width;
   viewport.height = (float)fb->buffer_size.height;
   viewport.minDepth = 0.0f;
   viewport.maxDepth = 1.0f;
   viewport.x = 0.0f;
   viewport.y = 0.0f;
   vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);
   
   // Set scissor
   VkRect2D scissor = rpBegin.renderArea;
   vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);
}

static void submitFrame( VkDevice device, VkQueue queue, VkCommandBuffer cmdBuffer, FrameBuffers *fb)
{
   VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
   VkPresentInfoKHR     presentInfo;
   VkSubmitInfo         submitInfo;
   
   vkCmdEndRenderPass( cmdBuffer);
   vkEndCommandBuffer( cmdBuffer);
   
   memset( &submitInfo, 0, sizeof( submitInfo));
   submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
   submitInfo.waitSemaphoreCount   = 1;
   submitInfo.pWaitSemaphores      = &fb->present_complete_semaphore[fb->current_frame];
   submitInfo.pWaitDstStageMask    = waitStages;
   submitInfo.commandBufferCount   = 1;
   submitInfo.pCommandBuffers      = &cmdBuffer;
   submitInfo.signalSemaphoreCount = 1;
   submitInfo.pSignalSemaphores    = &fb->render_complete_semaphore[fb->current_frame];
   
   vkQueueSubmit( queue, 1, &submitInfo, fb->flight_fence[fb->current_frame]);
   
   memset( &presentInfo, 0, sizeof( presentInfo));
   presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
   presentInfo.waitSemaphoreCount = 1;
   presentInfo.pWaitSemaphores    = &fb->render_complete_semaphore[fb->current_frame];
   presentInfo.swapchainCount     = 1;
   presentInfo.pSwapchains        = &fb->swap_chain;
   presentInfo.pImageIndices      = &fb->current_buffer;
   
   vkQueuePresentKHR( queue, &presentInfo);
   
   fb->current_frame = (fb->current_frame + 1) % fb->swapchain_image_count;
   fb->num_swaps++;
   
   if( fb->num_swaps >= fb->swapchain_image_count)
   {
      vkWaitForFences( device, 1, &fb->flight_fence[fb->current_frame], VK_TRUE, UINT64_MAX);
      vkResetFences( device, 1, &fb->flight_fence[fb->current_frame]);
   }
}

int main(void)
{
   DemoData          data;
   FrameBuffers      fb;
   GLFWwindow        *window;
   NVGcontext        *vg, *vgFbo;
   NVGframebuffer    fbo;
   VKNVGCreateInfo   createInfo;
   VkCommandBuffer   *cmdBuffers, fboCmd;
   VkInstance        instance;
   VkNvgExt          extQuery;
   VkQueue           queue;
   VkResult          res;
   VkSurfaceKHR      surface;
   VulkanDevice      *device;
   VkSemaphore       fboCompleteSemaphore;
   int               fboWidth, fboHeight;
   double            prevt, t, mx, my;
   float             pxRatio;
   int               fbHeight, fbWidth, winHeight, winWidth, i;
   uint32_t          gpuCount, gpuIdx, j;
   VkPhysicalDevice  gpu[32];

   printf( "Starting Vulkan FBO demo...\n");
   
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
   window = glfwCreateWindow( 1000, 600, "NanoVG FBO Test (Vulkan)", NULL, NULL);
   if( !window)
   {
      glfwTerminate();
      return( -1);
   }

   // Create Vulkan instance
   instance = createVkInstance( false);
   if( instance == VK_NULL_HANDLE)
   {
      printf( "Failed to create Vulkan instance\n");
      glfwTerminate();
      return( -1);
   }
   printf( "Vulkan instance created\n");

   // Create surface
   res = glfwCreateWindowSurface( instance, window, NULL, &surface);
   if( res != VK_SUCCESS)
   {
      printf( "Failed to create surface\n");
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
   for( j = 0; j < gpuCount; j++)
   {
      uint32_t qfc = 0;
      vkGetPhysicalDeviceQueueFamilyProperties( gpu[j], &qfc, NULL);
      if( qfc >= 1)
      {
         gpuIdx = j;
         break;
      }
   }

   printf( "Using GPU device %u\n", gpuIdx);

   // Create logical device
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
   printf( "Vulkan device created\n");

   // Get window size
   glfwGetWindowSize( window, &winWidth, &winHeight);
   glfwGetFramebufferSize( window, &fbWidth, &fbHeight);
   pxRatio = (float) fbWidth / (float) winWidth;

   // Get queue
   vkGetDeviceQueue( device->device, device->graphicsQueueFamilyIndex, 0, &queue);

   // Create framebuffers
   fb = createFrameBuffers( device, surface, queue, winWidth, winHeight, 0);

   // Create command buffers
   cmdBuffers = createCmdBuffer( device->device, device->commandPool, fb.swapchain_image_count);

   // Create NanoVG context
   memset( &createInfo, 0, sizeof( createInfo));
   createInfo.device                = device->device;
   createInfo.gpu                   = device->gpu;
   createInfo.renderpass            = fb.render_pass;
   createInfo.cmdBuffer             = cmdBuffers;
   createInfo.swapchainImageCount   = fb.swapchain_image_count;
   createInfo.currentFrame          = &fb.current_frame;
   createInfo.ext.dynamicState      = extQuery.dynamicState;
   createInfo.ext.colorBlendEquation = extQuery.colorBlendEquation;
   createInfo.ext.colorWriteMask    = extQuery.colorWriteMask;

   vg = nvgCreateVk( createInfo, NVG_ANTIALIAS | NVG_STENCIL_STROKES, queue);
   if( !vg)
   {
      printf( "Could not init nanovg.\n");
      return( -1);
   }
   printf( "NanoVG context created\n");

   // Create framebuffer for render-to-texture
   fboWidth  = (int) (100 * pxRatio);
   fboHeight = (int) (100 * pxRatio);
   fbo = vknvgCreateFramebuffer( vg, fboWidth, fboHeight, NVG_IMAGE_REPEATX | NVG_IMAGE_REPEATY);
   if( fbo == NULL)
   {
      printf( "Could not create FBO.\n");
      return( -1);
   }
   printf( "FBO created\n");

   // Get framebuffer struct for accessing Vulkan objects
   VKNVGframebuffer *vkfb = (VKNVGframebuffer *) fbo;
   printf( "FBO image ID: %d\n", vkfb->nvgImageHandle);

   // Create dedicated command pool and buffer for FBO rendering
   VkCommandPoolCreateInfo poolInfo;
   VkCommandPool           fboPool;
   VkCommandBufferAllocateInfo allocInfo;
   
   memset( &poolInfo, 0, sizeof( poolInfo));
   poolInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
   poolInfo.queueFamilyIndex = device->graphicsQueueFamilyIndex;
   poolInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
   vkCreateCommandPool( device->device, &poolInfo, NULL, &fboPool);

   memset( &allocInfo, 0, sizeof( allocInfo));
   allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
   allocInfo.commandPool        = fboPool;
   allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
   allocInfo.commandBufferCount = 1;
   vkAllocateCommandBuffers( device->device, &allocInfo, &fboCmd);

   // Create second NanoVG context for offscreen rendering
   static uint32_t fboCurrentFrame = 0;
   VKNVGCreateInfo fboCreateInfo;
   
   memset( &fboCreateInfo, 0, sizeof( fboCreateInfo));
   fboCreateInfo.device                = device->device;
   fboCreateInfo.gpu                   = device->gpu;
   fboCreateInfo.renderpass            = vkfb->renderPass;
   fboCreateInfo.cmdBuffer             = &fboCmd;
   fboCreateInfo.swapchainImageCount   = 1;
   fboCreateInfo.currentFrame          = &fboCurrentFrame;
   fboCreateInfo.ext.dynamicState      = extQuery.dynamicState;
   fboCreateInfo.ext.colorBlendEquation = extQuery.colorBlendEquation;
   fboCreateInfo.ext.colorWriteMask     = extQuery.colorWriteMask;

   vgFbo = nvgCreateVk( fboCreateInfo, NVG_ANTIALIAS | NVG_STENCIL_STROKES, queue);
   if( !vgFbo)
   {
      printf( "Could not create offscreen NanoVG context.\n");
      return( -1);
   }
   printf( "Offscreen NanoVG context created\n");

   // Create semaphore for FBO completion signaling
   VkSemaphoreCreateInfo semInfo = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
   vkCreateSemaphore(device->device, &semInfo, NULL, &fboCompleteSemaphore);

   // Load demo data into offscreen context since that's where we render
   if( loadDemoData( vgFbo, &data) == -1)
      return( -1);

   glfwSetTime( 0);
   prevt = glfwGetTime();

   printf( "Entering main loop...\n");
   while( !glfwWindowShouldClose( window))
   {
      VKNVGframebuffer      *vkfb;
      VkSubmitInfo          fboSubmit;

      t = glfwGetTime();

      glfwGetCursorPos( window, &mx, &my);
      glfwGetWindowSize( window, &winWidth, &winHeight);
      glfwGetFramebufferSize( window, &fbWidth, &fbHeight);
      pxRatio = (float) fbWidth / (float) winWidth;

      vkfb = (VKNVGframebuffer *) fbo;

      // Render pattern to offscreen framebuffer every frame
      renderPattern( vgFbo, vkfb, fboCmd, fboWidth, fboHeight, (float) t, pxRatio, &data);

      // Submit FBO rendering and wait (like working demo)
      memset( &fboSubmit, 0, sizeof( fboSubmit));
      fboSubmit.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
      fboSubmit.commandBufferCount = 1;
      fboSubmit.pCommandBuffers    = &fboCmd;
      vkQueueSubmit( queue, 1, &fboSubmit, VK_NULL_HANDLE);
      vkQueueWaitIdle( queue);

      // Begin main frame
      prepareFrame( device->device, cmdBuffers[fb.current_frame], &fb);

      nvgBeginFrame( vg, winWidth, winHeight, pxRatio);

      // Draw colored bars
      for( i = 0; i < 20; i++)
      {
         nvgBeginPath( vg);
         nvgRect( vg, 10 + i * 30, 10, 10, winHeight - 20);
         nvgFillColor( vg, nvgHSLA( i / 19.0f, 0.5f, 0.5f, 255));
         nvgFill( vg);
      }

      // Draw moving rectangle with framebuffer texture
      if( fb.num_swaps == 10)
         printf( "Drawing with FBO image ID: %d\n", vkfb->nvgImageHandle);

      nvgBeginPath( vg);
      nvgRoundedRect( vg, 140 + sinf( (float) t * 1.3f) * 100, 140 + cosf( (float) t * 1.71244f) * 100, 250, 250, 20);
      nvgFillPaint( vg, nvgImagePattern( vg, 0, 0, 100, 100, 0, vkfb->nvgImageHandle, 1.0f));
      nvgFill( vg);

      nvgEndFrame( vg);

      // Submit frame
      submitFrame( device->device, queue, cmdBuffers[fb.current_frame], &fb);
      
      glfwPollEvents();
   }

   // Cleanup
   printf( "Cleaning up...\n");
   vkDeviceWaitIdle( device->device);
   freeDemoData( vgFbo, &data);  // Free from offscreen context where it was loaded
   nvgDeleteVk( vgFbo);  // Delete offscreen context first
   vknvgDeleteFramebuffer( vg, fbo);
   nvgDeleteVk( vg);
   destroyFrameBuffers( device, &fb, queue);
   free( cmdBuffers);
   destroyVulkanDevice( device);
   vkDestroySurfaceKHR( instance, surface, NULL);
   vkDestroyInstance( instance, NULL);
   glfwTerminate();
   
   printf( "Done.\n");
   return( 0);
}
