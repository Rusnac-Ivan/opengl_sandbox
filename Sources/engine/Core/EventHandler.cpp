#include "EventHandler.h"
#include <Rendering/View.h>
#include <imgui.h>

#ifdef __EMSCRIPTEN__
	#include <emscripten/val.h>
#endif


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
	if (ImGui::GetIO().WantCaptureMouse)
		return;

	mView->OnMouseMove(xpos, ypos);
}

void EventHandler::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	if (ImGui::GetIO().WantCaptureMouse)
		return;

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
	mView->OnResize(width, height);
	
	std::string msg("FramebufferSizeCallback: " + std::to_string(width) + "x" + std::to_string(height));
#ifdef __EMSCRIPTEN__
	
	emscripten::val::global("console").call<void>("log", msg);
#else
	printf("%s\n", msg.c_str());
#endif
}

void EventHandler::WindowSizeCallback(GLFWwindow* window, int width, int height)
{
	std::string msg("WindowSizeCallback: " + std::to_string(width) + "x" + std::to_string(height));
#ifdef __EMSCRIPTEN__
	emscripten::val::global("console").call<void>("log", msg);
#else
	printf("%s\n", msg.c_str());
#endif
}