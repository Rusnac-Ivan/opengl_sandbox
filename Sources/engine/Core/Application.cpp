#include "Application.h"

void Application::MainLoop(void* ptr)
{
	Application* app = reinterpret_cast<Application*>(ptr);
	app->mView->OnUpdate();

	app->mWindow.RenderGUI();

	app->mWindow.SwapBuffers();
	app->mWindow.PollEvents();
}