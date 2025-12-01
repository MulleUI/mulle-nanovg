// macOS demo - Metal backend
#include "demo.h"
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#import <AppKit/AppKit.h>
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

int main(int argc, char** argv)
{
	GLFWwindow* window;
	NVGcontext* vg = NULL;
	DemoData data;
	double prevt = 0;
	
	if (!glfwInit()) {
		printf("Failed to init GLFW.\n");
		return -1;
	}
	
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	window = glfwCreateWindow(1000, 600, "NanoVG Demo - Metal", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return -1;
	}
	
	// Get the NSView and create Metal layer
	NSWindow* nswin = glfwGetCocoaWindow(window);
	NSView* view = [nswin contentView];
	[view setWantsLayer:YES];
	
	CAMetalLayer* metalLayer = [CAMetalLayer layer];
	metalLayer.device = MTLCreateSystemDefaultDevice();
	metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
	metalLayer.framebufferOnly = YES;
	[view setLayer:metalLayer];
	
	// Create NanoVG context
	vg = nvgCreateMTL((__bridge void*)metalLayer, NVG_ANTIALIAS);
	if (vg == NULL) {
		printf("Could not init nanovg.\n");
		return -1;
	}
	printf("NanoVG context created successfully\n");
	
	if (loadDemoData(vg, &data) == -1) {
		printf("Could not load demo data.\n");
		return -1;
	}
	
	glfwSetTime(0);
	prevt = glfwGetTime();
	
	while (!glfwWindowShouldClose(window)) {
		double mx, my, t, dt;
		int winWidth, winHeight;
		int fbWidth, fbHeight;
		float pxRatio;
		
		t = glfwGetTime();
		dt = t - prevt;
		prevt = t;
		
		glfwGetCursorPos(window, &mx, &my);
		glfwGetWindowSize(window, &winWidth, &winHeight);
		glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
		pxRatio = (float)fbWidth / (float)winWidth;
		
		// Update Metal layer size
		metalLayer.drawableSize = CGSizeMake(fbWidth, fbHeight);
		
		// Render
		nvgBeginFrame(vg, winWidth, winHeight, pxRatio);
		renderDemo(vg, mx, my, winWidth, winHeight, t, 0, &data);
		nvgEndFrame(vg);
		
		glfwPollEvents();
	}
	
	freeDemoData(vg, &data);
	nvgDeleteMTL(vg);
	
	glfwTerminate();
	return 0;
}
