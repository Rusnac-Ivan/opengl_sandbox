#include "EventHandler.h"
#include <Rendering/View.h>
#include <imgui.h>



View* EventHandler::mView = nullptr;

void EventHandler::SetListener(View* view) 
{ 
	mView = view; 
}

void EventHandler::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (ImGui::GetIO().WantCaptureKeyboard)
		return;

	if (action == GLFW_PRESS)
		mView->OnKeyPress(key);
	if (action == GLFW_REPEAT)
		mView->OnKeyRepeat(key);
	if (action == GLFW_RELEASE)
		mView->OnKeyRelease(key);

	printf("KeyCallback\n");
}

void EventHandler::MouseMoveCallback(GLFWwindow* window, double xpos, double ypos)
{
	

	mView->OnMouseMove(xpos, ypos);
}

void EventHandler::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	//if (ImGui::GetIO().WantCaptureMouse)
		//return;

	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	if (action == GLFW_PRESS)
	{
		if (button == GLFW_MOUSE_BUTTON_LEFT)
		{
			mView->OnMouseLeftDown(xpos, ypos);
		}
		else if (button == GLFW_MOUSE_BUTTON_RIGHT)
		{
			mView->OnMouseRightDown(xpos, ypos);
		}
		else if (button == GLFW_MOUSE_BUTTON_MIDDLE)
		{
			mView->OnMouseMiddleDown(xpos, ypos);
		}
		printf("GLFW_PRESS\n");
	}
	else if (action == GLFW_RELEASE)
	{
		if (button == GLFW_MOUSE_BUTTON_LEFT)
		{
			mView->OnMouseLeftUp(xpos, ypos);
		}
		else if (button == GLFW_MOUSE_BUTTON_RIGHT)
		{
			mView->OnMouseRightUp(xpos, ypos);
		}
		else if (button == GLFW_MOUSE_BUTTON_MIDDLE)
		{
			mView->OnMouseMiddleUp(xpos, ypos);
		}
		printf("GLFW_RELEASE\n");
	}
}

void EventHandler::MouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (ImGui::GetIO().WantCaptureMouse)
		return;

	mView->OnMouseWhell(yoffset);
	printf("MouseScrollCallback\n");
}

void EventHandler::FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	//mView->OnResize(width, height);
	
	/*std::string msg("FramebufferSizeCallback: " + std::to_string(width) + "x" + std::to_string(height));
#ifdef __EMSCRIPTEN__
	
	emscripten::val::global("console").call<void>("log", msg);
#else
	printf("%s\n", msg.c_str());
#endif*/
}

void EventHandler::WindowSizeCallback(GLFWwindow* window, int width, int height)
{
	
	mView->OnResize(width, height);
	std::string msg("WindowSizeCallback: " + std::to_string(width) + "x" + std::to_string(height));
#ifdef __EMSCRIPTEN__
	emscripten::val::global("console").call<void>("log", msg);
	emscripten::val error = emscripten::val::global("Error").new_();
	emscripten::val::global("console").call<void>("log", error["stack"]);
#else
	printf("%s\n", msg.c_str());
#endif
}

#ifdef __EMSCRIPTEN__

	EM_BOOL EventHandler::emscripten_window_resized_callback(int eventType, const void* reserved, void* userData)
	{
		//METHOD();

		double width, height;
		emscripten_get_element_css_size("canvas", &width, &height);

		int w = (int)width, h = (int)height;

		// resize window
		GLFWwindow* window = (GLFWwindow*)userData;
		glfwSetWindowSize(window, w, h);

		printf("GLFWwindow*: %p\n", window);

		std::string msg("emscripten_window_resized_callback: " + std::to_string(width) + "x" + std::to_string(height));
		emscripten::val::global("console").call<void>("log", msg);

		// engine-specific code - internal render size should be updated here
		//event_t e(ET_WINDOW_SIZE, w, h, timer::current_time());
		//LOGI("Window resized to %dx%d", w, h);
		//platform->on_window_size_changed(w, h);
		mView->OnResize(width, height);
		return true;
	}

#endif