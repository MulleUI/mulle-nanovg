#ifndef NANOVG_VK_UTILS_H
#define NANOVG_VK_UTILS_H

#include "nanovg-framebuffer.h"

void vknvgBindFramebuffer( NVGcontext *ctx, NVGframebuffer fb);
NVGframebuffer vknvgCreateFramebuffer( NVGcontext *ctx, int w, int h, int imageFlags);
void vknvgDeleteFramebuffer( NVGcontext *ctx, NVGframebuffer fb);
void vknvgClearWithColor( NVGcontext *ctx, NVGcolor color);
void vknvgReadPixels( NVGcontext *ctx, NVGframebuffer fb, int x, int y, int width, int height, void *data);

#endif

#ifdef NANOVG_VULKAN_IMPLEMENTATION

typedef struct VKNVGframebuffer
{
   VkFramebuffer   framebuffer;
   VkImage         colorImage;
   VkImageView     colorView;
   VkDeviceMemory  colorMem;
   VkImage         depthImage;
   VkImageView     depthView;
   VkDeviceMemory  depthMem;
   VkRenderPass    renderPass;
   int             nvgImageHandle;
   int             width;
   int             height;
} VKNVGframebuffer;


static VkResult vknvg_findMemoryType( VkPhysicalDevice gpu, uint32_t typeBits, VkFlags requirements, uint32_t *typeIndex)
{
   VkPhysicalDeviceMemoryProperties   memProps;
   uint32_t                           i;

   vkGetPhysicalDeviceMemoryProperties( gpu, &memProps);
   
   for( i = 0; i < memProps.memoryTypeCount; i++)
   {
      if( (typeBits & 1) == 1)
      {
         if( (memProps.memoryTypes[i].propertyFlags & requirements) == requirements)
         {
            *typeIndex = i;
            return( VK_SUCCESS);
         }
      }
      typeBits >>= 1;
   }
   
   return( VK_ERROR_FORMAT_NOT_SUPPORTED);
}


void vknvgBindFramebuffer( NVGcontext *ctx, NVGframebuffer fb)
{
   VKNVGcontext        *vk;
   VKNVGframebuffer    *vkfb;

   vk   = (VKNVGcontext *) nvgInternalParams( ctx)->userPtr;
   vkfb = (VKNVGframebuffer *) fb;
   
   vk->currentFramebuffer = vkfb ? vkfb->nvgImageHandle : 0;
}


NVGframebuffer vknvgCreateFramebuffer( NVGcontext *ctx, int w, int h, int imageFlags)
{
   VKNVGcontext                  *vk;
   VKNVGframebuffer              *vkfb;
   VkDevice                      device;
   VkPhysicalDevice              gpu;
   const VkAllocationCallbacks   *allocator;
   VkImageCreateInfo             imageInfo;
   VkMemoryRequirements          memReqs;
   VkMemoryAllocateInfo          allocInfo;
   VkImageViewCreateInfo         viewInfo;
   VkFramebufferCreateInfo       fbInfo;
   VkAttachmentDescription       attachments[2];
   VkAttachmentReference         colorRef;
   VkAttachmentReference         depthRef;
   VkSubpassDescription          subpass;
   VkRenderPassCreateInfo        rpInfo;
   VkImageView                   views[2];
   uint32_t                      memTypeIndex;
   VkResult                      res;

   vk        = (VKNVGcontext *) nvgInternalParams( ctx)->userPtr;
   device    = vk->createInfo.device;
   gpu       = vk->createInfo.gpu;
   allocator = vk->createInfo.allocator;
   
   vkfb = (VKNVGframebuffer *) malloc( sizeof( VKNVGframebuffer));
   if( ! vkfb)
      return( NULL);
   memset( vkfb, 0, sizeof( VKNVGframebuffer));
   
   vkfb->width  = w;
   vkfb->height = h;
   
   memset( &imageInfo, 0, sizeof( imageInfo));
   imageInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
   imageInfo.imageType     = VK_IMAGE_TYPE_2D;
   imageInfo.format        = VK_FORMAT_R8G8B8A8_UNORM;
   imageInfo.extent.width  = w;
   imageInfo.extent.height = h;
   imageInfo.extent.depth  = 1;
   imageInfo.mipLevels     = 1;
   imageInfo.arrayLayers   = 1;
   imageInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
   imageInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;
   imageInfo.usage         = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
   imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
   
   res = vkCreateImage( device, &imageInfo, allocator, &vkfb->colorImage);
   if( res != VK_SUCCESS)
      goto error;
   
   vkGetImageMemoryRequirements( device, vkfb->colorImage, &memReqs);
   
   res = vknvg_findMemoryType( gpu, memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memTypeIndex);
   if( res != VK_SUCCESS)
      goto error;
   
   memset( &allocInfo, 0, sizeof( allocInfo));
   allocInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
   allocInfo.allocationSize  = memReqs.size;
   allocInfo.memoryTypeIndex = memTypeIndex;
   
   res = vkAllocateMemory( device, &allocInfo, allocator, &vkfb->colorMem);
   if( res != VK_SUCCESS)
      goto error;
   
   vkBindImageMemory( device, vkfb->colorImage, vkfb->colorMem, 0);
   
   memset( &viewInfo, 0, sizeof( viewInfo));
   viewInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
   viewInfo.image                           = vkfb->colorImage;
   viewInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
   viewInfo.format                          = VK_FORMAT_R8G8B8A8_UNORM;
   viewInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
   viewInfo.subresourceRange.baseMipLevel   = 0;
   viewInfo.subresourceRange.levelCount     = 1;
   viewInfo.subresourceRange.baseArrayLayer = 0;
   viewInfo.subresourceRange.layerCount     = 1;
   
   res = vkCreateImageView( device, &viewInfo, allocator, &vkfb->colorView);
   if( res != VK_SUCCESS)
      goto error;
   
   imageInfo.format = VK_FORMAT_D24_UNORM_S8_UINT;
   imageInfo.usage  = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
   
   res = vkCreateImage( device, &imageInfo, allocator, &vkfb->depthImage);
   if( res != VK_SUCCESS)
      goto error;
   
   vkGetImageMemoryRequirements( device, vkfb->depthImage, &memReqs);
   
   res = vknvg_findMemoryType( gpu, memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memTypeIndex);
   if( res != VK_SUCCESS)
      goto error;
   
   allocInfo.allocationSize = memReqs.size;
   
   res = vkAllocateMemory( device, &allocInfo, allocator, &vkfb->depthMem);
   if( res != VK_SUCCESS)
      goto error;
   
   vkBindImageMemory( device, vkfb->depthImage, vkfb->depthMem, 0);
   
   viewInfo.image                       = vkfb->depthImage;
   viewInfo.format                      = VK_FORMAT_D24_UNORM_S8_UINT;
   viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
   
   res = vkCreateImageView( device, &viewInfo, allocator, &vkfb->depthView);
   if( res != VK_SUCCESS)
      goto error;
   
   memset( attachments, 0, sizeof( attachments));
   attachments[0].format         = VK_FORMAT_R8G8B8A8_UNORM;
   attachments[0].samples        = VK_SAMPLE_COUNT_1_BIT;
   attachments[0].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
   attachments[0].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
   attachments[0].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
   attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
   attachments[0].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
   attachments[0].finalLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
   
   attachments[1].format         = VK_FORMAT_D24_UNORM_S8_UINT;
   attachments[1].samples        = VK_SAMPLE_COUNT_1_BIT;
   attachments[1].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
   attachments[1].storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
   attachments[1].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
   attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
   attachments[1].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
   attachments[1].finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
   
   memset( &colorRef, 0, sizeof( colorRef));
   colorRef.attachment = 0;
   colorRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
   
   memset( &depthRef, 0, sizeof( depthRef));
   depthRef.attachment = 1;
   depthRef.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
   
   memset( &subpass, 0, sizeof( subpass));
   subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
   subpass.colorAttachmentCount    = 1;
   subpass.pColorAttachments       = &colorRef;
   subpass.pDepthStencilAttachment = &depthRef;
   
   memset( &rpInfo, 0, sizeof( rpInfo));
   rpInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
   rpInfo.attachmentCount = 2;
   rpInfo.pAttachments    = attachments;
   rpInfo.subpassCount    = 1;
   rpInfo.pSubpasses      = &subpass;
   
   res = vkCreateRenderPass( device, &rpInfo, allocator, &vkfb->renderPass);
   if( res != VK_SUCCESS)
      goto error;
   
   views[0] = vkfb->colorView;
   views[1] = vkfb->depthView;
   
   memset( &fbInfo, 0, sizeof( fbInfo));
   fbInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
   fbInfo.renderPass      = vkfb->renderPass;
   fbInfo.attachmentCount = 2;
   fbInfo.pAttachments    = views;
   fbInfo.width           = w;
   fbInfo.height          = h;
   fbInfo.layers          = 1;
   
   res = vkCreateFramebuffer( device, &fbInfo, allocator, &vkfb->framebuffer);
   if( res != VK_SUCCESS)
      goto error;
   
   vkfb->nvgImageHandle = vknvgCreateImageFromHandle( ctx, vkfb->colorView, w, h, imageFlags | NVG_IMAGE_PREMULTIPLIED);
   if( ! vkfb->nvgImageHandle)
      goto error;
   
   return( (NVGframebuffer) vkfb);

error:
   if( vkfb)
   {
      if( vkfb->framebuffer)
         vkDestroyFramebuffer( device, vkfb->framebuffer, allocator);
      if( vkfb->renderPass)
         vkDestroyRenderPass( device, vkfb->renderPass, allocator);
      if( vkfb->depthView)
         vkDestroyImageView( device, vkfb->depthView, allocator);
      if( vkfb->depthImage)
         vkDestroyImage( device, vkfb->depthImage, allocator);
      if( vkfb->depthMem)
         vkFreeMemory( device, vkfb->depthMem, allocator);
      if( vkfb->colorView)
         vkDestroyImageView( device, vkfb->colorView, allocator);
      if( vkfb->colorImage)
         vkDestroyImage( device, vkfb->colorImage, allocator);
      if( vkfb->colorMem)
         vkFreeMemory( device, vkfb->colorMem, allocator);
      free( vkfb);
   }
   return( NULL);
}


void vknvgDeleteFramebuffer( NVGcontext *ctx, NVGframebuffer fb)
{
   VKNVGcontext                  *vk;
   VKNVGframebuffer              *vkfb;
   VKNVGtexture                  *tex;
   VkDevice                      device;
   const VkAllocationCallbacks   *allocator;

   if( ! fb)
      return;
   
   vk        = (VKNVGcontext *) nvgInternalParams( ctx)->userPtr;
   vkfb      = (VKNVGframebuffer *) fb;
   device    = vk->createInfo.device;
   allocator = vk->createInfo.allocator;
   
   // Wait for device to finish using the framebuffer
   vkDeviceWaitIdle( device);
   
   // Clear the NanoVG texture entry without destroying the Vulkan image
   // (we manage the image ourselves)
   if( vkfb->nvgImageHandle)
   {
      tex = vknvg_findTexture( vk, vkfb->nvgImageHandle);
      if( tex)
      {
         // Destroy the sampler (created by vknvgCreateImageFromHandle)
         if( tex->sampler)
            vkDestroySampler( device, tex->sampler, allocator);
         // Clear the texture entry but don't destroy image/view/memory
         tex->image   = VK_NULL_HANDLE;
         tex->view    = VK_NULL_HANDLE;
         tex->mem     = VK_NULL_HANDLE;
         tex->sampler = VK_NULL_HANDLE;
      }
      // Delete the NanoVG image handle to free the texture slot
      nvgDeleteImage( ctx, vkfb->nvgImageHandle);
   }
   
   // Now delete Vulkan objects
   if( vkfb->framebuffer)
      vkDestroyFramebuffer( device, vkfb->framebuffer, allocator);
   if( vkfb->renderPass)
      vkDestroyRenderPass( device, vkfb->renderPass, allocator);
   if( vkfb->depthView)
      vkDestroyImageView( device, vkfb->depthView, allocator);
   if( vkfb->depthImage)
      vkDestroyImage( device, vkfb->depthImage, allocator);
   if( vkfb->depthMem)
      vkFreeMemory( device, vkfb->depthMem, allocator);
   if( vkfb->colorView)
      vkDestroyImageView( device, vkfb->colorView, allocator);
   if( vkfb->colorImage)
      vkDestroyImage( device, vkfb->colorImage, allocator);
   if( vkfb->colorMem)
      vkFreeMemory( device, vkfb->colorMem, allocator);
   
   free( vkfb);
}


void vknvgClearWithColor( NVGcontext *ctx, NVGcolor color)
{
   VKNVGcontext   *vk;

   vk               = (VKNVGcontext *) nvgInternalParams( ctx)->userPtr;
   vk->clearColor   = color;
   vk->pendingClear = 1;
}

void vknvgReadPixels( NVGcontext *ctx, NVGframebuffer fb, int x, int y, int width, int height, void *data)
{
   VKNVGcontext                  *vk;
   VKNVGframebuffer              *vkfb;
   VkDevice                      device;
   const VkAllocationCallbacks   *allocator;
   VkBuffer                      stagingBuffer;
   VkDeviceMemory                stagingMemory;
   VkBufferCreateInfo            bufferInfo;
   VkMemoryRequirements          memReqs;
   VkMemoryAllocateInfo          allocInfo;
   VkCommandBuffer               cmdBuf;
   VkCommandBufferAllocateInfo   cmdAllocInfo;
   VkCommandBufferBeginInfo      beginInfo;
   VkBufferImageCopy             region;
   VkImageMemoryBarrier          barrier;
   VkSubmitInfo                  submitInfo;
   void                          *mapped;
   uint32_t                      memTypeIndex;
   unsigned char                 *src;
   unsigned char                 *dst;
   int                           row;
   
   vk        = (VKNVGcontext *) nvgInternalParams( ctx)->userPtr;
   vkfb      = (VKNVGframebuffer *) fb;
   device    = vk->createInfo.device;
   allocator = vk->createInfo.allocator;
   
   if( ! vkfb || ! vkfb->colorImage)
      return;
   
   // Create staging buffer
   memset( &bufferInfo, 0, sizeof( bufferInfo));
   bufferInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
   bufferInfo.size        = width * height * 4;
   bufferInfo.usage       = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
   bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
   
   if( vkCreateBuffer( device, &bufferInfo, allocator, &stagingBuffer) != VK_SUCCESS)
      return;
   
   vkGetBufferMemoryRequirements( device, stagingBuffer, &memReqs);
   
   if( vknvg_findMemoryType( vk->createInfo.gpu, memReqs.memoryTypeBits, 
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                             &memTypeIndex) != VK_SUCCESS)
   {
      vkDestroyBuffer( device, stagingBuffer, allocator);
      return;
   }
   
   memset( &allocInfo, 0, sizeof( allocInfo));
   allocInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
   allocInfo.allocationSize  = memReqs.size;
   allocInfo.memoryTypeIndex = memTypeIndex;
   
   if( vkAllocateMemory( device, &allocInfo, allocator, &stagingMemory) != VK_SUCCESS)
   {
      vkDestroyBuffer( device, stagingBuffer, allocator);
      return;
   }
   
   vkBindBufferMemory( device, stagingBuffer, stagingMemory, 0);
   
   // Create command buffer from utility pool
   memset( &cmdAllocInfo, 0, sizeof( cmdAllocInfo));
   cmdAllocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
   cmdAllocInfo.commandPool        = vk->utilityCommandPool;
   cmdAllocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
   cmdAllocInfo.commandBufferCount = 1;
   
   if( vkAllocateCommandBuffers( device, &cmdAllocInfo, &cmdBuf) != VK_SUCCESS)
   {
      vkFreeMemory( device, stagingMemory, allocator);
      vkDestroyBuffer( device, stagingBuffer, allocator);
      return;
   }
   
   memset( &beginInfo, 0, sizeof( beginInfo));
   beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
   beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
   
   vkBeginCommandBuffer( cmdBuf, &beginInfo);
   
   // Transition image to transfer src
   memset( &barrier, 0, sizeof( barrier));
   barrier.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
   barrier.oldLayout           = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
   barrier.newLayout           = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
   barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
   barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
   barrier.image               = vkfb->colorImage;
   barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
   barrier.subresourceRange.baseMipLevel   = 0;
   barrier.subresourceRange.levelCount     = 1;
   barrier.subresourceRange.baseArrayLayer = 0;
   barrier.subresourceRange.layerCount     = 1;
   barrier.srcAccessMask       = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
   barrier.dstAccessMask       = VK_ACCESS_TRANSFER_READ_BIT;
   
   vkCmdPipelineBarrier( cmdBuf, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                         0, 0, NULL, 0, NULL, 1, &barrier);
   
   // Copy image to buffer
   memset( &region, 0, sizeof( region));
   region.bufferOffset      = 0;
   region.bufferRowLength   = 0;
   region.bufferImageHeight = 0;
   region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
   region.imageSubresource.mipLevel       = 0;
   region.imageSubresource.baseArrayLayer = 0;
   region.imageSubresource.layerCount     = 1;
   region.imageOffset.x = x;
   region.imageOffset.y = y;
   region.imageOffset.z = 0;
   region.imageExtent.width  = width;
   region.imageExtent.height = height;
   region.imageExtent.depth  = 1;
   
   vkCmdCopyImageToBuffer( cmdBuf, vkfb->colorImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                           stagingBuffer, 1, &region);
   
   // Transition back
   barrier.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
   barrier.newLayout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
   barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
   barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
   
   vkCmdPipelineBarrier( cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                         0, 0, NULL, 0, NULL, 1, &barrier);
   
   vkEndCommandBuffer( cmdBuf);
   
   // Submit and wait using context's queue
   memset( &submitInfo, 0, sizeof( submitInfo));
   submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
   submitInfo.commandBufferCount = 1;
   submitInfo.pCommandBuffers    = &cmdBuf;
   
   vkQueueSubmit( vk->queue, 1, &submitInfo, VK_NULL_HANDLE);
   vkQueueWaitIdle( vk->queue);
   
   // Map and copy
   if( vkMapMemory( device, stagingMemory, 0, VK_WHOLE_SIZE, 0, &mapped) == VK_SUCCESS)
   {
      src = (unsigned char *) mapped;
      dst = (unsigned char *) data;
      
      for( row = 0; row < height; row++)
      {
         memcpy( dst, src, width * 4);
         src += width * 4;
         dst += width * 4;
      }
      
      vkUnmapMemory( device, stagingMemory);
   }
   
   // Cleanup
   vkFreeCommandBuffers( device, vk->utilityCommandPool, 1, &cmdBuf);
   vkFreeMemory( device, stagingMemory, allocator);
   vkDestroyBuffer( device, stagingBuffer, allocator);
}

#endif
