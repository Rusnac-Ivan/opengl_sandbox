#include "Application.h"

void Application::MainLoop(void* ptr)
{
	Application* app = reinterpret_cast<Application*>(ptr);

	app->mWindow.OnRender();
}