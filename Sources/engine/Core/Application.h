#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include "EventHandler.h"
#include "Window.h"
#include <Rendering/View.h>
#include <Core/Platform.h>

class Application
{
private:
	Application(uint32_t width, uint32_t height, const char *windowName)
	{
		mWindow.Create(width, height, windowName);



		//mView = new View;
		static View view;
		mView = &view;
		mView->OnCreate(width, height);
		EventHandler::SetListener(mView);
#ifdef __EMSCRIPTEN__
		mView->OnInitialize();
		emscripten_set_main_loop_arg(Application::MainLoop, (void*)this, 0, 1);
		mView->OnFinalize();
#else
		mView->OnInitialize();
		while (mWindow.WindowIsOpen())
		{
			MainLoop(this);
		}
		mView->OnFinalize();
#endif
	}
	Application(Application &app) {}
	Application &operator=(Application &app) { return *this; }
	~Application()
	{
		mView->OnDestroy();
		//delete mView;
	}

	static void MainLoop(void* ptr);

public:
	static Application &RunApplication(uint32_t width, uint32_t height, const char *windowName)
	{
		static Application app(width, height, windowName);
		return app;
	}

private:
	
	View* mView;
	Window mWindow;
};

#endif