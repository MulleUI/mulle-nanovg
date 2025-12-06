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
   ID3D11DepthStencilView   *depthStencilView;
   ID3D11Texture2D          *backBuffer;
   ID3D11Texture2D          *depthStencil;
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
   depthStencilView  = NULL;
   depthStencil      = NULL;
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
   
   // Create depth-stencil buffer
   glfwGetFramebufferSize( window, &fbWidth, &fbHeight);
   D3D11_TEXTURE2D_DESC depthDesc = { 0 };
   depthDesc.Width = fbWidth;
   depthDesc.Height = fbHeight;
   depthDesc.MipLevels = 1;
   depthDesc.ArraySize = 1;
   depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
   depthDesc.SampleDesc.Count = 1;
   depthDesc.SampleDesc.Quality = 0;
   depthDesc.Usage = D3D11_USAGE_DEFAULT;
   depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
   depthDesc.CPUAccessFlags = 0;
   depthDesc.MiscFlags = 0;
   
   hr = device->lpVtbl->CreateTexture2D( device, &depthDesc, NULL, &depthStencil);
   if( FAILED( hr))
   {
      printf( "Failed to create depth-stencil texture.\n");
      glfwTerminate();
      return( -1);
   }
   
   D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = { 0 };
   dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
   dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
   dsvDesc.Texture2D.MipSlice = 0;
   
   hr = device->lpVtbl->CreateDepthStencilView( device, (ID3D11Resource*)depthStencil, &dsvDesc, &depthStencilView);
   if( FAILED( hr))
   {
      printf( "Failed to create depth-stencil view.\n");
      glfwTerminate();
      return( -1);
   }
   
   // Set viewport (required for rendering!)
   D3D11_VIEWPORT viewport = { 0 };
   viewport.Width = (float)fbWidth;
   viewport.Height = (float)fbHeight;
   viewport.MaxDepth = 1.0f;
   deviceContext->lpVtbl->RSSetViewports( deviceContext, 1, &viewport);
   
   // Create NanoVG context
   vg = nvgCreateD3D11( device, NVG_ANTIALIAS | NVG_STENCIL_STROKES);
   if( vg == NULL)
   {
      printf( "Could not init nanovg.\n");
      return( -1);
   }
   printf( "NanoVG context created successfully\n");
   
   printf( "About to load demo data...\n");
   if( loadDemoData( vg, &data) == -1)
   {
      printf( "Could not load demo data.\n");
      return( -1);
   }
   printf( "Demo data loaded successfully\n");
   
   glfwSetTime( 0);
   prevt = glfwGetTime();
   
   printf( "Starting render loop...\n");
   while( !glfwWindowShouldClose( window))
   {
      t  = glfwGetTime();
      dt = t - prevt;
      prevt = t;
      
      if( t < 0.1)  // Only print first frame
         printf( "Frame start: t=%.3f\n", t);
      
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
         if( depthStencilView)
         {
            depthStencilView->lpVtbl->Release( depthStencilView);
            depthStencilView = NULL;
         }
         if( depthStencil)
         {
            depthStencil->lpVtbl->Release( depthStencil);
            depthStencil = NULL;
         }
         
         swapChain->lpVtbl->ResizeBuffers( swapChain, 1, fbWidth, fbHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
         swapChain->lpVtbl->GetBuffer( swapChain, 0, &IID_ID3D11Texture2D, (void**)&backBuffer);
         device->lpVtbl->CreateRenderTargetView( device, (ID3D11Resource*)backBuffer, NULL, &renderTargetView);
         backBuffer->lpVtbl->Release( backBuffer);
         
         // Recreate depth-stencil buffer
         D3D11_TEXTURE2D_DESC depthDesc = { 0 };
         depthDesc.Width = fbWidth;
         depthDesc.Height = fbHeight;
         depthDesc.MipLevels = 1;
         depthDesc.ArraySize = 1;
         depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
         depthDesc.SampleDesc.Count = 1;
         depthDesc.SampleDesc.Quality = 0;
         depthDesc.Usage = D3D11_USAGE_DEFAULT;
         depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
         depthDesc.CPUAccessFlags = 0;
         depthDesc.MiscFlags = 0;
         device->lpVtbl->CreateTexture2D( device, &depthDesc, NULL, &depthStencil);
         
         D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = { 0 };
         dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
         dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
         dsvDesc.Texture2D.MipSlice = 0;
         device->lpVtbl->CreateDepthStencilView( device, (ID3D11Resource*)depthStencil, &dsvDesc, &depthStencilView);
         
         swapChainDesc.BufferDesc.Width  = fbWidth;
         swapChainDesc.BufferDesc.Height = fbHeight;
         
         // Update viewport after resize
         D3D11_VIEWPORT viewport = { 0 };
         viewport.Width = (float)fbWidth;
         viewport.Height = (float)fbHeight;
         viewport.MaxDepth = 1.0f;
         deviceContext->lpVtbl->RSSetViewports( deviceContext, 1, &viewport);
      }
      
      // Clear and set render target
      clearColor[0] = 0.3f;
      clearColor[1] = 0.3f;
      clearColor[2] = 0.32f;
      clearColor[3] = 1.0f;
      deviceContext->lpVtbl->ClearRenderTargetView( deviceContext, renderTargetView, clearColor);
      deviceContext->lpVtbl->ClearDepthStencilView( deviceContext, depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
      deviceContext->lpVtbl->OMSetRenderTargets( deviceContext, 1, &renderTargetView, depthStencilView);
      
      // Render with NanoVG
      if( t < 0.1)
         printf( "About to call nvgBeginFrame\n");
      nvgBeginFrame( vg, winWidth, winHeight, pxRatio);
      if( t < 0.1)
         printf( "About to call renderDemo\n");
      renderDemo( vg, mx, my, winWidth, winHeight, t, 0, &data);
      if( t < 0.1)
         printf( "About to call nvgEndFrame\n");
      nvgEndFrame( vg);
      
      // Present
      swapChain->lpVtbl->Present( swapChain, 1, 0);
      
      glfwPollEvents();
   }
   
   freeDemoData( vg, &data);
   nvgDeleteD3D11( vg);
   
   if( depthStencilView)
      depthStencilView->lpVtbl->Release( depthStencilView);
   if( depthStencil)
      depthStencil->lpVtbl->Release( depthStencil);
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
