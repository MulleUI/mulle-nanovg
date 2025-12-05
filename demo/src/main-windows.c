// Windows demo - DirectX 11 backend
#include "demo.h"
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#define NANOVG_D3D11_IMPLEMENTATION
#include <nanovg_d3d11.h>
#include <d3d11.h>


int main( int argc, char** argv)
{
   DemoData                 data;
   DXGI_SWAP_CHAIN_DESC     swapChainDesc;
   GLFWwindow               *window;
   HRESULT                  hr;
   HWND                     hwnd;
   ID3D11Device             *device;
   ID3D11DeviceContext      *deviceContext;
   ID3D11RenderTargetView   *renderTargetView;
   ID3D11Texture2D          *backBuffer;
   IDXGISwapChain           *swapChain;
   NVGcontext               *vg;
   double                   dt, mx, my, prevt, t;
   float                    clearColor[4];
   float                    pxRatio;
   int                      fbHeight, fbWidth, winHeight, winWidth;
   
   device            = NULL;
   deviceContext     = NULL;
   swapChain         = NULL;
   renderTargetView  = NULL;
   vg                = NULL;
   prevt             = 0;
   
   if( !glfwInit())
   {
      printf( "Failed to init GLFW.\n");
      return( -1);
   }
   
   glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API);
   window = glfwCreateWindow( 1000, 600, "NanoVG Demo - DirectX 11", NULL, NULL);
   if( !window)
   {
      glfwTerminate();
      return( -1);
   }
   
   // Get native window handle
   hwnd = glfwGetWin32Window( window);
   
   // Create D3D11 device and swap chain
   memset( &swapChainDesc, 0, sizeof( swapChainDesc));
   swapChainDesc.BufferCount              = 1;
   swapChainDesc.BufferDesc.Format        = DXGI_FORMAT_R8G8B8A8_UNORM;
   swapChainDesc.BufferUsage              = DXGI_USAGE_RENDER_TARGET_OUTPUT;
   swapChainDesc.OutputWindow             = hwnd;
   swapChainDesc.SampleDesc.Count         = 1;
   swapChainDesc.Windowed                 = TRUE;
   
   hr = D3D11CreateDeviceAndSwapChain( NULL,
                                        D3D_DRIVER_TYPE_HARDWARE,
                                        NULL,
                                        0,
                                        NULL,
                                        0,
                                        D3D11_SDK_VERSION,
                                        &swapChainDesc,
                                        &swapChain,
                                        &device,
                                        NULL,
                                        &deviceContext);
   
   if( FAILED( hr))
   {
      printf( "Failed to create D3D11 device and swap chain.\n");
      glfwTerminate();
      return( -1);
   }
   
   // Create render target view
   backBuffer = NULL;
   swapChain->lpVtbl->GetBuffer( swapChain, 0, &IID_ID3D11Texture2D, (void**)&backBuffer);
   device->lpVtbl->CreateRenderTargetView( device, (ID3D11Resource*)backBuffer, NULL, &renderTargetView);
   backBuffer->lpVtbl->Release( backBuffer);
   
   // Create NanoVG context
   vg = nvgCreateD3D11( device, NVG_ANTIALIAS | NVG_STENCIL_STROKES);
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
      
      // Update swap chain if window resized
      if( fbWidth != swapChainDesc.BufferDesc.Width || fbHeight != swapChainDesc.BufferDesc.Height)
      {
         if( renderTargetView)
         {
            renderTargetView->lpVtbl->Release( renderTargetView);
            renderTargetView = NULL;
         }
         swapChain->lpVtbl->ResizeBuffers( swapChain, 1, fbWidth, fbHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
         swapChain->lpVtbl->GetBuffer( swapChain, 0, &IID_ID3D11Texture2D, (void**)&backBuffer);
         device->lpVtbl->CreateRenderTargetView( device, (ID3D11Resource*)backBuffer, NULL, &renderTargetView);
         backBuffer->lpVtbl->Release( backBuffer);
         swapChainDesc.BufferDesc.Width  = fbWidth;
         swapChainDesc.BufferDesc.Height = fbHeight;
      }
      
      // Clear and set render target
      clearColor[0] = 0.3f;
      clearColor[1] = 0.3f;
      clearColor[2] = 0.32f;
      clearColor[3] = 1.0f;
      deviceContext->lpVtbl->ClearRenderTargetView( deviceContext, renderTargetView, clearColor);
      deviceContext->lpVtbl->OMSetRenderTargets( deviceContext, 1, &renderTargetView, NULL);
      
      // Render with NanoVG
      nvgBeginFrame( vg, winWidth, winHeight, pxRatio);
      renderDemo( vg, mx, my, winWidth, winHeight, t, 0, &data);
      nvgEndFrame( vg);
      
      // Present
      swapChain->lpVtbl->Present( swapChain, 1, 0);
      
      glfwPollEvents();
   }
   
   freeDemoData( vg, &data);
   nvgDeleteD3D11( vg);
   
   if( renderTargetView)
      renderTargetView->lpVtbl->Release( renderTargetView);
   if( swapChain)
      swapChain->lpVtbl->Release( swapChain);
   if( deviceContext)
      deviceContext->lpVtbl->Release( deviceContext);
   if( device)
      device->lpVtbl->Release( device);
   
   glfwTerminate();
   return( 0);
}
