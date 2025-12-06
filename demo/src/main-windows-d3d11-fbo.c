//
// main-windows-d3d11-fbo.c - Framebuffer test for Windows/D3D11
//

#include <stdio.h>
#include <math.h>
#include <stdint.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#define NANOVG_D3D11_IMPLEMENTATION
#include <nanovg.h>
#include <nanovg_d3d11.h>

#include <d3d11.h>
#include <dxgi.h>

// Minimal struct definitions from nanovg_d3d11.c
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

static void renderPattern( NVGcontext* vg, ID3D11DeviceContext* context, ID3D11DepthStencilView* depthStencil,
                          NVGframebuffer fb, float t, float pxRatio)
{
   D3D11_VIEWPORT   viewport;
   float            clearColor[4];
   float            r;
   float            s;
   float            sr;
   int              fboHeight;
   int              fboWidth;
   int              ph;
   int              pw;
   int              winHeight;
   int              winWidth;
   int              x;
   int              y;
   struct NVGparams         *params;
   struct D3DNVGcontext     *D3D;
   struct D3DNVGtexture     *tex;

   if( fb == NULL)
      return;

   s  = 20.0f;
   sr = (cosf( t) + 1) * 0.5f;
   r  = s * 0.6f * (0.2f + 0.8f * sr);

   fboWidth  = (int) (100 * pxRatio);
   fboHeight = (int) (100 * pxRatio);
   winWidth  = (int) (fboWidth / pxRatio);
   winHeight = (int) (fboHeight / pxRatio);

   // Get the texture's render target view
   params = nvgInternalParams( vg);
   D3D    = (struct D3DNVGcontext*)params->userPtr;
   tex    = D3Dnvg__findTexture( D3D, (int)(intptr_t)fb);

   // Set FBO as render target
   memset( &viewport, 0, sizeof( viewport));
   viewport.Width     = (float)fboWidth;
   viewport.Height    = (float)fboHeight;
   viewport.MaxDepth  = 1.0f;
   context->lpVtbl->RSSetViewports( context, 1, &viewport);
   context->lpVtbl->OMSetRenderTargets( context, 1, &tex->renderTargetView, depthStencil);

   // Clear
   clearColor[0] = 0.0f;
   clearColor[1] = 0.0f;
   clearColor[2] = 0.0f;
   clearColor[3] = 0.0f;
   context->lpVtbl->ClearRenderTargetView( context, tex->renderTargetView, clearColor);
   context->lpVtbl->ClearDepthStencilView( context, depthStencil, D3D11_CLEAR_STENCIL, 1.0f, 0);

   nvgBeginFrame( vg, winWidth, winHeight, pxRatio);

   pw = (int) ceilf( winWidth / s);
   ph = (int) ceilf( winHeight / s);

   nvgBeginPath( vg);
   for( y = 0; y < ph; y++)
   {
      for( x = 0; x < pw; x++)
      {
         float   cx;
         float   cy;

         cx = (x + 0.5f) * s;
         cy = (y + 0.5f) * s;
         nvgCircle( vg, cx, cy, r);
      }
   }
   nvgFillColor( vg, nvgRGBA( 220, 160, 0, 200));
   nvgFill( vg);

   nvgEndFrame( vg);
}

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
   float                           pxRatio;
   int                             fbHeight;
   int                             fbWidth;
   int                             winHeight;
   int                             winWidth;

   if( !glfwInit())
   {
      printf( "Failed to init GLFW.\n");
      return( -1);
   }

   glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API);
   window = glfwCreateWindow( 1000, 600, "NanoVG FBO Test (D3D11)", NULL, NULL);
   if( !window)
   {
      glfwTerminate();
      return( -1);
   }

   hwnd = glfwGetWin32Window( window);

   glfwGetFramebufferSize( window, &fbWidth, &fbHeight);

   memset( &scd, 0, sizeof( scd));
   scd.BufferCount              = 1;
   scd.BufferDesc.Format        = DXGI_FORMAT_R8G8B8A8_UNORM;
   scd.BufferDesc.Width         = fbWidth;
   scd.BufferDesc.Height        = fbHeight;
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
   depthDesc.Width              = fbWidth;
   depthDesc.Height             = fbHeight;
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

   memset( &viewport, 0, sizeof( viewport));
   viewport.Width     = (float)fbWidth;
   viewport.Height    = (float)fbHeight;
   viewport.MaxDepth  = 1.0f;
   context->lpVtbl->RSSetViewports( context, 1, &viewport);

   vg = nvgCreateD3D11( device, NVG_ANTIALIAS | NVG_STENCIL_STROKES);
   if( !vg)
   {
      printf( "Could not init nanovg.\n");
      return( -1);
   }

   glfwGetWindowSize( window, &winWidth, &winHeight);
   glfwGetFramebufferSize( window, &fbWidth, &fbHeight);
   pxRatio = (float) fbWidth / (float) winWidth;

   fb = d3dnvgCreateFramebuffer( vg, (int) (100 * pxRatio), (int) (100 * pxRatio), NVG_IMAGE_REPEATX | NVG_IMAGE_REPEATY);
   if( fb == NULL)
   {
      printf( "Could not create FBO.\n");
      return( -1);
   }

   glfwSetTime( 0);

   while( !glfwWindowShouldClose( window))
   {
      double   mx;
      double   my;
      double   t;
      int      i;

      t = glfwGetTime();

      glfwGetCursorPos( window, &mx, &my);
      glfwGetWindowSize( window, &winWidth, &winHeight);
      glfwGetFramebufferSize( window, &fbWidth, &fbHeight);
      pxRatio = (float) fbWidth / (float) winWidth;

      renderPattern( vg, context, depthStencil, fb, t, pxRatio);

      // Restore viewport to window size
      memset( &viewport, 0, sizeof( viewport));
      viewport.Width     = (float)fbWidth;
      viewport.Height    = (float)fbHeight;
      viewport.MaxDepth  = 1.0f;
      context->lpVtbl->RSSetViewports( context, 1, &viewport);

      clearColor[0] = 0.3f;
      clearColor[1] = 0.3f;
      clearColor[2] = 0.32f;
      clearColor[3] = 1.0f;
      context->lpVtbl->ClearRenderTargetView( context, renderTarget, clearColor);
      context->lpVtbl->ClearDepthStencilView( context, depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
      context->lpVtbl->OMSetRenderTargets( context, 1, &renderTarget, depthStencil);

      nvgBeginFrame( vg, winWidth, winHeight, pxRatio);

      for( i = 0; i < 20; i++)
      {
         nvgBeginPath( vg);
         nvgRect( vg, 10 + i * 30, 10, 10, winHeight - 20);
         nvgFillColor( vg, nvgHSLA( i / 19.0f, 0.5f, 0.5f, 255));
         nvgFill( vg);
      }

      nvgBeginPath( vg);
      nvgRoundedRect( vg, 140 + sinf( t * 1.3f) * 100, 140 + cosf( t * 1.71244f) * 100, 250, 250, 20);
      nvgFillPaint( vg, nvgImagePattern( vg, 0, 0, 100, 100, 0, (int)(intptr_t)fb, 1.0f));
      nvgFill( vg);

      nvgEndFrame( vg);

      swapchain->lpVtbl->Present( swapchain, 1, 0);

      glfwPollEvents();
   }

   nvgDeleteImage( vg, (int)(intptr_t)fb);
   nvgDeleteD3D11( vg);

   depthStencil->lpVtbl->Release( depthStencil);
   renderTarget->lpVtbl->Release( renderTarget);
   swapchain->lpVtbl->Release( swapchain);
   context->lpVtbl->Release( context);
   device->lpVtbl->Release( device);

   glfwTerminate();
   return( 0);
}
