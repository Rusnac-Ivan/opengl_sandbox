#include "imgui_impl_3d_to_2d.h"
#include "imgui.h"
// GLFW
#include <GLFW/glfw3.h>
#ifdef _WIN32
    #undef APIENTRY
    #define GLFW_EXPOSE_NATIVE_WIN32
    #include <GLFW/glfw3native.h>   // for glfwGetWin32Window
#endif

#include <cstdio>

// GLFW data
enum GlfwClientApi
{
    GlfwClientApi_Unknown,
    GlfwClientApi_OpenGL,
    GlfwClientApi_Vulkan
};

struct ImGui_ImplGlfw_3d_to_2d_Data
{
    GLFWwindow* Window;
    GlfwClientApi           ClientApi;
    double                  Time;
    GLFWwindow* MouseWindow;
    bool                    MouseJustPressed[ImGuiMouseButton_COUNT];
    GLFWcursor* MouseCursors[ImGuiMouseCursor_COUNT];
    bool                    InstalledCallbacks;

    // Chain GLFW callbacks: our callbacks will call the user's previously installed callbacks, if any.
    GLFWwindowfocusfun      PrevUserCallbackWindowFocus;
    GLFWcursorenterfun      PrevUserCallbackCursorEnter;
    GLFWmousebuttonfun      PrevUserCallbackMousebutton;
    GLFWscrollfun           PrevUserCallbackScroll;
    GLFWkeyfun              PrevUserCallbackKey;
    GLFWcharfun             PrevUserCallbackChar;
    GLFWmonitorfun          PrevUserCallbackMonitor;

    ImGui_ImplGlfw_3d_to_2d_Data() { memset(this, 0, sizeof(*this)); }
};


// Functions
static const char* ImGui_ImplGlfw_3d_to_2d_GetClipboardText(void* user_data)
{
    return glfwGetClipboardString((GLFWwindow*)user_data);
}

static void ImGui_ImplGlfw_3d_to_2d_SetClipboardText(void* user_data, const char* text)
{
    glfwSetClipboardString((GLFWwindow*)user_data, text);
}

bool ImGui_ImplGlfw_3d_to_2d_Init(GLFWwindow* window, bool install_callbacks)
{
    ImGuiIO& io = ImGui::GetIO();
    IM_ASSERT(io.BackendPlatformUserData == NULL && "Already initialized a platform backend!");

    // Setup backend capabilities flags
    ImGui_ImplGlfw_3d_to_2d_Data* bd = IM_NEW(ImGui_ImplGlfw_3d_to_2d_Data)();
    io.BackendPlatformUserData = (void*)bd;
    io.BackendPlatformName = "imgui_impl_glfw";
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;         // We can honor GetMouseCursor() values (optional)
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;          // We can honor io.WantSetMousePos requests (optional, rarely used)

    bd->Window = window;
    bd->Time = 0.0;

    // Keyboard mapping. Dear ImGui will use those indices to peek into the io.KeysDown[] array.
    io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
    io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
    io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
    io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
    io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
    io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
    io.KeyMap[ImGuiKey_Insert] = GLFW_KEY_INSERT;
    io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
    io.KeyMap[ImGuiKey_Space] = GLFW_KEY_SPACE;
    io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
    io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
    io.KeyMap[ImGuiKey_KeyPadEnter] = GLFW_KEY_KP_ENTER;
    io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
    io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
    io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
    io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
    io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
    io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;

    io.SetClipboardTextFn = ImGui_ImplGlfw_3d_to_2d_SetClipboardText;
    io.GetClipboardTextFn = ImGui_ImplGlfw_3d_to_2d_GetClipboardText;
    io.ClipboardUserData = bd->Window;
#if defined(_WIN32)
    io.ImeWindowHandle = (void*)glfwGetWin32Window(bd->Window);
#endif

    // Create mouse cursors
    // (By design, on X11 cursors are user configurable and some cursors may be missing. When a cursor doesn't exist,
    // GLFW will emit an error which will often be printed by the app, so we temporarily disable error reporting.
    // Missing cursors will return NULL and our _UpdateMouseCursor() function will use the Arrow cursor instead.)
    GLFWerrorfun prev_error_callback = glfwSetErrorCallback(NULL);
    bd->MouseCursors[ImGuiMouseCursor_Arrow] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    bd->MouseCursors[ImGuiMouseCursor_TextInput] = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
    bd->MouseCursors[ImGuiMouseCursor_ResizeNS] = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
    bd->MouseCursors[ImGuiMouseCursor_ResizeEW] = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
    bd->MouseCursors[ImGuiMouseCursor_Hand] = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
#if GLFW_HAS_NEW_CURSORS
    bd->MouseCursors[ImGuiMouseCursor_ResizeAll] = glfwCreateStandardCursor(GLFW_RESIZE_ALL_CURSOR);
    bd->MouseCursors[ImGuiMouseCursor_ResizeNESW] = glfwCreateStandardCursor(GLFW_RESIZE_NESW_CURSOR);
    bd->MouseCursors[ImGuiMouseCursor_ResizeNWSE] = glfwCreateStandardCursor(GLFW_RESIZE_NWSE_CURSOR);
    bd->MouseCursors[ImGuiMouseCursor_NotAllowed] = glfwCreateStandardCursor(GLFW_NOT_ALLOWED_CURSOR);
#else
    bd->MouseCursors[ImGuiMouseCursor_ResizeAll] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    bd->MouseCursors[ImGuiMouseCursor_ResizeNESW] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    bd->MouseCursors[ImGuiMouseCursor_ResizeNWSE] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    bd->MouseCursors[ImGuiMouseCursor_NotAllowed] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
#endif
    glfwSetErrorCallback(prev_error_callback);

    // Chain GLFW callbacks: our callbacks will call the user's previously installed callbacks, if any.
    bd->PrevUserCallbackWindowFocus = NULL;
    bd->PrevUserCallbackMousebutton= NULL;
    bd->PrevUserCallbackScroll = NULL;
    bd->PrevUserCallbackKey = NULL;
    bd->PrevUserCallbackChar = NULL;
    bd->PrevUserCallbackMonitor = NULL;
    if (install_callbacks)
    {
        bd->InstalledCallbacks = true;
        bd->PrevUserCallbackWindowFocus = glfwSetWindowFocusCallback(window, ImGui_ImplGlfw_3d_to_2d_WindowFocusCallback);
        bd->PrevUserCallbackCursorEnter = glfwSetCursorEnterCallback(window, ImGui_ImplGlfw_3d_to_2d_CursorEnterCallback);
        bd->PrevUserCallbackMousebutton = glfwSetMouseButtonCallback(window, ImGui_ImplGlfw_3d_to_2d_MouseButtonCallback);
        bd->PrevUserCallbackScroll = glfwSetScrollCallback(window, ImGui_ImplGlfw_3d_to_2d_ScrollCallback);
        bd->PrevUserCallbackKey = glfwSetKeyCallback(window, ImGui_ImplGlfw_3d_to_2d_KeyCallback);
        bd->PrevUserCallbackChar = glfwSetCharCallback(window, ImGui_ImplGlfw_3d_to_2d_CharCallback);
        bd->PrevUserCallbackMonitor = glfwSetMonitorCallback(ImGui_ImplGlfw_3d_to_2d_MonitorCallback);
    }

    bd->ClientApi = GlfwClientApi_OpenGL;
    return true;
}


static ImGui_ImplGlfw_3d_to_2d_Data* ImGui_ImplGlfw_3d_to_2d_GetBackendData()
{
    return ImGui::GetCurrentContext() ? (ImGui_ImplGlfw_3d_to_2d_Data*)ImGui::GetIO().BackendPlatformUserData : NULL;
}

static void ImGui_ImplGlfw_3d_to_2d_UpdateMousePosAndButtons(float mouse_x, float mouse_y)
{
    ImGui_ImplGlfw_3d_to_2d_Data* bd = ImGui_ImplGlfw_3d_to_2d_GetBackendData();
    ImGuiIO& io = ImGui::GetIO();

    const ImVec2 mouse_pos_prev = io.MousePos;
    io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);

    // Update mouse buttons
    // (if a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame)
    for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++)
    {
        io.MouseDown[i] = bd->MouseJustPressed[i];
    }

#ifdef __EMSCRIPTEN__
    const bool focused = true;
#else
    const bool focused = glfwGetWindowAttrib(bd->Window, GLFW_FOCUSED) != 0;
#endif
    GLFWwindow* mouse_window = (bd->MouseWindow == bd->Window || focused) ? bd->Window : NULL;

    // Set OS mouse position from Dear ImGui if requested (rarely used, only when ImGuiConfigFlags_NavEnableSetMousePos is enabled by user)
    if (io.WantSetMousePos && focused)
        glfwSetCursorPos(bd->Window, (double)mouse_pos_prev.x, (double)mouse_pos_prev.y);

    // Set Dear ImGui mouse position from OS position
    if (mouse_window != NULL)
    {
        //double mouse_x, mouse_y;
        //glfwGetCursorPos(mouse_window, &mouse_x, &mouse_y);
        io.MousePos = ImVec2((float)mouse_x, (float)mouse_y);
    }
}

static void ImGui_ImplGlfw_3d_to_2d_UpdateMouseCursor()
{
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplGlfw_3d_to_2d_Data* bd = ImGui_ImplGlfw_3d_to_2d_GetBackendData();
    if ((io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) || glfwGetInputMode(bd->Window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
        return;

    ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
    if (imgui_cursor == ImGuiMouseCursor_None || io.MouseDrawCursor)
    {
        // Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
        glfwSetInputMode(bd->Window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    }
    else
    {
        // Show OS mouse cursor
        // FIXME-PLATFORM: Unfocused windows seems to fail changing the mouse cursor with GLFW 3.2, but 3.3 works here.
        glfwSetCursor(bd->Window, bd->MouseCursors[imgui_cursor] ? bd->MouseCursors[imgui_cursor] : bd->MouseCursors[ImGuiMouseCursor_Arrow]);
        glfwSetInputMode(bd->Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

static void ImGui_ImplGlfw_3d_to_2d_UpdateGamepads()
{
    ImGuiIO& io = ImGui::GetIO();
    memset(io.NavInputs, 0, sizeof(io.NavInputs));
    if ((io.ConfigFlags & ImGuiConfigFlags_NavEnableGamepad) == 0)
        return;

    // Update gamepad inputs
#define MAP_BUTTON(NAV_NO, BUTTON_NO)       { if (buttons_count > BUTTON_NO && buttons[BUTTON_NO] == GLFW_PRESS) io.NavInputs[NAV_NO] = 1.0f; }
#define MAP_ANALOG(NAV_NO, AXIS_NO, V0, V1) { float v = (axes_count > AXIS_NO) ? axes[AXIS_NO] : V0; v = (v - V0) / (V1 - V0); if (v > 1.0f) v = 1.0f; if (io.NavInputs[NAV_NO] < v) io.NavInputs[NAV_NO] = v; }
    int axes_count = 0, buttons_count = 0;
    const float* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axes_count);
    const unsigned char* buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &buttons_count);
    MAP_BUTTON(ImGuiNavInput_Activate, 0);     // Cross / A
    MAP_BUTTON(ImGuiNavInput_Cancel, 1);     // Circle / B
    MAP_BUTTON(ImGuiNavInput_Menu, 2);     // Square / X
    MAP_BUTTON(ImGuiNavInput_Input, 3);     // Triangle / Y
    MAP_BUTTON(ImGuiNavInput_DpadLeft, 13);    // D-Pad Left
    MAP_BUTTON(ImGuiNavInput_DpadRight, 11);    // D-Pad Right
    MAP_BUTTON(ImGuiNavInput_DpadUp, 10);    // D-Pad Up
    MAP_BUTTON(ImGuiNavInput_DpadDown, 12);    // D-Pad Down
    MAP_BUTTON(ImGuiNavInput_FocusPrev, 4);     // L1 / LB
    MAP_BUTTON(ImGuiNavInput_FocusNext, 5);     // R1 / RB
    MAP_BUTTON(ImGuiNavInput_TweakSlow, 4);     // L1 / LB
    MAP_BUTTON(ImGuiNavInput_TweakFast, 5);     // R1 / RB
    MAP_ANALOG(ImGuiNavInput_LStickLeft, 0, -0.3f, -0.9f);
    MAP_ANALOG(ImGuiNavInput_LStickRight, 0, +0.3f, +0.9f);
    MAP_ANALOG(ImGuiNavInput_LStickUp, 1, +0.3f, +0.9f);
    MAP_ANALOG(ImGuiNavInput_LStickDown, 1, -0.3f, -0.9f);
#undef MAP_BUTTON
#undef MAP_ANALOG
    if (axes_count > 0 && buttons_count > 0)
        io.BackendFlags |= ImGuiBackendFlags_HasGamepad;
    else
        io.BackendFlags &= ~ImGuiBackendFlags_HasGamepad;
}

static ImGuiMouseCursor     g_LastMouseCursor = ImGuiMouseCursor_COUNT;

void     ImGui_ImplGlfw_3d_to_2d_NewFrame(float mouse_x, float mouse_y, float width, float height)
{
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplGlfw_3d_to_2d_Data* bd = ImGui_ImplGlfw_3d_to_2d_GetBackendData();
    IM_ASSERT(bd != NULL && "Did you call ImGui_ImplGlfw_InitForXXX()?");

    // Setup display size (every frame to accommodate for window resizing)
    int w = width, h = height;
    int display_w = width, display_h = height;
    //glfwGetWindowSize(bd->Window, &w, &h);
    //glfwGetFramebufferSize(bd->Window, &display_w, &display_h);
    io.DisplaySize = ImVec2((float)w, (float)h);
    if (w > 0 && h > 0)
        io.DisplayFramebufferScale = ImVec2((float)display_w / w, (float)display_h / h);

    // Setup time step
    double current_time = glfwGetTime();
    io.DeltaTime = bd->Time > 0.0 ? (float)(current_time - bd->Time) : (float)(1.0f / 60.0f);
    bd->Time = current_time;

    // Update OS mouse position
    ImGui_ImplGlfw_3d_to_2d_UpdateMousePosAndButtons(mouse_x, mouse_y);

    ImGui_ImplGlfw_3d_to_2d_UpdateMouseCursor();

    // Update game controllers (if enabled and available)
    //ImGui_ImplGlfw_3d_to_2d_UpdateGamepads();
}

// GLFW callbacks
// - When calling Init with 'install_callbacks=true': GLFW callbacks will be installed for you. They will call user's previously installed callbacks, if any.
// - When calling Init with 'install_callbacks=false': GLFW callbacks won't be installed. You will need to call those function yourself from your own GLFW callbacks.
void     ImGui_ImplGlfw_3d_to_2d_WindowFocusCallback(GLFWwindow* window, int focused)
{
    ImGui_ImplGlfw_3d_to_2d_Data* bd = ImGui_ImplGlfw_3d_to_2d_GetBackendData();
    if (bd->PrevUserCallbackWindowFocus != NULL && window == bd->Window)
        bd->PrevUserCallbackWindowFocus(window, focused);

    ImGuiIO& io = ImGui::GetIO();
    io.AddFocusEvent(focused != 0);
}

void     ImGui_ImplGlfw_3d_to_2d_CursorEnterCallback(GLFWwindow* window, int entered)
{
    ImGui_ImplGlfw_3d_to_2d_Data* bd = ImGui_ImplGlfw_3d_to_2d_GetBackendData();
    if (bd->PrevUserCallbackCursorEnter != NULL && window == bd->Window)
        bd->PrevUserCallbackCursorEnter(window, entered);

    if (entered)
        bd->MouseWindow = window;
    if (!entered && bd->MouseWindow == window)
        bd->MouseWindow = NULL;
}

void     ImGui_ImplGlfw_3d_to_2d_MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    printf("ImGui_ImplGlfw_3d_to_2d_MouseButtonCallback\n");
    ImGui_ImplGlfw_3d_to_2d_Data* bd = ImGui_ImplGlfw_3d_to_2d_GetBackendData();
    if (bd->PrevUserCallbackMousebutton != NULL && window == bd->Window)
        bd->PrevUserCallbackMousebutton(window, button, action, mods);

    if (action == GLFW_PRESS && button >= 0 && button < IM_ARRAYSIZE(bd->MouseJustPressed))
    {
        printf("ON PRESS\n");
        bd->MouseJustPressed[button] = true;
    }
    
    if (action == GLFW_RELEASE && button >= 0 && button < IM_ARRAYSIZE(bd->MouseJustPressed))
    {
        printf("ON RELEASE\n");
        bd->MouseJustPressed[button] = false;
    }
}

void     ImGui_ImplGlfw_3d_to_2d_ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    ImGui_ImplGlfw_3d_to_2d_Data* bd = ImGui_ImplGlfw_3d_to_2d_GetBackendData();
    if (bd->PrevUserCallbackScroll != NULL && window == bd->Window)
        bd->PrevUserCallbackScroll(window, xoffset, yoffset);

    ImGuiIO& io = ImGui::GetIO();
    io.MouseWheelH += (float)xoffset;
    io.MouseWheel += (float)yoffset;
}

void     ImGui_ImplGlfw_3d_to_2d_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    ImGui_ImplGlfw_3d_to_2d_Data* bd = ImGui_ImplGlfw_3d_to_2d_GetBackendData();
    if (bd->PrevUserCallbackKey != NULL && window == bd->Window)
        bd->PrevUserCallbackKey(window, key, scancode, action, mods);

    ImGuiIO& io = ImGui::GetIO();
    if (key >= 0 && key < IM_ARRAYSIZE(io.KeysDown))
    {
        if (action == GLFW_PRESS)
            io.KeysDown[key] = true;
        if (action == GLFW_RELEASE)
            io.KeysDown[key] = false;
    }

    // Modifiers are not reliable across systems
    io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
    io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
    io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
#ifdef _WIN32
    io.KeySuper = false;
#else
    io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];
#endif
}
void     ImGui_ImplGlfw_3d_to_2d_CharCallback(GLFWwindow* window, unsigned int c)
{
    ImGui_ImplGlfw_3d_to_2d_Data* bd = ImGui_ImplGlfw_3d_to_2d_GetBackendData();
    if (bd->PrevUserCallbackChar != NULL && window == bd->Window)
        bd->PrevUserCallbackChar(window, c);

    ImGuiIO& io = ImGui::GetIO();
    io.AddInputCharacter(c);
}

void     ImGui_ImplGlfw_3d_to_2d_MonitorCallback(GLFWmonitor* monitor, int event)
{
    // Unused in 'master' branch but 'docking' branch will use this, so we declare it ahead of it so if you have to install callbacks you can install this one too.
}

void     ImGui_ImplGlfw_3d_to_2d_WindowResizeCallback(GLFWmonitor* monitor, int width, int height)
{
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)(width), (float)(height));
}

void ImGui_ImplGlfw_3d_to_2d_Shutdown()
{
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplGlfw_3d_to_2d_Data* bd = ImGui_ImplGlfw_3d_to_2d_GetBackendData();

    if (bd->InstalledCallbacks)
    {
        glfwSetWindowFocusCallback(bd->Window, bd->PrevUserCallbackWindowFocus);
        glfwSetCursorEnterCallback(bd->Window, bd->PrevUserCallbackCursorEnter);
        glfwSetMouseButtonCallback(bd->Window, bd->PrevUserCallbackMousebutton);
        glfwSetScrollCallback(bd->Window, bd->PrevUserCallbackScroll);
        glfwSetKeyCallback(bd->Window, bd->PrevUserCallbackKey);
        glfwSetCharCallback(bd->Window, bd->PrevUserCallbackChar);
        glfwSetMonitorCallback(bd->PrevUserCallbackMonitor);
    }

    for (ImGuiMouseCursor cursor_n = 0; cursor_n < ImGuiMouseCursor_COUNT; cursor_n++)
        glfwDestroyCursor(bd->MouseCursors[cursor_n]);

    io.BackendPlatformName = NULL;
    io.BackendPlatformUserData = NULL;
    IM_DELETE(bd);
}

