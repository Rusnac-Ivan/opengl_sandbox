#include <iostream>
#include <chrono>
#include <Core/Application.h>

int main()
{
    Application &app = Application::RunApplication(1280, 720, "Opengl Window");
    return 0;
}
