// Vulkan demo - Vulkan backend
#include "demo.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define NANOVG_VULKAN_IMPLEMENTATION
#include <nanovg_vk.h>
#include "vulkan_util.h"


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
   window = glfwCreateWindow( 1000, 600, "NanoVG Demo - Vulkan", NULL, NULL);
   if( !window)
   {
      glfwTerminate();
      return( -1);
   }
   
   // Create Vulkan instance
   instance = createVkInstance( false);
   
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
      prepareFrame( device->device, cmdBuffer[fb.current_buffer], &fb);
      
      // Render with NanoVG
      nvgBeginFrame( vg, winWidth, winHeight, pxRatio);
      renderDemo( vg, mx, my, winWidth, winHeight, t, 0, &data);
      nvgEndFrame( vg);
      
      // Submit frame
      submitFrame( device->device, executionQueue, presentQueue, cmdBuffer[fb.current_buffer], &fb);
      
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
