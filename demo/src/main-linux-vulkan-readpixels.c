// Vulkan ReadPixels test - render to FBO, read pixels, save to PNG

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GLFW/glfw3.h>

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
   VulkanContext     vkCtx;
   int               fbSize;
   unsigned char     *pixels;

   fbSize = 512;

   if( !glfwInit())
   {
      printf( "Failed to init GLFW.\n");
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

   if( !initVulkan( &vkCtx, window, 100, 100))
   {
      printf( "Failed to init Vulkan.\n");
      glfwTerminate();
      return( -1);
   }

   vg = nvgCreateVk( vkCtx.device, vkCtx.physicalDevice, vkCtx.graphicsQueue, 
                     vkCtx.graphicsQueueFamily, vkCtx.renderPass, NVG_ANTIALIAS | NVG_STENCIL_STROKES);
   if( !vg)
   {
      printf( "Could not init nanovg.\n");
      return( -1);
   }

   printf( "Creating %dx%d framebuffer...\n", fbSize, fbSize);
   fb = vknvgCreateFramebuffer( vg, fbSize, fbSize, 0);
   if( fb == NULL)
   {
      printf( "Could not create FBO.\n");
      return( -1);
   }

   // Begin command buffer
   VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
   beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
   vkBeginCommandBuffer( vkCtx.commandBuffer, &beginInfo);

   // Get framebuffer info
   VKNVGframebuffer* vkfb = (VKNVGframebuffer*)(intptr_t)fb;

   // Begin render pass for FBO
   VkRenderPassBeginInfo rpInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
   rpInfo.renderPass = vkfb->renderPass;
   rpInfo.framebuffer = vkfb->framebuffer;
   rpInfo.renderArea.extent.width = fbSize;
   rpInfo.renderArea.extent.height = fbSize;
   
   VkClearValue clearValues[2];
   clearValues[0].color.float32[0] = 0.2f;
   clearValues[0].color.float32[1] = 0.2f;
   clearValues[0].color.float32[2] = 0.3f;
   clearValues[0].color.float32[3] = 1.0f;
   clearValues[1].depthStencil.depth = 1.0f;
   clearValues[1].depthStencil.stencil = 0;
   rpInfo.clearValueCount = 2;
   rpInfo.pClearValues = clearValues;

   vkCmdBeginRenderPass( vkCtx.commandBuffer, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

   printf( "Rendering to framebuffer...\n");
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

   vkCmdEndRenderPass( vkCtx.commandBuffer);
   vkEndCommandBuffer( vkCtx.commandBuffer);

   // Submit and wait
   VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
   submitInfo.commandBufferCount = 1;
   submitInfo.pCommandBuffers = &vkCtx.commandBuffer;
   vkQueueSubmit( vkCtx.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
   vkQueueWaitIdle( vkCtx.graphicsQueue);

   printf( "Reading pixels...\n");
   pixels = (unsigned char*)malloc( fbSize * fbSize * 4);
   vknvgReadPixels( vg, vkfb->nvgImageHandle, 0, 0, fbSize, fbSize, pixels);

   printf( "Saving to output.png...\n");
   stbi_write_png( "output.png", fbSize, fbSize, 4, pixels, fbSize * 4);

   free( pixels);
   vknvgDeleteFramebuffer( vg, fb);
   nvgDeleteVk( vg);
   cleanupVulkan( &vkCtx);
   glfwTerminate();
   
   printf( "Done! Check output.png\n");
   return( 0);
}
