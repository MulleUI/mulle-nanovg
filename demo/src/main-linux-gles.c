// Linux demo - OpenGL ES 3 backend
#define GLFW_INCLUDE_ES3
#include <GLFW/glfw3.h>
#include "demo.h"
#define NANOVG_GLES3_IMPLEMENTATION
#include <nanovg_gl.h>

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
	
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	
	window = glfwCreateWindow(1000, 600, "NanoVG Demo - GLES3", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return -1;
	}
	
	glfwMakeContextCurrent(window);
	
	vg = nvgCreateGLES3(NVG_ANTIALIAS | NVG_STENCIL_STROKES);
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
		
		glViewport(0, 0, fbWidth, fbHeight);
		glClearColor(0.3f, 0.3f, 0.32f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		
		nvgBeginFrame(vg, winWidth, winHeight, pxRatio);
		renderDemo(vg, mx, my, winWidth, winHeight, t, 0, &data);
		nvgEndFrame(vg);
		
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	
	freeDemoData(vg, &data);
	nvgDeleteGLES3(vg);
	
	glfwTerminate();
	return 0;
}
