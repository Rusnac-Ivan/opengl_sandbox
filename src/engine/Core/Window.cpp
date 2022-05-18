#include "Window.h"
#include <Rendering/imgui_impl_3d_to_2d.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#ifdef __EMSCRIPTEN__
#include "webxr.h"
#endif

#ifdef __EMSCRIPTEN__
static void OnError(int error_code)
{
	switch (error_code)
	{
	case WEBXR_ERR_API_UNSUPPORTED:
		printf("WebXR unsupported in this browser.\n");
		break;
	case WEBXR_ERR_GL_INCAPABLE:
		printf("GL context cannot be used to render to WebXR\n");
		break;
	case WEBXR_ERR_SESSION_UNSUPPORTED:
		printf("VR not supported on this device\n");
		break;
	default:
		printf("Unknown WebXR error with code\n");
	}
}
#endif

static void glfw_error_callback(int error, const char *description)
{
	fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

/*void GLAPIENTRY
MessageCallback(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam)
{
	fprintf(stderr, "OpenGL CALLBACK: %-s\n\ttype = 0x%x\n\tseverity = 0x%x\n\tmessage = %s\n\n",
		(type == GL_DEBUG_TYPE_ERROR ? "!!! GL ERROR !!!" : ""),
		type, severity, message);
	assert(type != GL_DEBUG_TYPE_ERROR && "OpenGL throw ERROR!");
}*/

void Window::Create(uint32_t width, uint32_t height, const char *windowName)
{
	glfwSetErrorCallback(glfw_error_callback);

	mWidth = width;
	mHeight = height;

	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW library!\n");
		exit(1);
	}

#ifdef __EMSCRIPTEN__
	mGLSLVersion = "#version 300 es";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#else
	mGLSLVersion = "#version 400";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif

	// glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 8);

	glfwWindowHint(GLFW_RESIZABLE, 1);
	// glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

#ifdef __EMSCRIPTEN__

	webxr_init(
		/* Frame callback */
		[](void *userData, int time, WebXRRigidTransform *headPose, WebXRView views[2], int viewCount)
		{
			View *thiz = ((Window *)userData)->mView.get();

			if (headPose)
			{
				thiz->_headPos = glm::vec3(headPose->position[0], headPose->position[1], headPose->position[2]);
				thiz->_headOrientation = glm::quat(headPose->orientation[0], headPose->orientation[1], headPose->orientation[2], headPose->orientation[3]);

			}

			thiz->_viewCount = viewCount;

			int viewIndex = 0;
			for (WebXRView view : {views[0], views[1]})
			{
				thiz->_viewports[viewIndex] = {view.viewport[0], view.viewport[1], view.viewport[2], view.viewport[3]};
				// thiz->_viewMatrices[viewIndex] = glm::transpose(glm::make_mat4(view.viewPose.matrix));
				thiz->_viewMatrices[viewIndex] = glm::translate(glm::mat4(1.), thiz->_headPos + glm::vec3(view.viewPose.position[0], view.viewPose.position[1], view.viewPose.position[2]));
				// https://stackoverflow.com/questions/48348509/glmquat-why-the-order-of-x-y-z-w-components-are-mixed
				glm::mat4 rot = glm::toMat4(glm::quat(view.viewPose.orientation[3], view.viewPose.orientation[0], view.viewPose.orientation[1], view.viewPose.orientation[2]));
				thiz->_viewMatrices[viewIndex] = glm::inverse(thiz->_viewMatrices[viewIndex] * rot);

				// glm::decompose(thiz->_viewMatrices[viewIndex], thiz->scale, thiz->rotation, thiz->translation, thiz->skew, thiz->perspective);

				thiz->_projectionMatrices[viewIndex] = (glm::make_mat4(view.projectionMatrix));
				++viewIndex;
			}

			constexpr int maxInputCount = 2;
			WebXRInputSource sources[maxInputCount];
			WebXRRigidTransform controllersPose[maxInputCount];

			webxr_get_input_sources(sources, maxInputCount, &thiz->_controllerCount);

			for (int i = 0; i < thiz->_controllerCount; ++i)
			{
				webxr_get_input_pose(sources + i, controllersPose + i);

				thiz->_controllerPos[i] = glm::vec3(controllersPose[i].position[0], controllersPose[i].position[1], controllersPose[i].position[2]);
				// thiz->_controllerOrientation[i] = glm::quat(controllersPose[i].orientation[3], controllersPose[i].orientation[0], controllersPose[i].orientation[1], controllersPose[i].orientation[2]);
				thiz->_controllerOrientation[i] = glm::rotate(glm::mat4(1.f), glm::radians(40.f), glm::vec3(-1.f, 0.f, 0.f));
				thiz->_controllerOrientation[i] = glm::toMat4(glm::quat(controllersPose[i].orientation[3], controllersPose[i].orientation[0], controllersPose[i].orientation[1], controllersPose[i].orientation[2])) * thiz->_controllerOrientation[i];

				// thiz->_controllerMatrix[i] = glm::make_mat4(controllersPose[i].matrix);

				thiz->_controllerMatrix[i] = glm::translate(glm::mat4(1.), thiz->_controllerPos[i]);
				thiz->_controllerMatrix[i] = thiz->_controllerMatrix[i] * thiz->_controllerOrientation[i];

				thiz->_controllerDir[i] = glm::vec3(thiz->_controllerOrientation[i] * glm::vec4(0.f, 0.f, -1.f, 1.f));
			}

			if (thiz->mReadyToDraw)
			{
				thiz->OnSceneDraw();
				thiz->OnGUIDraw();
			}
		},
		/* Session start callback */
		[](void *userData, int mode)
		{
			webxr_set_projection_params(0.01f, 100.f);

			printf("webxr_init: Session start callback\n");
			View *thiz = ((Window *)userData)->mView.get();

			webxr_set_select_start_callback([](WebXRInputSource *inputSource, void *userData)
											{ 
								printf("select_start_callback\n"); 
								ImGui_ImplGlfw_3d_to_2d_MouseButtonCallback(((Window *)userData)->mGLFWWindow, GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS, 0); },
											userData);

			webxr_set_select_end_callback([](WebXRInputSource *inputSource, void *userData)
										  { 
								printf("select_end_callback\n");
								ImGui_ImplGlfw_3d_to_2d_MouseButtonCallback(((Window *)userData)->mGLFWWindow, GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE, 0); },
										  userData);
		},
		/* Session end callback */
		[](void *userData, int mode)
		{
			printf("webxr_init: Session end callback\n");
		},
		/* Error callback */
		[](void *userData, int error)
		{
			printf("webxr_init: Errord callback\n");
			OnError(error);
		},
		/* userData */
		this);

	// GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	// const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	// assert(false);
	// glfwSetWindowMonitor(mGLFWWindow, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);

	// mGLFWWindow = glfwCreateWindow(mode->width, mode->height, windowName, monitor, NULL);
	int c_width,
		c_height, is_fullscreen;

	emscripten_get_canvas_size(&c_width, &c_height, &is_fullscreen);

	printf("emscripten_get_canvas_size %dx%d, is_fullscreen: %d\n", c_width, c_height, is_fullscreen);

	mGLFWWindow = glfwCreateWindow(c_width, c_height, windowName, nullptr, nullptr);
	mWidth = c_width;
	mHeight = c_height;

	if (true)
	{
		EmscriptenFullscreenStrategy strategy;
		strategy.scaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_STDDEF;
		strategy.filteringMode = EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT;
		strategy.canvasResizedCallback = EventHandler::emscripten_window_resized_callback;
		strategy.canvasResizedCallbackUserData = mGLFWWindow; // pointer to user data
		emscripten_enter_soft_fullscreen("canvas", &strategy);
	}

	printf("GLFWwindow*: %p\n", mGLFWWindow);

	webxr_is_session_supported(WEBXR_SESSION_MODE_IMMERSIVE_VR, [](int mode, int supported)
							   {
		printf("mode: %d, supported: %d\n", mode, supported);
		if ((mode == WEBXR_SESSION_MODE_IMMERSIVE_VR) && (supported))
		{
			//webxr_request_session(WEBXR_SESSION_MODE_IMMERSIVE_VR, WEBXR_SESSION_FEATURE_LOCAL, WEBXR_SESSION_FEATURE_LOCAL);
		} });
#else
	mGLFWWindow = glfwCreateWindow(mWidth, mHeight, windowName, nullptr, nullptr);
#endif

	if (mGLFWWindow == nullptr)
	{
		fprintf(stderr, "Failed to create GLFW window!\n");
		exit(1);
	}

	glfwMakeContextCurrent(mGLFWWindow);
	// glfwSwapInterval(1);

#ifndef __EMSCRIPTEN__
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		fprintf(stderr, "Failed to initialize GLAD!\n");
		exit(1);
	}
#endif

	// glEnable(GL_DEBUG_OUTPUT);
	// glDebugMessageCallback(MessageCallback, NULL);

#ifndef __EMSCRIPTEN__
	if (glfwRawMouseMotionSupported())
		glfwSetInputMode(mGLFWWindow, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
#endif

	glfwSetKeyCallback(mGLFWWindow, EventHandler::KeyCallback);
	glfwSetCursorPosCallback(mGLFWWindow, EventHandler::MouseMoveCallback);
	glfwSetMouseButtonCallback(mGLFWWindow, EventHandler::MouseButtonCallback);
	glfwSetScrollCallback(mGLFWWindow, EventHandler::MouseScrollCallback);
	glfwSetFramebufferSizeCallback(mGLFWWindow, EventHandler::FramebufferSizeCallback);
	glfwSetWindowSizeCallback(mGLFWWindow, EventHandler::WindowSizeCallback);
#ifdef __EMSCRIPTEN__
	// em_canvasresized_callback_func();
	// EmscriptenFullscreenStrategy.canvasResizedCallback;
	// emscripten_set_resize_callback(nullptr, nullptr, false, emscWindowSizeChanged);
#endif

	mView = std::make_unique<View>();
	mView->OnCreate(mWidth, mHeight);
}

Window::~Window()
{
	if (mGLFWWindow != nullptr)
	{
		glfwDestroyWindow(mGLFWWindow);
		glfwTerminate();
	}
}

static void MyStyleColors(ImGuiStyle *dst)
{
	ImGuiStyle &style = ImGui::GetStyle();

	style.FrameRounding = 0.f;
	style.FramePadding = ImVec2(5.f, 2.f);
	style.FrameBorderSize = 1.f;
	style.WindowBorderSize = 1.f;
	style.GrabRounding = 20.f;
	style.GrabMinSize = 18.f;
	style.WindowRounding = 0.f;
	style.DisplayWindowPadding = ImVec2(8.f, 8.f);
	style.WindowPadding = ImVec2(8.f, 8.f);
	style.ItemInnerSpacing = ImVec2(8.f, 4.f);

	// float active_col_orange[3] = { 255.f / 255.f, 129.f / 255.f, 54.f / 255.f };
	// float hover_col_orange[3] = { 0.8f, 0.4f, 0.f };
	// float default_col_orange[3] = { 235.f / 255.f, 109.f / 255.f, 34.f / 255.f };

	float active_col_white[3] = {0.8f, 0.8f, 0.8f};
	float hover_col_white[3] = {0.7f, 0.7f, 0.7f};
	float default_col_white[3] = {0.6f, 0.6f, 0.6f};

	float default_col[3] = {66.f / 255.f, 68.f / 255.f, 69.f / 255.f};
	float hover_col[3] = {56.f / 255.f, 58.f / 255.f, 59.f / 255.f};
	float active_col[3] = {46.f / 255.f, 48.f / 255.f, 49.f / 255.f};

	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.25f, 0.25f, 0.25f, 0.8f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(default_col[0], default_col[1], default_col[2], 1.f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(active_col[0], active_col[1], active_col[2], 1.f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(default_col[0], default_col[1], default_col[2], 1.f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(active_col[0], active_col[1], active_col[2], 1.f);
	style.Colors[ImGuiCol_Header] = ImVec4(default_col[0], default_col[1], default_col[2], 1.f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(hover_col[0], hover_col[1], hover_col[2], 1.f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(active_col[0], active_col[1], active_col[2], 1.f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(active_col_white[0], active_col_white[1], active_col_white[2], 0.16f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(active_col_white[0], active_col_white[1], active_col_white[2], 0.32f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(active_col_white[0], active_col_white[1], active_col_white[2], 0.48f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(active_col_white[0], active_col_white[1], active_col_white[2], 1.0f);
	style.Colors[ImGuiCol_Tab] = ImVec4(default_col[0], default_col[1], default_col[2], 0.5f);
	style.Colors[ImGuiCol_TabHovered] = ImVec4(hover_col[0], hover_col[1], hover_col[2], 1.0f);
	style.Colors[ImGuiCol_TabActive] = ImVec4(active_col[0], active_col[1], active_col[2], 1.0f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(active_col[0], active_col[1], active_col[2], 0.2f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(active_col[0], active_col[1], active_col[2], 0.4f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(active_col[0], active_col[1], active_col[2], 0.6f);
	style.Colors[ImGuiCol_Separator] = ImVec4(active_col[0], active_col[1], active_col[2], 0.6f);
	style.Colors[ImGuiCol_Button] = ImVec4(default_col[0], default_col[1], default_col[2], 1.0f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(active_col_white[0], active_col_white[1], active_col_white[2], 0.6f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(active_col_white[0], active_col_white[1], active_col_white[2], 1.0f);
	style.Colors[ImGuiCol_Text] = ImVec4(1.f, 1.f, 1.f, 1.f);
}

void Window::OnInitialize()
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();

	io.WantCaptureMouse = false;
	io.WantCaptureKeyboard = false;

	// Setup Dear ImGui style
	// ImGui::StyleColorsDark();
	// ImGui::StyleColorsLight();
	ImGui::StyleColorsClassic();

	// Setup Platform/Renderer backends
	// ImGui_ImplGlfw_InitForOpenGL(mGLFWWindow, true);
#ifndef __EMSCRIPTEN__
	ImGui_ImplGlfw_3d_to_2d_Init(mGLFWWindow, false);
	ImGui_ImplOpenGL3_Init(mGLSLVersion.c_str());
#else
	ImGui_ImplGlfw_3d_to_2d_Init(mGLFWWindow, false);
	ImGui_ImplOpenGL3_Init(mGLSLVersion.c_str());
#endif
	

	mView->OnInitialize();
}
void Window::OnRender()
{
	mView->OnSceneDraw();

	// Start the Dear ImGui frame
	// ImGui_ImplOpenGL3_NewFrame();
	// ImGui_ImplGlfw_NewFrame();
	// ImGui::NewFrame();

	mView->OnGUIDraw();
	std::string info(std::to_string(ImGui::GetIO().MousePos.x) + ", " + std::to_string(ImGui::GetIO().MousePos.y));
	// glfwSetWindowTitle(mGLFWWindow, info.c_str());

	// Rendering
	// ImGui::Render();
	// ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	glfwSwapBuffers(mGLFWWindow);
	glfwPollEvents();
}
void Window::OnFinalize()
{
	mView->OnFinalize();

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_3d_to_2d_Shutdown();
	ImGui::DestroyContext();

	mView.release();
}

/*void Window::GUI::DrawElements()
{
	static bool show_demo_window = true;
	ImGui::ShowDemoWindow(&show_demo_window);
}*/