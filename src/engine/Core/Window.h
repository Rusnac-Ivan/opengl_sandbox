#ifndef _WINDOW_H_
#define _WINDOW_H_

#include "Platform.h"
#include "EventHandler.h"
#include <Rendering/View.h>

#ifdef __EMSCRIPTEN__
	#define IMGUI_IMPL_OPENGL_ES2
#else
#endif


//#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <iostream>
#include <cstdint>
#include <memory>

class Window
{
public:
	Window() : mWidth(0), mHeight(0), mGLFWWindow(nullptr) {}
	~Window();

	void Create(uint32_t width, uint32_t height, const char *windowName);

	void OnInitialize();
	void OnRender();
	void OnFinalize();

	bool WindowIsOpen() { return !glfwWindowShouldClose(mGLFWWindow); }

	uint32_t GetWidth() { return mWidth; }
	uint32_t GetHeight() { return mHeight; }

	View* GetView() { return mView.get(); }

private:

	uint32_t mWidth;
	uint32_t mHeight;
	GLFWwindow *mGLFWWindow;
	std::string mGLSLVersion;
	std::unique_ptr<View> mView;

};

#endif