#ifndef NANOVG_VK_UTILS_H
#define NANOVG_VK_UTILS_H

void vknvgBindFramebuffer( NVGcontext *ctx, int img);
int vknvgCreateFramebuffer( NVGcontext *ctx, int w, int h, int imageFlags);

#endif

#ifdef NANOVG_VULKAN_IMPLEMENTATION

void vknvgBindFramebuffer( NVGcontext *ctx, int img)
{
   VKNVGcontext   *vk;

   vk                     = (VKNVGcontext *) nvgInternalParams( ctx)->userPtr;
   vk->currentFramebuffer = img;
}


int vknvgCreateFramebuffer( NVGcontext *ctx, int w, int h, int imageFlags)
{
   return( nvgCreateImageRGBA( ctx, w, h, imageFlags | NVG_IMAGE_PREMULTIPLIED, NULL));
}

#endif
