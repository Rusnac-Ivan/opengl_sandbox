#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include "EventHandler.h"
#include "Window.h"
#include <Core/Platform.h>

class View;

class Application
{
private:
	Application(uint32_t width, uint32_t height, const char *windowName)
	{
		mWindow.Create(width, height, windowName);

		EventHandler::SetListener(mWindow.GetView());

		mWindow.OnInitialize();
#ifdef __EMSCRIPTEN__
		emscripten_set_main_loop_arg(Application::MainLoop, (void *)this, 0, 1);
#else
		while (mWindow.WindowIsOpen())
		{
			MainLoop(this);
		}
#endif
		mWindow.OnFinalize();
	}
	Application(Application &app) {}
	Application &operator=(Application &app) { return *this; }
	~Application()
	{
	}

	static void MainLoop(void *ptr);

public:
	static Application &RunApplication(uint32_t width, uint32_t height, const char *windowName)
	{
		static Application app(width, height, windowName);
		return app;
	}

private:
	// View* mView;
	Window mWindow;
};

#endif