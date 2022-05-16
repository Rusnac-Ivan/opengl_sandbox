#pragma once
#include "imgui.h"      // IMGUI_IMPL_API
#include <Core/CallBack.h>

struct GLFWwindow;
struct GLFWmonitor;

using MouseButtonCallback = CallBack<void(GLFWwindow* window, int button, int action, int mods)>;
using WindowFocusCallback = CallBack<void(GLFWwindow* window, int focused)>;
using CursorEnterCallback = CallBack<void(GLFWwindow* window, int entered)>;
using ScrollCallback = CallBack<void(GLFWwindow* window, double xoffset, double yoffset)>;
using KeyCallback = CallBack<void(GLFWwindow* window, int key, int scancode, int action, int mods)>;
using CharCallback = CallBack<void(GLFWwindow* window, unsigned int codepoint)>;
using MonitorCallback = CallBack<void(GLFWmonitor* monitor, int event)>;
using WindowResizeCallback = CallBack<void(GLFWmonitor* monitor, int width, int height)>;


IMGUI_IMPL_API bool		ImGui_ImplGlfw_3d_to_2d_Init(GLFWwindow* window, bool install_callbacks);
IMGUI_IMPL_API void     ImGui_ImplGlfw_3d_to_2d_NewFrame(float mouse_x, float mouse_y);
IMGUI_IMPL_API void     ImGui_ImplGlfw_3d_to_2d_Shutdown();


// GLFW callbacks
// - When calling Init with 'install_callbacks=true': GLFW callbacks will be installed for you. They will call user's previously installed callbacks, if any.
// - When calling Init with 'install_callbacks=false': GLFW callbacks won't be installed. You will need to call those function yourself from your own GLFW callbacks.
IMGUI_IMPL_API void     ImGui_ImplGlfw_3d_to_2d_WindowFocusCallback(GLFWwindow* window, int focused);
IMGUI_IMPL_API void     ImGui_ImplGlfw_3d_to_2d_CursorEnterCallback(GLFWwindow* window, int entered);
IMGUI_IMPL_API void     ImGui_ImplGlfw_3d_to_2d_MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
IMGUI_IMPL_API void     ImGui_ImplGlfw_3d_to_2d_ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
IMGUI_IMPL_API void     ImGui_ImplGlfw_3d_to_2d_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
IMGUI_IMPL_API void     ImGui_ImplGlfw_3d_to_2d_CharCallback(GLFWwindow* window, unsigned int c);
IMGUI_IMPL_API void     ImGui_ImplGlfw_3d_to_2d_MonitorCallback(GLFWmonitor* monitor, int event);
IMGUI_IMPL_API void     ImGui_ImplGlfw_3d_to_2d_WindowResizeCallback(GLFWmonitor* monitor, int width, int height);

