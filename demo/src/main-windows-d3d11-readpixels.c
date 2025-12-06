// D3D11 ReadPixels test - render to FBO, read pixels, save to PNG

#include <stdio.h>
#include <stdlib.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#define NANOVG_D3D11_IMPLEMENTATION
#include <nanovg.h>
#include <nanovg_d3d11.h>

#include <d3d11.h>
#include <dxgi.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// Minimal struct definitions
struct D3DNVGtexture {
	int id;
	ID3D11Texture2D* tex;
	ID3D11ShaderResourceView* resourceView;
    ID3D11RenderTargetView* renderTargetView;
	int width, height;
	int type;
	int flags;
};

struct D3DNVGcontext;
extern struct D3DNVGtexture* D3Dnvg__findTexture(struct D3DNVGcontext* D3D, int id);

int main(void)
{
   D3D11_DEPTH_STENCIL_VIEW_DESC   dsvDesc;
   D3D11_TEXTURE2D_DESC            depthDesc;
   D3D11_VIEWPORT                  viewport;
   DXGI_SWAP_CHAIN_DESC            scd;
   GLFWwindow                      *window;
   HRESULT                         hr;
   HWND                            hwnd;
   ID3D11Device                    *device;
   ID3D11DeviceContext             *context;
   ID3D11DepthStencilView          *depthStencil;
   ID3D11RenderTargetView          *renderTarget;
   ID3D11Texture2D                 *backBuffer;
   ID3D11Texture2D                 *depthBuffer;
   IDXGISwapChain                  *swapchain;
   NVGcontext                      *vg;
   NVGframebuffer                  fb;
   float                           clearColor[4];
   int                             fbSize;
   int                             fbWidth;
   int                             fbHeight;
   struct NVGparams                *params;
   struct D3DNVGcontext            *D3D;
   struct D3DNVGtexture            *tex;
   unsigned char                   *pixels;

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

   hwnd = glfwGetWin32Window( window);

   memset( &scd, 0, sizeof( scd));
   scd.BufferCount              = 1;
   scd.BufferDesc.Format        = DXGI_FORMAT_R8G8B8A8_UNORM;
   scd.BufferDesc.Width         = 100;
   scd.BufferDesc.Height        = 100;
   scd.BufferUsage              = DXGI_USAGE_RENDER_TARGET_OUTPUT;
   scd.OutputWindow             = hwnd;
   scd.SampleDesc.Count         = 1;
   scd.Windowed                 = TRUE;

   hr = D3D11CreateDeviceAndSwapChain( NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0,
                                       NULL, 0, D3D11_SDK_VERSION,
                                       &scd, &swapchain, &device, NULL, &context);
   if( FAILED( hr))
   {
      printf( "Failed to create D3D11 device.\n");
      glfwTerminate();
      return( -1);
   }

   swapchain->lpVtbl->GetBuffer( swapchain, 0, &IID_ID3D11Texture2D, (void**)&backBuffer);
   device->lpVtbl->CreateRenderTargetView( device, (ID3D11Resource*)backBuffer, NULL, &renderTarget);
   backBuffer->lpVtbl->Release( backBuffer);

   memset( &depthDesc, 0, sizeof( depthDesc));
   depthDesc.Width              = 100;
   depthDesc.Height             = 100;
   depthDesc.MipLevels          = 1;
   depthDesc.ArraySize          = 1;
   depthDesc.Format             = DXGI_FORMAT_D24_UNORM_S8_UINT;
   depthDesc.SampleDesc.Count   = 1;
   depthDesc.Usage              = D3D11_USAGE_DEFAULT;
   depthDesc.BindFlags          = D3D11_BIND_DEPTH_STENCIL;

   device->lpVtbl->CreateTexture2D( device, &depthDesc, NULL, &depthBuffer);

   memset( &dsvDesc, 0, sizeof( dsvDesc));
   dsvDesc.Format                  = DXGI_FORMAT_D24_UNORM_S8_UINT;
   dsvDesc.ViewDimension           = D3D11_DSV_DIMENSION_TEXTURE2D;

   device->lpVtbl->CreateDepthStencilView( device, (ID3D11Resource*)depthBuffer, &dsvDesc, &depthStencil);
   depthBuffer->lpVtbl->Release( depthBuffer);

   vg = nvgCreateD3D11( device, NVG_ANTIALIAS | NVG_STENCIL_STROKES);
   if( !vg)
   {
      printf( "Could not init nanovg.\n");
      return( -1);
   }

   printf( "Creating %dx%d framebuffer...\n", fbSize, fbSize);
   fb = d3dnvgCreateFramebuffer( vg, fbSize, fbSize, 0);
   if( fb == NULL)
   {
      printf( "Could not create FBO.\n");
      return( -1);
   }

   // Get texture and set as render target
   params = nvgInternalParams( vg);
   D3D    = (struct D3DNVGcontext*)params->userPtr;
   tex    = D3Dnvg__findTexture( D3D, (int)(intptr_t)fb);

   memset( &viewport, 0, sizeof( viewport));
   viewport.Width     = (float)fbSize;
   viewport.Height    = (float)fbSize;
   viewport.MaxDepth  = 1.0f;
   context->lpVtbl->RSSetViewports( context, 1, &viewport);
   context->lpVtbl->OMSetRenderTargets( context, 1, &tex->renderTargetView, depthStencil);

   // Clear
   clearColor[0] = 0.2f;
   clearColor[1] = 0.2f;
   clearColor[2] = 0.3f;
   clearColor[3] = 1.0f;
   context->lpVtbl->ClearRenderTargetView( context, tex->renderTargetView, clearColor);
   context->lpVtbl->ClearDepthStencilView( context, depthStencil, D3D11_CLEAR_STENCIL, 1.0f, 0);

   printf( "Rendering to framebuffer...\n");
   nvgBeginFrame( vg, fbSize, fbSize, 1.0f);

   // Draw some shapes
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

   // Read pixels
   printf( "Reading pixels...\n");
   pixels = (unsigned char*)malloc( fbSize * fbSize * 4);
   d3dnvgReadPixels( vg, (int)(intptr_t)fb, 0, 0, fbSize, fbSize, pixels);

   // Save to PNG
   printf( "Saving to output.png...\n");
   stbi_write_png( "output.png", fbSize, fbSize, 4, pixels, fbSize * 4);

   free( pixels);
   nvgDeleteImage( vg, (int)(intptr_t)fb);
   nvgDeleteD3D11( vg);

   depthStencil->lpVtbl->Release( depthStencil);
   renderTarget->lpVtbl->Release( renderTarget);
   swapchain->lpVtbl->Release( swapchain);
   context->lpVtbl->Release( context);
   device->lpVtbl->Release( device);

   glfwTerminate();
   
   printf( "Done! Check output.png\n");
   return( 0);
}
