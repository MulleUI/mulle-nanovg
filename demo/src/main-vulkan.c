// Vulkan demo - Minimal Vulkan backend example for NanoVG
//
// VULKAN BASICS FOR BEGINNERS:
// Vulkan is a low-level graphics API that gives you explicit control over the GPU.
// Unlike OpenGL, you must manually manage almost everything:
//   - Instance: The connection to the Vulkan library
//   - Physical Device (GPU): The actual graphics hardware
//   - Logical Device: Your application's interface to the GPU
//   - Queue: Where you submit commands to be executed
//   - Surface: The window where rendering appears
//   - Swapchain: A queue of images to display (double/triple buffering)
//   - Command Buffers: Records of GPU commands to execute
//   - Render Pass: Describes how rendering operations work
//   - Framebuffer: Attachments (color, depth) for a render pass
//   - Semaphores/Fences: Synchronization between GPU operations

#include "demo.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define NANOVG_VULKAN_IMPLEMENTATION
#include <nanovg_vk.h>
#include "vulkan_util.h"

// prepareFrame: Acquires next swapchain image and begins render pass
static void prepareFrame( VkDevice device, VkCommandBuffer cmdBuffer, FrameBuffers *fb)
{
   VkClearValue             clearValues[2];
   VkCommandBufferBeginInfo cmdBufInfo;
   VkRect2D                 scissor;
   VkRenderPassBeginInfo    rpBegin;
   VkResult                 res;
   VkViewport               viewport;
   
   // STEP 1: Acquire next image from swapchain
   // This gets the next available image to render into. We wait on a semaphore
   // that signals when the image is ready to use.
   res = vkAcquireNextImageKHR( device, fb->swap_chain, UINT64_MAX, 
                                fb->present_complete_semaphore[fb->current_frame],
                                VK_NULL_HANDLE, &fb->current_buffer);
   if( res == VK_ERROR_OUT_OF_DATE_KHR)
      return;  // Swapchain needs recreation (window resized, etc.)
   
   // STEP 2: Begin recording commands into command buffer
   memset( &cmdBufInfo, 0, sizeof( cmdBufInfo));
   cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
   vkBeginCommandBuffer( cmdBuffer, &cmdBufInfo);
   
   // STEP 3: Set clear values for color and depth/stencil attachments
   // Color: RGBA (here: black with full opacity)
   clearValues[0].color.float32[0]       = 0.0f;  // Red
   clearValues[0].color.float32[1]       = 0.0f;  // Green
   clearValues[0].color.float32[2]       = 0.0f;  // Blue
   clearValues[0].color.float32[3]       = 1.0f;  // Alpha
   // Depth/Stencil: depth=1.0 (far), stencil=0 (NanoVG uses stencil for clipping)
   clearValues[1].depthStencil.depth     = 1.0f;
   clearValues[1].depthStencil.stencil   = 0;
   
   // STEP 4: Begin render pass
   // A render pass defines what attachments to use and how to use them
   memset( &rpBegin, 0, sizeof( rpBegin));
   rpBegin.sType                         = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
   rpBegin.renderPass                    = fb->render_pass;
   rpBegin.framebuffer                   = fb->framebuffers[fb->current_buffer];
   rpBegin.renderArea.offset.x           = 0;
   rpBegin.renderArea.offset.y           = 0;
   rpBegin.renderArea.extent.width       = fb->buffer_size.width;
   rpBegin.renderArea.extent.height      = fb->buffer_size.height;
   rpBegin.clearValueCount               = 2;  // Color + depth/stencil
   rpBegin.pClearValues                  = clearValues;
   
   vkCmdBeginRenderPass( cmdBuffer, &rpBegin, VK_SUBPASS_CONTENTS_INLINE);
   
   // STEP 5: Set viewport (maps normalized device coordinates to pixels)
   viewport.width    = (float)fb->buffer_size.width;
   viewport.height   = (float)fb->buffer_size.height;
   viewport.minDepth = 0.0f;  // Near plane
   viewport.maxDepth = 1.0f;  // Far plane
   viewport.x        = 0.0f;
   viewport.y        = 0.0f;
   vkCmdSetViewport( cmdBuffer, 0, 1, &viewport);
   
   // STEP 6: Set scissor rectangle (clips rendering to this region)
   scissor = rpBegin.renderArea;
   vkCmdSetScissor( cmdBuffer, 0, 1, &scissor);
}

// submitFrame: Ends render pass, submits commands, and presents image
static void submitFrame( VkDevice device, VkQueue graphicsQueue, VkQueue presentQueue,
                         VkCommandBuffer cmdBuffer, FrameBuffers *fb)
{
   VkPipelineStageFlags waitStages;
   VkPresentInfoKHR     presentInfo;
   VkSubmitInfo         submitInfo;
   
   // STEP 1: End render pass and command buffer recording
   vkCmdEndRenderPass( cmdBuffer);
   vkEndCommandBuffer( cmdBuffer);
   
   // STEP 2: Submit command buffer to GPU queue
   // We wait for the present_complete semaphore (image is ready)
   // and signal render_complete semaphore (rendering is done)
   waitStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
   
   memset( &submitInfo, 0, sizeof( submitInfo));
   submitInfo.sType                   = VK_STRUCTURE_TYPE_SUBMIT_INFO;
   submitInfo.waitSemaphoreCount      = 1;
   submitInfo.pWaitSemaphores         = &fb->present_complete_semaphore[fb->current_frame];
   submitInfo.pWaitDstStageMask       = &waitStages;  // Wait before color output
   submitInfo.commandBufferCount      = 1;
   submitInfo.pCommandBuffers         = &cmdBuffer;
   submitInfo.signalSemaphoreCount    = 1;
   submitInfo.pSignalSemaphores       = &fb->render_complete_semaphore[fb->current_frame];
   
   vkQueueSubmit( graphicsQueue, 1, &submitInfo, fb->flight_fence[fb->current_frame]);
   
   // STEP 3: Present the rendered image to the screen
   // We wait for render_complete semaphore (rendering is done)
   memset( &presentInfo, 0, sizeof( presentInfo));
   presentInfo.sType                  = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
   presentInfo.waitSemaphoreCount     = 1;
   presentInfo.pWaitSemaphores        = &fb->render_complete_semaphore[fb->current_frame];
   presentInfo.swapchainCount         = 1;
   presentInfo.pSwapchains            = &fb->swap_chain;
   presentInfo.pImageIndices          = &fb->current_buffer;
   
   vkQueuePresentKHR( presentQueue, &presentInfo);
   
   // STEP 4: Advance to next frame (circular buffer)
   fb->current_frame = (fb->current_frame + 1) % fb->swapchain_image_count;
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
   
   vg       = NULL;
   prevt    = 0;
   device   = NULL;
   memset( &fb, 0, sizeof( fb));
   memset( &extQuery, 0, sizeof( extQuery));
   
   // ========== INITIALIZATION PHASE ==========
   
   // STEP 1: Initialize GLFW (window library)
   if( !glfwInit())
   {
      printf( "Failed to init GLFW.\n");
      return( -1);
   }
   
   // STEP 2: Check if Vulkan is supported on this system
   if( !glfwVulkanSupported())
   {
      printf( "Vulkan not supported.\n");
      glfwTerminate();
      return( -1);
   }
   
   // STEP 3: Create window (tell GLFW not to create OpenGL context)
   glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API);
   window = glfwCreateWindow( 1000, 600, "NanoVG Demo - Vulkan", NULL, NULL);
   if( !window)
   {
      glfwTerminate();
      return( -1);
   }
   
   // STEP 4: Create Vulkan instance
   // The instance is your application's connection to the Vulkan library.
   // It loads the Vulkan driver and enables extensions/layers.
   instance = createVkInstance( false);  // false = no validation layers
   
   // STEP 5: Create window surface
   // A surface is the connection between Vulkan and the window system.
   // It represents where the rendered images will be displayed.
   res = glfwCreateWindowSurface( instance, window, NULL, &surface);
   if( res != VK_SUCCESS)
   {
      printf( "Failed to create window surface.\n");
      glfwTerminate();
      return( -1);
   }
   
   // STEP 6: Enumerate physical devices (GPUs)
   // Query how many GPUs are available in the system
   gpuCount = 0;
   res = vkEnumeratePhysicalDevices( instance, &gpuCount, NULL);
   if( res != VK_SUCCESS || gpuCount < 1)
   {
      printf( "No Vulkan device found.\n");
      glfwTerminate();
      return( -1);
   }
   
   // Get the list of physical devices
   vkEnumeratePhysicalDevices( instance, &gpuCount, gpu);
   
   // STEP 7: Select a GPU with graphics queue support
   // Queue families are groups of queues with similar capabilities.
   // We need a graphics queue to submit rendering commands.
   gpuIdx = 0;
   for( i = 0; i < gpuCount; i++)
   {
      uint32_t   qfc;
      
      vkGetPhysicalDeviceQueueFamilyProperties( gpu[i], &qfc, NULL);
      if( qfc >= 1)  // Has at least one queue family
      {
         gpuIdx = i;
         break;
      }
   }
   
   printf( "Using GPU device %u\n", gpuIdx);
   
   // STEP 8: Create logical device
   // The logical device is your application's interface to the GPU.
   // It also queries for optional extensions (dynamic state, etc.)
   device = createVulkanDevice( gpu[gpuIdx], surface, &extQuery);
   
   // STEP 9: Get window dimensions
   glfwGetWindowSize( window, &winWidth, &winHeight);
   
   // STEP 10: Get device queues
   // Queues are where you submit command buffers for execution.
   // executionQueue: for rendering commands
   // presentQueue: for presenting images to screen (often same as execution)
   vkGetDeviceQueue( device->device, device->graphicsQueueFamilyIndex, 0, &executionQueue);
   vkGetDeviceQueue( device->device, device->graphicsQueueFamilyIndex, 0, &presentQueue);
   
   // STEP 11: Create swapchain and framebuffers
   // Swapchain: A queue of images for double/triple buffering
   // Framebuffers: Attachments (color + depth/stencil) for each swapchain image
   // Render pass: Describes how rendering operations work
   fb = createFrameBuffers( device, surface, executionQueue, winWidth, winHeight, 0);
   
   // STEP 12: Create command buffers
   // Command buffers record GPU commands (draw calls, state changes, etc.)
   // We need one per swapchain image for parallel recording
   cmdBuffer = createCmdBuffer( device->device, device->commandPool, fb.swapchain_image_count);
   
   // STEP 13: Create NanoVG context
   // NanoVG needs to know about the Vulkan device, render pass, and command buffers
   memset( &createInfo, 0, sizeof( createInfo));
   createInfo.device                = device->device;           // Logical device
   createInfo.gpu                   = device->gpu;              // Physical device
   createInfo.renderpass            = fb.render_pass;           // Render pass to use
   createInfo.cmdBuffer             = cmdBuffer;                // Command buffers
   createInfo.swapchainImageCount   = fb.swapchain_image_count; // Number of images
   createInfo.currentFrame          = &fb.current_frame;        // Frame index pointer
   createInfo.ext.dynamicState      = extQuery.dynamicState;    // Optional extensions
   createInfo.ext.colorBlendEquation = extQuery.colorBlendEquation;
   createInfo.ext.colorWriteMask    = extQuery.colorWriteMask;
   
   // NVG_ANTIALIAS: Enable anti-aliasing for smooth edges
   // NVG_STENCIL_STROKES: Use stencil buffer for stroke rendering
   vg = nvgCreateVk( createInfo, NVG_ANTIALIAS | NVG_STENCIL_STROKES, executionQueue);
   if( vg == NULL)
   {
      printf( "Could not init nanovg.\n");
      return( -1);
   }
   printf( "NanoVG context created successfully\n");
   
   // STEP 14: Load demo assets (fonts, images)
   if( loadDemoData( vg, &data) == -1)
   {
      printf( "Could not load demo data.\n");
      return( -1);
   }
   
   // ========== RENDER LOOP ==========
   
   glfwSetTime( 0);
   prevt = glfwGetTime();
   
   while( !glfwWindowShouldClose( window))
   {
      // Calculate delta time for animations
      t  = glfwGetTime();
      dt = t - prevt;
      prevt = t;
      
      // Get mouse position and window size
      glfwGetCursorPos( window, &mx, &my);
      glfwGetWindowSize( window, &winWidth, &winHeight);
      glfwGetFramebufferSize( window, &fbWidth, &fbHeight);
      pxRatio = (float)fbWidth / (float)winWidth;  // For HiDPI displays
      
      // RENDER STEP 1: Prepare frame (acquire image, begin render pass)
      prepareFrame( device->device, cmdBuffer[fb.current_buffer], &fb);
      
      // RENDER STEP 2: NanoVG rendering
      // nvgBeginFrame: Sets up NanoVG state for this frame
      nvgBeginFrame( vg, winWidth, winHeight, pxRatio);
      // renderDemo: Draws all the demo UI elements
      renderDemo( vg, mx, my, winWidth, winHeight, t, 0, &data);
      // nvgEndFrame: Flushes NanoVG commands to Vulkan command buffer
      nvgEndFrame( vg);
      
      // RENDER STEP 3: Submit frame (end render pass, submit to GPU, present)
      submitFrame( device->device, executionQueue, presentQueue, cmdBuffer[fb.current_buffer], &fb);
      
      // Process window events (keyboard, mouse, close button)
      glfwPollEvents();
   }
   
   // ========== CLEANUP PHASE ==========
   
   // Wait for GPU to finish all operations
   // THIS IS CRITICAL: Without this, you'll destroy resources while GPU is still using them,
   // causing driver crashes or GPU hangs. The OS can't clean up GPU state until GPU is idle.
   vkDeviceWaitIdle( device->device);
   
   // NOTE: The cleanup below is technically optional since the OS will reclaim all resources
   // when the process exits. However, we do it anyway for:
   //   1. Validation layers - they'll complain about leaked resources
   //   2. Good practice - shows proper resource ownership and destruction order
   //   3. Library use - if this code becomes reusable, you can't rely on process exit
   //   4. Memory leak detectors - keeps Valgrind/ASan happy
   
   // Free demo assets
   freeDemoData( vg, &data);
   
   // Destroy NanoVG context
   nvgDeleteVk( vg);
   
   // Destroy Vulkan resources in reverse order of creation
   destroyFrameBuffers( device, &fb, executionQueue);
   destroyVulkanDevice( device);
   vkDestroySurfaceKHR( instance, surface, NULL);
   vkDestroyInstance( instance, NULL);
   
   // Cleanup GLFW
   glfwTerminate();
   return( 0);
}
