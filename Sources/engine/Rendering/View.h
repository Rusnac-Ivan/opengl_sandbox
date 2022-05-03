#ifndef _VIEW_H_
#define _VIEW_H_

#include <GLObjects/RenderContext.h>
#include <Tools/Camera.h>
#include <Tools/Frustum.h>
#include <GLObjects/Program.h>
#include <GLObjects/VertexArray.h>
#include <GLObjects/Framebuffer.h>
#include <memory>

class View
{
private:
    Camera mCamera;
    //Perspective mPerspective;
    //std::unique_ptr<gl::VertexBuffer> mBishopVBO;
    //std::unique_ptr<gl::VertexBuffer> mKnightVBO;
    //std::unique_ptr<gl::VertexArray> mBishopVAO;
    //std::unique_ptr<gl::VertexArray> mKnightVAO;

    gl::VertexBuffer mMenuVBO;
    gl::VertexArray mMenuVAO;
    gl::IndexBuffer mMenuEBO;

    std::unique_ptr<gl::Program> mProgram;
    float mWidth;
    float mHeight;

    gl::Framebuffer mFBMenu;
    gl::Texture2D* mMenuColor;
    uint32_t mMenuWidth;
    uint32_t mMenuHeight;

    //RubiksCube mRubiksCube;

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