// macOS Vulkan FBO demo - MoltenVK backend with render-to-texture
#include "demo.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define NANOVG_VULKAN_IMPLEMENTATION
#include <nanovg_vk.h>
#include <nanovg_vk_utils.h>
#include "vulkan_util.h"

// Create render pass for offscreen FBO rendering
static VkRenderPass createFBORenderPass(VkDevice device, VkFormat colorFormat)
{
   VkAttachmentDescription colorAttachment = {0};
   VkAttachmentReference colorRef = {0};
   VkSubpassDescription subpass = {0};
   VkRenderPassCreateInfo rpInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
   VkRenderPass renderPass;

   colorAttachment.format = colorFormat;
   colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
   colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
   colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
   colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
   colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
   colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
   colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

   colorRef.attachment = 0;
   colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

   subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
   subpass.colorAttachmentCount = 1;
   subpass.pColorAttachments = &colorRef;

   rpInfo.attachmentCount = 1;
   rpInfo.pAttachments = &colorAttachment;
   rpInfo.subpassCount = 1;
   rpInfo.pSubpasses = &subpass;

   vkCreateRenderPass(device, &rpInfo, NULL, &renderPass);
   return renderPass;
}

// Create Vulkan framebuffer for NanoVG FBO texture
static VkFramebuffer createFBOFramebuffer(VkDevice device, VkRenderPass renderPass, 
                                          VkImageView imageView, int width, int height)
{
   VkFramebufferCreateInfo fbInfo = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
   VkFramebuffer framebuffer;

   fbInfo.renderPass = renderPass;
   fbInfo.attachmentCount = 1;
   fbInfo.pAttachments = &imageView;
   fbInfo.width = width;
   fbInfo.height = height;
   fbInfo.layers = 1;

   vkCreateFramebuffer(device, &fbInfo, NULL, &framebuffer);
   return framebuffer;
}

static void renderPattern(NVGcontext* vg, NVGframebuffer fbo, VkDevice device, VkCommandBuffer cmdBuf,
                          VkRenderPass renderPass, VkFramebuffer vkFbo, int fboWidth, int fboHeight,
                          float t, float pxRatio)
{
   VkCommandBufferBeginInfo beginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
   VkRenderPassBeginInfo rpBegin = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
   VkClearValue clearValue = {{{0.0f, 0.0f, 0.0f, 0.0f}}};
   float r, s, sr;
   int winWidth, winHeight, ph, pw, x, y;

   if(!fbo) return;

   s = 20.0f;
   sr = (cosf(t) + 1) * 0.5f;
   r = s * 0.6f * (0.2f + 0.8f * sr);

   winWidth = (int)(fboWidth / pxRatio);
   winHeight = (int)(fboHeight / pxRatio);

   beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
   vkBeginCommandBuffer(cmdBuf, &beginInfo);

   rpBegin.renderPass = renderPass;
   rpBegin.framebuffer = vkFbo;
   rpBegin.renderArea.extent.width = fboWidth;
   rpBegin.renderArea.extent.height = fboHeight;
   rpBegin.clearValueCount = 1;
   rpBegin.pClearValues = &clearValue;

   vkCmdBeginRenderPass(cmdBuf, &rpBegin, VK_SUBPASS_CONTENTS_INLINE);

   // Set viewport and scissor for FBO rendering
   VkViewport viewport = {0};
   viewport.width = (float)fboWidth;
   viewport.height = (float)fboHeight;
   viewport.minDepth = 0.0f;
   viewport.maxDepth = 1.0f;
   vkCmdSetViewport(cmdBuf, 0, 1, &viewport);

   VkRect2D scissor = {0};
   scissor.extent.width = fboWidth;
   scissor.extent.height = fboHeight;
   vkCmdSetScissor(cmdBuf, 0, 1, &scissor);

   vknvgBindFramebuffer(vg, fbo);
   nvgBeginFrame(vg, winWidth, winHeight, pxRatio);

   // Fill entire FBO with solid red
   nvgBeginPath(vg);
   nvgRect(vg, 0, 0, winWidth, winHeight);
   nvgFillColor(vg, nvgRGBA(255, 0, 0, 255));
   nvgFill(vg);

   nvgEndFrame(vg);
   vknvgBindFramebuffer(vg, NULL);

   vkCmdEndRenderPass(cmdBuf);
   
   // Transition image to SHADER_READ_ONLY_OPTIMAL for sampling
   VkImageMemoryBarrier barrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
   NVGparams* params = nvgInternalParams(vg);
   VKNVGcontext* vkCtx = (VKNVGcontext*)params->userPtr;
   VKNVGtexture* tex = vknvg_findTexture(vkCtx, (int)(intptr_t)fbo);
   
   barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
   barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
   barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
   barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
   barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
   barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
   barrier.image = tex->image;
   barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
   barrier.subresourceRange.levelCount = 1;
   barrier.subresourceRange.layerCount = 1;
   
   vkCmdPipelineBarrier(cmdBuf, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);
   
   // Update texture's layout field so NanoVG knows it's in SHADER_READ_ONLY_OPTIMAL
   tex->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
   
   vkEndCommandBuffer(cmdBuf);
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

static void submitFrame( VkDevice device, VkQueue queue, VkCommandBuffer cmdBuffer, FrameBuffers *fb, VkSemaphore fboSemaphore)
{
   VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT };
   VkSemaphore waitSemaphores[] = { fb->present_complete_semaphore[fb->current_frame], fboSemaphore };
   VkPresentInfoKHR     presentInfo;
   VkSubmitInfo         submitInfo;
   
   vkCmdEndRenderPass( cmdBuffer);
   vkEndCommandBuffer( cmdBuffer);
   
   memset( &submitInfo, 0, sizeof( submitInfo));
   submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
   submitInfo.waitSemaphoreCount   = 2;
   submitInfo.pWaitSemaphores      = waitSemaphores;
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

// Get Vulkan image view from NanoVG framebuffer
static VkImageView getFBOImageView(NVGcontext* vg, NVGframebuffer fbo)
{
   NVGparams* params = nvgInternalParams(vg);
   VKNVGcontext* vk = (VKNVGcontext*)params->userPtr;
   int imageId = (int)(intptr_t)fbo;
   VKNVGtexture* tex = vknvg_findTexture(vk, imageId);
   return tex ? tex->view : VK_NULL_HANDLE;
}

int main(void)
{
   DemoData          data;
   FrameBuffers      fb;
   GLFWwindow        *window;
   NVGcontext        *vg;
   NVGframebuffer    fbo;
   VKNVGCreateInfo   createInfo;
   VkCommandBuffer   *cmdBuffers, fboCmd;
   VkInstance        instance;
   VkNvgExt          extQuery;
   VkQueue           queue;
   VkResult          res;
   VkSurfaceKHR      surface;
   VulkanDevice      *device;
   VkRenderPass      fboRenderPass;
   VkFramebuffer     fboFramebuffer;
   VkImageView       fboImageView;
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
   window = glfwCreateWindow( 1000, 600, "NanoVG FBO Test (MoltenVK)", NULL, NULL);
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
   fboWidth = (int) (100 * pxRatio);
   fboHeight = (int) (100 * pxRatio);
   fbo = vknvgCreateFramebuffer( vg, fboWidth, fboHeight, NVG_IMAGE_REPEATX | NVG_IMAGE_REPEATY);
   if( fbo == NULL)
   {
      printf( "Could not create FBO.\n");
      return( -1);
   }
   printf( "FBO created\n");
   printf( "FBO image ID: %d\n", (int)(intptr_t)fbo);

   // Create Vulkan render pass for FBO
   fboRenderPass = createFBORenderPass( device->device, VK_FORMAT_R8G8B8A8_UNORM);
   
   // Get image view from NanoVG FBO
   fboImageView = getFBOImageView( vg, fbo);
   if( fboImageView == VK_NULL_HANDLE)
   {
      printf( "Could not get FBO image view.\n");
      return( -1);
   }
   
   // Create Vulkan framebuffer for FBO
   fboFramebuffer = createFBOFramebuffer( device->device, fboRenderPass, fboImageView, fboWidth, fboHeight);
   
   // Create dedicated command pool and buffer for FBO rendering
   VkCommandPoolCreateInfo poolInfo = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
   VkCommandPool fboPool;
   poolInfo.queueFamilyIndex = device->graphicsQueueFamilyIndex;
   poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
   vkCreateCommandPool(device->device, &poolInfo, NULL, &fboPool);
   
   VkCommandBufferAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
   allocInfo.commandPool = fboPool;
   allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
   allocInfo.commandBufferCount = 1;
   vkAllocateCommandBuffers(device->device, &allocInfo, &fboCmd);

   // Create semaphore for FBO completion signaling
   VkSemaphoreCreateInfo semInfo = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
   vkCreateSemaphore(device->device, &semInfo, NULL, &fboCompleteSemaphore);

   if( loadDemoData( vg, &data) == -1)
      return( -1);

   glfwSetTime( 0);
   prevt = glfwGetTime();

   printf( "Entering main loop...\n");
   while( !glfwWindowShouldClose( window))
   {
      t = glfwGetTime();

      glfwGetCursorPos( window, &mx, &my);
      glfwGetWindowSize( window, &winWidth, &winHeight);
      glfwGetFramebufferSize( window, &fbWidth, &fbHeight);
      pxRatio = (float) fbWidth / (float) winWidth;

      // Begin main command buffer first (NanoVG needs it for state tracking)
      VkCommandBufferBeginInfo mainBegin = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
      mainBegin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
      vkBeginCommandBuffer(cmdBuffers[fb.current_frame], &mainBegin);

      // Render pattern to offscreen framebuffer
      renderPattern( vg, fbo, device->device, fboCmd, fboRenderPass, fboFramebuffer, 
                     fboWidth, fboHeight, (float)t, pxRatio);
      
      // Submit FBO rendering with semaphore signal
      VkSubmitInfo fboSubmit = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
      fboSubmit.commandBufferCount = 1;
      fboSubmit.pCommandBuffers = &fboCmd;
      fboSubmit.signalSemaphoreCount = 1;
      fboSubmit.pSignalSemaphores = &fboCompleteSemaphore;
      vkQueueSubmit( queue, 1, &fboSubmit, VK_NULL_HANDLE);

      // End and restart main command buffer for actual frame rendering
      vkEndCommandBuffer(cmdBuffers[fb.current_frame]);

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
      int fboImageId = (int)(intptr_t)fbo;
      if(fb.num_swaps == 10) {
         printf("Drawing with FBO image ID: %d\n", fboImageId);
      }
      
      nvgBeginPath( vg);
      nvgRoundedRect( vg, 140 + sinf( (float)t * 1.3f) * 100, 140 + cosf( (float)t * 1.71244f) * 100, 250, 250, 20);
      nvgFillPaint( vg, nvgImagePattern( vg, 0, 0, 100, 100, 0, fboImageId, 1.0f));
      nvgFill( vg);

      nvgEndFrame( vg);

      // Submit frame
      submitFrame( device->device, queue, cmdBuffers[fb.current_frame], &fb, fboCompleteSemaphore);
      
      glfwPollEvents();
   }

   // Cleanup
   printf( "Cleaning up...\n");
   vkDeviceWaitIdle( device->device);
   vknvgDeleteFramebuffer( vg, fbo);
   freeDemoData( vg, &data);
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
