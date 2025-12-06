// Minimal D3D11 test - just clear red and draw green square with NanoVG
// The point of this is just to see if any nvg + D3D11 code is working

#include <stdio.h>
#include <math.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#define NANOVG_D3D11_IMPLEMENTATION
#include <nanovg_d3d11.h>

#include <d3d11.h>
#include <dxgi.h>

typedef struct D3D11Context
{
   ID3D11Device           *device;
   ID3D11DeviceContext    *context;
   IDXGISwapChain         *swapchain;
   ID3D11RenderTargetView *renderTarget;
   int                    width;
   int                    height;
} D3D11Context;

static int initD3D11( GLFWwindow *window, D3D11Context *d3d)
{
   DXGI_SWAP_CHAIN_DESC   scd;
   D3D_FEATURE_LEVEL      featureLevel;
   HRESULT                hr;
   HWND                   hwnd;
   ID3D11Texture2D        *backBuffer;
   
   hwnd = glfwGetWin32Window( window);
   
   memset( &scd, 0, sizeof( scd));
   scd.BufferCount                        = 1;
   scd.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
   scd.BufferDesc.Width                   = d3d->width;
   scd.BufferDesc.Height                  = d3d->height;
   scd.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
   scd.OutputWindow                       = hwnd;
   scd.SampleDesc.Count                   = 1;
   scd.SampleDesc.Quality                 = 0;
   scd.Windowed                           = TRUE;
   scd.SwapEffect                         = DXGI_SWAP_EFFECT_DISCARD;
   
   hr = D3D11CreateDeviceAndSwapChain( NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0,
                                       NULL, 0, D3D11_SDK_VERSION,
                                       &scd, &d3d->swapchain,
                                       &d3d->device, &featureLevel, &d3d->context);
   if( FAILED( hr))
   {
      printf( "Failed to create D3D11 device and swapchain\n");
      return( -1);
   }
   
   // Get back buffer
   hr = d3d->swapchain->lpVtbl->GetBuffer( d3d->swapchain, 0, &IID_ID3D11Texture2D, (void**)&backBuffer);
   if( FAILED( hr))
   {
      printf( "Failed to get back buffer\n");
      return( -1);
   }
   
   // Create render target view
   hr = d3d->device->lpVtbl->CreateRenderTargetView( d3d->device, (ID3D11Resource*)backBuffer, NULL, &d3d->renderTarget);
   backBuffer->lpVtbl->Release( backBuffer);
   if( FAILED( hr))
   {
      printf( "Failed to create render target view\n");
      return( -1);
   }
   
   return( 0);
}

static void cleanupD3D11( D3D11Context *d3d)
{
   if( d3d->renderTarget)
      d3d->renderTarget->lpVtbl->Release( d3d->renderTarget);
   if( d3d->swapchain)
      d3d->swapchain->lpVtbl->Release( d3d->swapchain);
   if( d3d->context)
      d3d->context->lpVtbl->Release( d3d->context);
   if( d3d->device)
      d3d->device->lpVtbl->Release( d3d->device);
}

int main( void)
{
   D3D11Context   d3d;
   GLFWwindow     *window;
   NVGcontext     *vg;
   float          clearColor[4];
   int            frameCount;
   
   memset( &d3d, 0, sizeof( d3d));
   d3d.width  = 800;
   d3d.height = 600;
   frameCount = 0;
   
   if( !glfwInit())
   {
      printf( "Failed to init GLFW\n");
      return( -1);
   }
   
   glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API);
   window = glfwCreateWindow( d3d.width, d3d.height, "Minimal D3D11 Test", NULL, NULL);
   if( !window)
   {
      printf( "Failed to create window\n");
      glfwTerminate();
      return( -1);
   }
   
   if( initD3D11( window, &d3d) < 0)
   {
      glfwTerminate();
      return( -1);
   }
   
   printf( "D3D11 initialized\n");
   
   // Create NanoVG context
   vg = nvgCreateD3D11( d3d.device, NVG_ANTIALIAS | NVG_STENCIL_STROKES);
   if( !vg)
   {
      printf( "Failed to create NanoVG context\n");
      cleanupD3D11( &d3d);
      glfwTerminate();
      return( -1);
   }
   
   printf( "NanoVG context created\n");
   printf( "Starting render loop...\n");
   
   // Clear color: red
   clearColor[0] = 1.0f;
   clearColor[1] = 0.0f;
   clearColor[2] = 0.0f;
   clearColor[3] = 1.0f;
   
   while( !glfwWindowShouldClose( window) && frameCount < 60)
   {
      // Clear to red
      d3d.context->lpVtbl->OMSetRenderTargets( d3d.context, 1, &d3d.renderTarget, NULL);
      d3d.context->lpVtbl->ClearRenderTargetView( d3d.context, d3d.renderTarget, clearColor);
      
      // Draw green square with NanoVG
      nvgBeginFrame( vg, d3d.width, d3d.height, 1.0f);
      
      nvgBeginPath( vg);
      nvgRect( vg, 100, 100, 200, 200);
      nvgFillColor( vg, nvgRGBA( 0, 255, 0, 255));
      nvgFill( vg);
      
      nvgEndFrame( vg);
      
      // Present
      d3d.swapchain->lpVtbl->Present( d3d.swapchain, 1, 0);
      
      glfwPollEvents();
      frameCount++;
   }
   
   printf( "Rendered %d frames\n", frameCount);
   
   nvgDeleteD3D11( vg);
   cleanupD3D11( &d3d);
   glfwTerminate();
   
   printf( "Done\n");
   return( 0);
}
