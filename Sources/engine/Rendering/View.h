#ifndef _VIEW_H_
#define _VIEW_H_

#include <GLObjects/RenderContext.h>
#include <Tools/Camera.h>
#include <Tools/Frustum.h>
#include <GLObjects/Program.h>
#include <GLObjects/VertexArray.h>
#include <GLObjects/Framebuffer.h>
#include <Rendering/Menu3D.h>
#include <memory>

class View
{
private:
    Camera mCamera;

    std::unique_ptr<gl::Program> mProgram;
    float mWidth;
    float mHeight;

    Menu3D mMenu3D;
    glm::vec2 mMousePos;

    glm::mat4 _viewMatrices[2];
    glm::mat4 _projectionMatrices[2];
    glm::mat4 _controllerTransformations[2];
    glm::vec4 _viewports[2];    
public:
    View();
    ~View();
    

    void OnInitialize();
    void OnSceneDraw();
    void OnGUIDraw();
    void OnFinalize();

    void OnMouseLeftDown(double x, double y);
    void OnMouseLeftUp(double x, double y);
    void OnMouseRightDown(double x, double y);
    void OnMouseRightUp(double x, double y);
    void OnMouseMiddleDown(double x, double y);
    void OnMouseMiddleUp(double x, double y);
    void OnMouseMove(double x, double y);
    void OnMouseWhell(double offset);

    void OnKeyPress(int key);
    void OnKeyRepeat(int key);
    void OnKeyRelease(int key);

    void OnResize(int width, int height);

    void OnCreate(int width, int height);
    void OnDestroy();
private:
    
};

#endif