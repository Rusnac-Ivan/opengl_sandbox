#include "View.h"
#include <GLObjects/Pipeline.h>
#include <GLObjects/Shader.h>
#include <Pieces/bishop.inl>
#include <Pieces/knight.inl>
#include "Scene.h"
#include <vector>
#include <fstream>
#include <imgui.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_glfw.h>
#define _USE_MATH_DEFINES
#include <cmath>
#ifdef __EMSCRIPTEN__
#include "webxr.h"
#endif
#include <glm/gtc/type_ptr.hpp>
#include <imgui_internal.h>
#include <Rendering/imgui_impl_3d_to_2d.h>

// https://github.com/KhronosGroup/glTF

float vertices[] = {
    -1.0f, -1.0f, 0.0f, 0.f, 0.f, 1.f, 0.f, 0.f,
    -1.0f, 1.0f, 0.0f, 0.f, 0.f, 1.f, 0.f, 1.f,
    1.0f, 1.0f, 0.0f, 0.f, 0.f, 1.f, 1.f, 1.f,
    1.0f, -1.0f, 0.0f, 0.f, 0.f, 1.f, 1.f, 0.f};
unsigned int indices[] = {
    // note that we start from 0!
    0, 1, 2, // first Triangle
    0, 2, 3  // second Triangle
};

// mKnightVBO = std::make_unique<gl::VertexBuffer>();
// mBishopVAO = std::make_unique<gl::VertexArray>();
// mKnightVAO = std::make_unique<gl::VertexArray>();

Scene::Model mRightController;
// Scene::Model mLeftController;

View::View()
{
    _viewCount = 1;
    _controllerCount = 0;

    mReadyToDraw = false;
    mMousePos = glm::vec2();
    mFPS.reserve(100);

    SphereGenerate();
    PrismGenerate();

    _controllerPos[0] = _controllerPos[1] = glm::vec3();
}
View::~View()
{
    int a = 0;
}

void View::OnCreate(int width, int height)
{
    mWidth = width;
    mHeight = height;
}
void View::OnDestroy()
{
}

void View::OnInitialize()
{
    const char *rayVertShader = GLSL(
#ifdef GL_ES
        \nprecision highp int; \n
             precision highp float; \n
#endif \n
                 layout(location = 0) in vec3 aPos;

         out vec3 FragPos;

         uniform mat4 model;
         uniform mat4 view;
         uniform mat4 projection;

         void main() {
             FragPos = vec3(model * vec4(aPos, 1.0));

             gl_Position = projection * view * vec4(FragPos, 1.0);
         });

    int rayVertShSize = strlen(rayVertShader);
    const char *rayFragShader = GLSL(
#ifdef GL_ES
        \nprecision highp int; \n
             precision highp float; \n
#endif \n
                 out vec4 FragColor;

         in vec3 FragPos;

         const vec3 objectColor = vec3(0.0, 1.0, 1.0);

         void main() {
             FragColor = vec4(objectColor, 0.4);
         });
    int rayFragShSize = strlen(rayFragShader);

    gl::Shader<gl::ShaderType::VERTEX> rayVertSh;
    gl::Shader<gl::ShaderType::FRAGMENT> rayFragSh;

    rayVertSh.LoadSources(1, &rayVertShader, &rayVertShSize);
    rayFragSh.LoadSources(1, &rayFragShader, &rayFragShSize);

    mRayProg.Attach(&rayVertSh, &rayFragSh, NULL);

    mRayProg.Link();

    mRayVBO.Data(mPrismVertices.size(), mPrismVertices.size() * sizeof(glm::vec3), mPrismVertices.data(), gl::Buffer::UsageMode::STATIC_DRAW);
    mRayVBO.AttributesPattern({gl::VertexBuffer::AttribType::POSITION});

    mRayVAO.LinkVBO(&mRayProg, &mRayVBO);
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Menu3D::Initialize();

    // mBishopVBO = std::make_unique<gl::VertexBuffer>();
    // mKnightVBO = std::make_unique<gl::VertexBuffer>();
    // mBishopVAO = std::make_unique<gl::VertexArray>();
    // mKnightVAO = std::make_unique<gl::VertexArray>();

    mProgram = std::make_unique<gl::Program>();

    gl::RenderContext::SetClearColor(0.0f, 0.3f, 0.2f, 1.00f);
    gl::Pipeline::EnableDepthTest();

    // mBishopVBO->Data(sizeof(__bishop_vert), __bishop_vert, gl::Buffer::UsageMode::STATIC_DRAW);
    // mBishopVBO->AttributesPattern({ gl::VertexBuffer::AttribType::POSITION, gl::VertexBuffer::AttribType::NORMAL });

    // mKnightVBO->Data(sizeof(__knight_vert), __knight_vert, gl::Buffer::UsageMode::STATIC_DRAW);
    // mKnightVBO->AttributesPattern({ gl::VertexBuffer::AttribType::POSITION, gl::VertexBuffer::AttribType::NORMAL });

    // mBishop->AddVBO(std::vector<gl::AttribType>({gl::AttribType::POSITION, gl::AttribType::NORMAL}), __bishop_vert_count, sizeof(__bishop_vert), __bishop_vert);
    // mKnight->AddVBO(std::vector<gl::AttribType>({gl::AttribType::POSITION, gl::AttribType::NORMAL}), __knight_vert_count, sizeof(__knight_vert), __knight_vert);

    const char *vertShader = GLSL(
#ifdef GL_ES
        \nprecision highp int; \n
             precision highp float; \n
#endif \n
                 layout(location = 0) in vec3 aPos;
         layout(location = 1) in vec3 aNorm;
         layout(location = 2) in vec2 aUV0;

         float rand(float x) {
             return fract(sin(x) * 100000.0);
         }

         out vec3 FragPos;
         out vec3 Normal;
         out vec2 UV0;

         uniform mat4 model;
         uniform mat4 view;
         uniform mat4 projection;

         void main() {
             FragPos = vec3(model * vec4(aPos, 1.0));
             Normal = mat3(transpose(inverse(model))) * aNorm;
             UV0 = aUV0;
             gl_Position = projection * view * vec4(FragPos, 1.0);
         });

    int vertShSize = strlen(vertShader);
    const char *fragShader = GLSL(
#ifdef GL_ES
        \nprecision highp int; \n
             precision highp float; \n
#endif \n
                 out vec4 FragColor;

         uniform sampler2D uBaseColor;

         in vec3 Normal;
         in vec3 FragPos;
         in vec2 UV0;

         uniform vec3 lightPos;
         uniform vec3 viewPos;
         uniform vec3 lightColor;

         const vec3 objectColor = vec3(0.3, 0.3, 0.3);

         void main() {
             // ambient
             float ambientStrength = 0.1;
             vec3 ambient = ambientStrength * lightColor;

             // diffuse
             vec3 norm = normalize(Normal);
             vec3 lightDir = normalize(lightPos - FragPos);
             float diff = max(dot(norm, lightDir), 0.0);
             vec3 diffuse = diff * lightColor;

             // specular
             float specularStrength = 0.5;
             vec3 viewDir = normalize(viewPos - FragPos);
             vec3 reflectDir = reflect(-lightDir, norm);
             float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
             vec3 specular = specularStrength * spec * lightColor;

             vec4 baseColor;

             {
                 baseColor = vec4(objectColor, 1.0);
             }

             vec3 result = (ambient + diffuse + specular) * baseColor.rgb * 2.0;
             FragColor = vec4(result, baseColor.a);
         });
    int fragShSize = strlen(fragShader);

    gl::Shader<gl::ShaderType::VERTEX> vertSh;
    gl::Shader<gl::ShaderType::FRAGMENT> fragSh;

    vertSh.LoadSources(1, &vertShader, &vertShSize);
    fragSh.LoadSources(1, &fragShader, &fragShSize);

    mProgram->Attach(&vertSh, &fragSh, NULL);

    mProgram->Link();

    // mBishopVAO->LinkVBO(mProgram.get(), mBishopVBO.get(), __bishop_vert_count);
    // mKnightVAO->LinkVBO(mProgram.get(), mKnightVBO.get(), __knight_vert_count);

    glm::mat4 model(1.f);

    mCamera.Init(glm::vec2(mWidth, mHeight), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f, 1.f, 0.f));

    mProgram->Use();
    mProgram->SetMatrix4(mProgram->Uniform("model"), model);
    mProgram->SetMatrix4(mProgram->Uniform("projection"), mCamera.GetProjectMat());
    mProgram->SetFloat3(mProgram->Uniform("lightColor"), glm::vec3(1.f, 1.f, 1.f));
    mProgram->StopUsing();

    mMenu3D.Create(500.f, 700.f);

#ifndef __EMSCRIPTEN__
    mCubeMap.SetPositiveX("D:\\Repositories\\opengl_sandbox\\resources\\cube_maps\\yokohama\\posx.jpg");
    mCubeMap.SetNegativeX("D:\\Repositories\\opengl_sandbox\\resources\\cube_maps\\yokohama\\negx.jpg");
    mCubeMap.SetPositiveY("D:\\Repositories\\opengl_sandbox\\resources\\cube_maps\\yokohama\\posy.jpg");
    mCubeMap.SetNegativeY("D:\\Repositories\\opengl_sandbox\\resources\\cube_maps\\yokohama\\negy.jpg");
    mCubeMap.SetPositiveZ("D:\\Repositories\\opengl_sandbox\\resources\\cube_maps\\yokohama\\posz.jpg");
    mCubeMap.SetNegativeZ("D:\\Repositories\\opengl_sandbox\\resources\\cube_maps\\yokohama\\negz.jpg");

    mRightController.loadFromFile("D:\\Repositories\\opengl_sandbox\\resources\\models\\controllers\\base\\generic_controller.glb");
    // mLeftController.loadFromFile("D:\\Repositories\\opengl_sandbox\\resources\\models\\controller\\left.glb");
#else
    mCubeMap.SetPositiveX("./resources/cube_maps/yokohama/posx.jpg");
    mCubeMap.SetNegativeX("./resources/cube_maps/yokohama/negx.jpg");
    mCubeMap.SetPositiveY("./resources/cube_maps/yokohama/posy.jpg");
    mCubeMap.SetNegativeY("./resources/cube_maps/yokohama/negy.jpg");
    mCubeMap.SetPositiveZ("./resources/cube_maps/yokohama/posz.jpg");
    mCubeMap.SetNegativeZ("./resources/cube_maps/yokohama/negz.jpg");

    mRightController.loadFromFile("./resources/models/controllers/base/generic_controller.glb");
#endif

    gl::CubeMap::Sampler sam;
    sam.magFilter = gl::Texture::FilterMode::LINEAR;
    sam.minFilter = gl::Texture::FilterMode::LINEAR;
    sam.wrapModeS = gl::Texture::WrapMode::CLAMP_TO_EDGE;
    sam.wrapModeT = gl::Texture::WrapMode::CLAMP_TO_EDGE;
    sam.wrapModeR = gl::Texture::WrapMode::CLAMP_TO_EDGE;

    mCubeMap.SetSampler(sam);

    mReadyToDraw = true;
}

void View::OnSceneDraw()
{
    if(!mProgram.get())
        return;

    gl::Pipeline::EnableBlending();
    gl::Pipeline::SetBlendFunc(gl::ComputOption::SRC_ALPHA, gl::ComputOption::ONE_MINUS_SRC_ALPHA);
    gl::RenderContext::SetClearColor(.3f, .5f, .8f, 1.f);

    gl::RenderContext::Clear(gl::BufferBit::COLOR, gl::BufferBit::DEPTH);

    for (int i = 0; i < _viewCount; i++)
    {
#ifndef __EMSCRIPTEN__
        gl::RenderContext::SetViewport(mWidth, mHeight);
#else
        gl::RenderContext::SetViewport(_viewports[i].x, _viewports[i].y, _viewports[i].z, _viewports[i].w);
#endif
        mProgram->Use();

#ifndef __EMSCRIPTEN__
        mViewPos = mCamera.GetPosition();
        mProgram->SetFloat3(mProgram->Uniform("lightPos"), mCamera.GetPosition());
        mProgram->SetFloat3(mProgram->Uniform("viewPos"), mCamera.GetPosition());
        mProgram->SetMatrix4(mProgram->Uniform("view"), mCamera.GetViewMat());
        mProgram->SetMatrix4(mProgram->Uniform("projection"), mCamera.GetProjectMat());

#else
        mViewPos = _headPos;
        mProgram->SetFloat3(mProgram->Uniform("lightPos"), _headPos);
        mProgram->SetFloat3(mProgram->Uniform("viewPos"), _headPos);
        mProgram->SetMatrix4(mProgram->Uniform("view"), this->_viewMatrices[i]);
        mProgram->SetMatrix4(mProgram->Uniform("projection"), this->_projectionMatrices[i]);
#endif

        glm::mat4 rightControllerModel = glm::scale(glm::mat4(1.0f), glm::vec3(1.f));

        for (int i = 0; i < _controllerCount; i++)
        {
#ifndef __EMSCRIPTEN__
            mRightController.draw(mProgram.get(), rightControllerModel);
#else
            mRightController.draw(mProgram.get(), _controllerMatrix[i] * rightControllerModel);
#endif
        }

        mProgram->StopUsing();

#ifndef __EMSCRIPTEN__
        mCubeMap.Draw(mCamera.GetPosition(), mCamera.GetViewMat(), mCamera.GetProjectMat());
        mMenu3D.RenderOut(mCamera.GetProjectMat() * mCamera.GetViewMat());
        mCamera.Update();
#else

        mCubeMap.Draw(this->_headPos, this->_viewMatrices[i], this->_projectionMatrices[i]);
        mMenu3D.RenderOut(this->_projectionMatrices[i] * this->_viewMatrices[i]);

#endif

        mRayProg.Use();
#ifndef __EMSCRIPTEN__
        mRayProg.SetMatrix4(mRayProg.Uniform("view"), mCamera.GetViewMat());
        mRayProg.SetMatrix4(mRayProg.Uniform("projection"), mCamera.GetProjectMat());
#else
        mRayProg.SetMatrix4(mRayProg.Uniform("view"), _viewMatrices[i]);
        mRayProg.SetMatrix4(mRayProg.Uniform("projection"), _projectionMatrices[i]);
#endif
        for (int i = 0; i < _controllerCount; i++)
        {
            glm::vec3 dir = _controllerDir[i];
            float scale = 1.5f;

            dir = glm::normalize(dir);

            glm::mat4 scale_mat(1.f);
            scale_mat = glm::scale(scale_mat, glm::vec3(1.f, scale, 1.f));

            glm::vec3 y_axis = glm::vec3(0.f, 1.f, 0.f);

            float angle = acos(glm::dot(y_axis, dir));

            angle = glm::radians(angle);

            glm::vec3 rot_axis = glm::cross(dir, y_axis);

            glm::mat4 rotation_mat;

            glm::vec3 v = rot_axis;
            v = glm::normalize(v);
            float x, y, z;

            x = v.x;
            y = v.y;
            z = v.z;

            angle = glm::degrees(angle);

            float cos_a = cos(angle);
            float one_minus_cos_a = 1.0f - cos_a;
            float sin_a = sin(angle);

            rotation_mat[0][0] = one_minus_cos_a * x * x + cos_a;
            rotation_mat[0][1] = one_minus_cos_a * x * y - sin_a * z;
            rotation_mat[0][2] = one_minus_cos_a * x * z + sin_a * y;
            rotation_mat[0][3] = 0.0f;
            rotation_mat[1][0] = one_minus_cos_a * y * x + sin_a * z;
            rotation_mat[1][1] = one_minus_cos_a * y * y + cos_a;
            rotation_mat[1][2] = one_minus_cos_a * y * z - sin_a * x;
            rotation_mat[1][3] = 0.0f;
            rotation_mat[2][0] = one_minus_cos_a * z * x - sin_a * y;
            rotation_mat[2][1] = one_minus_cos_a * z * y + sin_a * x;
            rotation_mat[2][2] = one_minus_cos_a * z * z + cos_a;
            rotation_mat[2][3] = 0.0f;
            rotation_mat[3][0] = 0.0f;
            rotation_mat[3][1] = 0.0f;
            rotation_mat[3][2] = 0.0f;
            rotation_mat[3][3] = 1.f;

            glm::mat4 rot_scale_mat;

            rot_scale_mat = rotation_mat * scale_mat;
            glm::mat4 translate = glm::translate(glm::mat4(1.f), _controllerPos[i]);

            glm::mat4 transform = translate * rot_scale_mat;

            mRayProg.SetMatrix4(mRayProg.Uniform("model"), transform);

            mRayVAO.Draw(gl::Primitive::TRIANGLES, 0, DataType::UNSIGNED_INT, NULL);
        }
        mRayProg.StopUsing();
    }
}

void View::OnGUIDraw()
{
    /*{
        // Start the Dear ImGui frame
	    ImGui_ImplOpenGL3_NewFrame();
        
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        float fps = (*GImGui).IO.Framerate;

        if (mFPS.size() > 100) // Max seconds to show
        {
            for (size_t i = 1; i < mFPS.size(); i++)
            {
                mFPS[i - 1] = mFPS[i];
            }
            mFPS[mFPS.size() - 1] = fps;
        }
        else
        {
            mFPS.push_back(fps);
        }
        float average_fps = 0.f;
        for (float val : mFPS)
        {
            average_fps += val / mFPS.size();
        }

        if (average_fps > 50.f)
        {
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.f, 0.7f, 0.f, 0.5f));
        }
        else if (average_fps > 20.f)
        {
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.7f, 0.7f, 0.f, 0.5f));
        }
        else
        {
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.7f, 0.f, 0.f, 0.5f));
        }

        ImGui::SetNextWindowSize(ImVec2(500.f, 200.f));
        ImGui::SetNextWindowPos(ImVec2(10.f, 10.f));
        ImGui::Begin("FPS Graph", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoNav);
        {
            char text[13] = {};
            sprintf(text, "FPS: %.3f", average_fps);
            ImGui::Text(text);

            ImGui::SetNextItemWidth(ImGui::GetWindowWidth() - 30.f);
            ImGui::PlotLines("##Frame Times", &mFPS[0], mFPS.size());

#ifndef __EMSCRIPTEN__
            _controllerDir[0] = mCamera.GetLook();
            _controllerPos[0] = mCamera.GetPosition();
#else
            // std::string controllerOrientationStr("ControlOrienta:{" + std::to_string(_controllerOrientation[0].x) + ", " + std::to_string(_controllerOrientation[0].y) + ", " + std::to_string(_controllerOrientation[0].z) + ", " + std::to_string(_controllerOrientation[0].w) + "}");
            // ImGui::Text(controllerOrientationStr.c_str());
#endif


            std::string headPosStr("HeadPos:{" + std::to_string(_headPos.x) + ", " + std::to_string(_headPos.y) + ", " + std::to_string(_headPos.z) + "}");
            ImGui::Text(headPosStr.c_str());

            std::string _headOrientationStr("HeadOrient:{" + std::to_string(_headOrientation.x) + ", " + std::to_string(_headOrientation.y) + ", " + std::to_string(_headOrientation.z) + ", " + std::to_string(_headOrientation.w) + "}");
            ImGui::Text(_headOrientationStr.c_str());

            std::string controllerPos0Str("ControllerPos[0]:{" + std::to_string(_controllerPos[0].x) + ", " + std::to_string(_controllerPos[0].y) + ", " + std::to_string(_controllerPos[0].z) + "}");
            ImGui::Text(controllerPos0Str.c_str());

            std::string controllerPos1Str("ControllerPos[1]:{" + std::to_string(_controllerPos[1].x) + ", " + std::to_string(_controllerPos[1].y) + ", " + std::to_string(_controllerPos[1].z) + "}");
            ImGui::Text(controllerPos1Str.c_str());
        }
        ImGui::End();
        ImGui::PopStyleColor();

        ImGui::EndFrame();
        // Rendering
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }*/

    {
#ifndef __EMSCRIPTEN__
        mMenu3D.RenderIn(&mCubeMap, mCamera.GetPosition(), mMousePos, glm::vec2(mWidth, mHeight), mCamera.GetViewMat(), mCamera.GetProjectMat());
#else
        mMenu3D.RenderIn(&mCubeMap, _controllerPos[0], _controllerDir[0], mMousePos, glm::vec2(mWidth, mHeight), this->_viewMatrices[0], this->_projectionMatrices[0]);
#endif
    }
}

void View::OnFinalize()
{
    // mBishopVBO.release();
    // mKnightVBO.release();
    // mBishopVAO.release();
    // mKnightVAO.release();
    mProgram.release();
    mRightController.destroy();
}

void View::OnMouseLeftDown(double x, double y)
{
    mCamera.BeginDrag((float)x, (float)y);
}

void View::OnMouseLeftUp(double x, double y)
{
    mCamera.EndDrag();
}

void View::OnMouseRightDown(double x, double y)
{
    mCamera.BeginPitch((float)x, (float)y);
}

void View::OnMouseRightUp(double x, double y)
{
    mCamera.EndPitch();
}

void View::OnMouseMiddleDown(double x, double y)
{
}

void View::OnMouseMiddleUp(double x, double y)
{
}

void View::OnMouseMove(double x, double y)
{
    mMousePos = glm::vec2(x, y);

    if (ImGui::GetIO().WantCaptureMouse)
        return;

    mCamera.Drag((float)x, (float)y);
    mCamera.Pitch((float)x, (float)y);
}

void View::OnMouseWhell(double offset)
{
    mCamera.Wheel(offset);
}

void View::OnKeyPress(int key)
{
}
void View::OnKeyRepeat(int key)
{
}
void View::OnKeyRelease(int key)
{
}

void View::OnResize(int width, int height)
{
    mWidth = width;
    mHeight = height;
    gl::RenderContext::SetViewport(width, height);
    mCamera.Resize(glm::vec2(width, height));
    /*
    mProgram->Use();
    // mProgram->SetMatrix4(mProgram->Uniform("projection"), mCamera.GetProjectMat());
#ifndef __EMSCRIPTEN__
    mProgram->SetMatrix4(mProgram->Uniform("projection"), mCamera.GetProjectMat());
#else
    mProgram->SetMatrix4(mProgram->Uniform("projection"), this->_projectionMatrices[0]);
#endif

    mProgram->StopUsing();
*/

#ifdef __EMSCRIPTEN__
    webxr_request_session(WEBXR_SESSION_MODE_IMMERSIVE_VR, WEBXR_SESSION_FEATURE_LOCAL_FLOOR, WEBXR_SESSION_FEATURE_UNBOUNDED);
#endif
}

void View::SphereGenerate()
{
    mSphereVertices.clear();

    std::vector<glm::vec3> vertices;

    float radius = 0.03f;
    int sectorCount = 20;
    int stackCount = 20;

    float x, y, z, xy;
    float nx, ny, nz, lengthInv = 1.f / radius;

    float sectorStep = 2.f * M_PI / sectorCount;
    float stackStep = M_PI / stackCount;
    float sectorAngle, stackAngle;

    for (int i = 0; i <= stackCount; ++i)
    {
        stackAngle = M_PI / 2 - i * stackStep;
        xy = radius * cosf(stackAngle);
        z = radius * sinf(stackAngle);
        for (int j = 0; j <= sectorCount; ++j)
        {
            sectorAngle = j * sectorStep;
            x = xy * cosf(sectorAngle);
            y = xy * sinf(sectorAngle);
            // nx = x * lengthInv;
            // ny = y * lengthInv;
            // nz = z * lengthInv;
            glm::vec3 vertex;
            vertex.x = x;
            vertex.y = y;
            vertex.z = z;
            vertices.push_back(vertex);
        }
    }
    for (int i = 0, k1, k2; i < stackCount; ++i)
    {
        k1 = i * (sectorCount + 1);
        k2 = k1 + sectorCount + 1;
        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
        {
            if (i != 0)
            {
                mSphereVertices.push_back(vertices[k1]);
                mSphereVertices.push_back(vertices[k2]);
                mSphereVertices.push_back(vertices[k1 + 1]);
            }
            if (i != (stackCount - 1))
            {
                mSphereVertices.push_back(vertices[k1 + 1]);
                mSphereVertices.push_back(vertices[k2]);
                mSphereVertices.push_back(vertices[k2 + 1]);
            }
        }
    }
}

void View::PrismGenerate()
{
    float a = 0.01;

    glm::vec3 P1 = glm::vec3(-a / 2.f, 0.f, -a / (2.f * sqrt(3.f)));
    glm::vec3 P2 = glm::vec3(0.f, 0.f, a / sqrt(3.f));
    glm::vec3 P3 = glm::vec3(a / 2.f, 0.f, -a / (2.f * sqrt(3.f)));

    glm::vec3 d = glm::vec3(0.f, 1.f, 0.f);

    glm::vec3 P1_, P2_, P3_;

    P1_ = P1 + d;
    P2_ = P2 + d;
    P3_ = P3 + d;

    mPrismVertices.push_back(P1);
    mPrismVertices.push_back(P2);
    mPrismVertices.push_back(P2_);

    mPrismVertices.push_back(P1);
    mPrismVertices.push_back(P2_);
    mPrismVertices.push_back(P1_);

    mPrismVertices.push_back(P2);
    mPrismVertices.push_back(P3_);
    mPrismVertices.push_back(P2_);

    mPrismVertices.push_back(P2);
    mPrismVertices.push_back(P3);
    mPrismVertices.push_back(P3_);

    mPrismVertices.push_back(P3);
    mPrismVertices.push_back(P1_);
    mPrismVertices.push_back(P3_);

    mPrismVertices.push_back(P3);
    mPrismVertices.push_back(P1);
    mPrismVertices.push_back(P1_);
}