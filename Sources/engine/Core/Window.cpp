#include "Window.h"

static void glfw_error_callback(int error, const char* description)
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

void Window::Create(uint32_t width, uint32_t height, const char* windowName)
{
	

	glfwSetErrorCallback(glfw_error_callback);

	mWidth = width;
	mHeight = height;

	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW library!\n");
		exit(1);
	}


	std::string glsl_version;
#ifdef __EMSCRIPTEN__
	glsl_version = "#version 300 es";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#else
	glsl_version = "#version 400";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif

	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 8);

	glfwWindowHint(GLFW_RESIZABLE, 1);
	glfwWindowHint(GLFW_MAXIMIZED, 1);
	//glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

	
#ifdef __EMSCRIPTEN__
	//GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	//const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	//assert(false);
	//glfwSetWindowMonitor(mGLFWWindow, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);

	//mGLFWWindow = glfwCreateWindow(mode->width, mode->height, windowName, monitor, NULL);
	int c_width, c_height, is_fullscreen;

	emscripten_get_canvas_size(&c_width, &c_height, &is_fullscreen);

	printf("emscripten_get_canvas_size %dx%d, is_fullscreen: %d\n", c_width, c_height, is_fullscreen);

	mGLFWWindow = glfwCreateWindow(c_width, c_height, windowName, nullptr, nullptr);
	mWidth = c_width;
	mHeight = c_height;

	if (true) {
		EmscriptenFullscreenStrategy strategy;
		strategy.scaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_STDDEF;
		strategy.filteringMode = EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT;
		strategy.canvasResizedCallback = EventHandler::emscripten_window_resized_callback;
		strategy.canvasResizedCallbackUserData = mGLFWWindow;   // pointer to user data
		emscripten_enter_soft_fullscreen("canvas", &strategy);
	}

	printf("GLFWwindow*: %p\n", mGLFWWindow);
#else
	mGLFWWindow = glfwCreateWindow(mWidth, mHeight, windowName, nullptr, nullptr);
#endif

	if (mGLFWWindow == nullptr)
	{
		fprintf(stderr, "Failed to create GLFW window!\n");
		exit(1);
	}


	glfwMakeContextCurrent(mGLFWWindow);
	//glfwSwapInterval(1);

#ifndef __EMSCRIPTEN__
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		fprintf(stderr, "Failed to initialize GLAD!\n");
		exit(1);
	}
#endif

	//glEnable(GL_DEBUG_OUTPUT);
	//glDebugMessageCallback(MessageCallback, NULL);

	mGUI.Init(glsl_version.c_str());

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
	//em_canvasresized_callback_func();
	//EmscriptenFullscreenStrategy.canvasResizedCallback;
	//emscripten_set_resize_callback(nullptr, nullptr, false, emscWindowSizeChanged);
#endif

}

void Window::PollEvents()
{
	glfwPollEvents();
	//glfwGetFramebufferSize(mGLFWWindow, reinterpret_cast<int*>(&mWidth), reinterpret_cast<int*>(&mHeight));
/*#ifdef __EMSCRIPTEN__
	int c_width, c_height, is_fullscreen;

	emscripten_get_canvas_size(&c_width, &c_height, &is_fullscreen);

	bool is_canvas_size_change = false;
	if (mWidth != c_width)
	{
		mWidth = c_width;
		is_canvas_size_change = true;
	}
	if (mHeight != c_height)
	{
		mHeight = c_height;
		is_canvas_size_change = true;
	}

	if (is_canvas_size_change)
	{
		EventHandler::WindowSizeCallback(mGLFWWindow, mWidth, mHeight);
	}
#endif*/
}

Window::~Window()
{
	if (mGLFWWindow != nullptr)
	{
		glfwDestroyWindow(mGLFWWindow);
		glfwTerminate();
	}
}

void Window::GUI::Init(const char* glsl_version)
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	io.WantCaptureMouse = false;
	io.WantCaptureKeyboard = false;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(mWindow->mGLFWWindow, true);
	ImGui_ImplOpenGL3_Init(glsl_version);
}

void Window::GUI::Render()
{
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	DrawElements();

	// Rendering
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Window::GUI::DrawElements()
{
	static bool show_demo_window = true;
	ImGui::ShowDemoWindow(&show_demo_window);
}