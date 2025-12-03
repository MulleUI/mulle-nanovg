#ifndef NANOVG_VK_UTILS_H
#define NANOVG_VK_UTILS_H

void vknvgBindFramebuffer( NVGcontext *ctx, NVGframebuffer fb);
NVGframebuffer vknvgCreateFramebuffer( NVGcontext *ctx, int w, int h, int imageFlags);

#endif

#ifdef NANOVG_VULKAN_IMPLEMENTATION

void vknvgBindFramebuffer( NVGcontext *ctx, NVGframebuffer fb)
{
   VKNVGcontext   *vk;

   vk                     = (VKNVGcontext *) nvgInternalParams( ctx)->userPtr;
   vk->currentFramebuffer = (int) (intptr_t) fb;
}


NVGframebuffer vknvgCreateFramebuffer( NVGcontext *ctx, int w, int h, int imageFlags)
{
   int   image;
   
   image = nvgCreateImageRGBA( ctx, w, h, imageFlags | NVG_IMAGE_PREMULTIPLIED, NULL);
   return( (NVGframebuffer) (intptr_t) image);
}

#endif
