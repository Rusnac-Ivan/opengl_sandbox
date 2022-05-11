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
#ifdef __EMSCRIPTEN__
#include "webxr.h"
#endif
#include <glm/gtc/type_ptr.hpp>
#include <imgui_internal.h>

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

Scene::Model mRightController;
//Scene::Model mLeftController;

View::View()
{
    mMousePos = glm::vec2();
    mFrames.reserve(100);

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


#ifdef __EMSCRIPTEN__
    static void OnError(int error_code)
    {
        switch (error_code)
        {
        case WEBXR_ERR_API_UNSUPPORTED:
            printf("WebXR unsupported in this browser.\n");
            break;
        case WEBXR_ERR_GL_INCAPABLE:
            printf("GL context cannot be used to render to WebXR\n");
            break;
        case WEBXR_ERR_SESSION_UNSUPPORTED:
            printf("VR not supported on this device\n");
            break;
        default:
            printf("Unknown WebXR error with code\n");
        }
    }
#endif

void View::OnInitialize()
{
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
             Normal = aNorm;
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
         // uniform vec3 objectColor;

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

             vec4 objectColor = texture(uBaseColor, UV0);

             vec3 result = (ambient + diffuse + specular) * objectColor.rgb * 2.0;
             FragColor = vec4(result, objectColor.a);
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
    // mProgram->SetFloat3(mProgram->Uniform("objectColor"), glm::vec3(0.714f, 0.4284, 0.18144));
    mProgram->StopUsing();

    

    // mMenuVBO.Data(4, sizeof(vertices), vertices, gl::Buffer::UsageMode::STATIC_DRAW);
    // mMenuVBO.AttributesPattern({gl::VertexBuffer::AttribType::POSITION, gl::VertexBuffer::AttribType::NORMAL, gl::VertexBuffer::AttribType::UV_0 });
    // mMenuEBO.Data(sizeof(indices), indices, DataType::UNSIGNED_INT, gl::Buffer::UsageMode::STATIC_DRAW);
    // mMenuEBO.SetIndexCount(6);

    // mMenuVAO.LinkVBO(mProgram.get(), &mMenuVBO);
    // mMenuVAO.LinkEBO(&mMenuEBO);

    // mFBMenu.Init(nullptr, mMenuWidth, mMenuHeight);
    //
    // gl::Texture2D::Sampler sampler;
    // sampler.minFilter = gl::Texture::FilterMode::LINEAR;
    // sampler.magFilter = gl::Texture::FilterMode::LINEAR;
    // sampler.wrapModeS = gl::Texture::WrapMode::CLAMP_TO_EDGE;
    // sampler.wrapModeT = gl::Texture::WrapMode::CLAMP_TO_EDGE;
    // mMenuColor = mFBMenu.AttachTexture(gl::AttachType::COLOR0, gl::Texture::Format::RGBA, gl::Texture::Format::RGBA, DataType::UNSIGNED_BYTE, sampler);

    /*if (!mFBMenu.CheckFramebufferStatus())
    {
        assert("Failed frame buffer !");
    }*/

    mMenu3D.Create(500.f, 700.f);

#ifndef __EMSCRIPTEN__
    mCubeMap.SetPositiveX("D:\\Repositories\\opengl_sandbox\\resources\\cube_maps\\yokohama\\posx.jpg");
    mCubeMap.SetNegativeX("D:\\Repositories\\opengl_sandbox\\resources\\cube_maps\\yokohama\\negx.jpg");
    mCubeMap.SetPositiveY("D:\\Repositories\\opengl_sandbox\\resources\\cube_maps\\yokohama\\posy.jpg");
    mCubeMap.SetNegativeY("D:\\Repositories\\opengl_sandbox\\resources\\cube_maps\\yokohama\\negy.jpg");
    mCubeMap.SetPositiveZ("D:\\Repositories\\opengl_sandbox\\resources\\cube_maps\\yokohama\\posz.jpg");
    mCubeMap.SetNegativeZ("D:\\Repositories\\opengl_sandbox\\resources\\cube_maps\\yokohama\\negz.jpg");

    mRightController.loadFromFile("D:\\Repositories\\opengl_sandbox\\resources\\models\\controllers\\base\\controller.glb");
    //mLeftController.loadFromFile("D:\\Repositories\\opengl_sandbox\\resources\\models\\controller\\left.glb");
#else
    mCubeMap.SetPositiveX("./resources/cube_maps/yokohama/posx.jpg");
    mCubeMap.SetNegativeX("./resources/cube_maps/yokohama/negx.jpg");
    mCubeMap.SetPositiveY("./resources/cube_maps/yokohama/posy.jpg");
    mCubeMap.SetNegativeY("./resources/cube_maps/yokohama/negy.jpg");
    mCubeMap.SetPositiveZ("./resources/cube_maps/yokohama/posz.jpg");
    mCubeMap.SetNegativeZ("./resources/cube_maps/yokohama/negz.jpg");

    mRightController.loadFromFile("./resources/models/controllers/base/controller.glb");

    webxr_init(
        /* Frame callback */
        [](void *userData, int time, WebXRRigidTransform *headPose, WebXRView views[2], int viewCount)
        {
            // printf("webxr_init: Frame callback\n");

            View *thiz = (View *)userData;

            int viewIndex = 0;
            for (WebXRView view : {views[0], views[1]})
            {
                thiz->_viewports[viewIndex] = {view.viewport[0], view.viewport[1], view.viewport[2], view.viewport[3]};

                thiz->_viewMatrices[viewIndex] = glm::make_mat4(view.viewPose.matrix);

                thiz->_projectionMatrices[viewIndex] = glm::make_mat4(view.projectionMatrix);

                // printf("{x:%d, y:%d, w:%d, h:%d}\n", view.viewport[0], view.viewport[1], view.viewport[2], view.viewport[3]);

                ++viewIndex;
            }
            if (headPose)
            {
                thiz->_headPos = glm::vec3(headPose->position[0], headPose->position[1], headPose->position[2]);
                //thiz->_controlleOrientation = glm::quat(headPose->orientation[0], headPose->orientation[1], headPose->orientation[2], headPose->orientation[3]);
                //thiz->_controllerDir = glm::mat3_cast(thiz->_controlleOrientation) * glm::vec3(0.f, 0.f, -1.f);
            }

            constexpr int maxInputCount = 2;
            WebXRInputSource sources[maxInputCount];
            WebXRRigidTransform controllersPose[maxInputCount];
            int sourcesCount = 0;
            webxr_get_input_sources(sources, maxInputCount, &sourcesCount);
            
            for (int i = 0; i < sourcesCount; ++i)
            {   
                webxr_get_input_pose(sources + i, controllersPose + i);
                //printf("WebXRInputSource id: %d, WebXRHandedness: %d, WebXRTargetRayMode: %d\n", sources[i].id, sources[i].handedness, sources[i].targetRayMode);
                thiz->_controllerPos[i] = glm::vec3(controllersPose[i].position[0], controllersPose[i].position[1], controllersPose[i].position[2]);
                thiz->_controllerOrientation[i] = glm::quat(controllersPose[i].orientation[0], controllersPose[i].orientation[1], controllersPose[i].orientation[2], controllersPose[i].orientation[3]);
                thiz->_controllerDir[i] = glm::vec3(glm::mat3_cast(thiz->_controllerOrientation[i]) * glm::vec3(0.f, 0.f, -1.f));
                thiz->_controllerMatrix[i] = glm::make_mat4(controllersPose[i].matrix);
            }

            ((View *)userData)->OnSceneDraw();
            ((View *)userData)->OnGUIDraw();
        },
        /* Session start callback */
        [](void *userData, int mode)
        {
            printf("webxr_init: Session start callback\n");
        },
        /* Session end callback */
        [](void *userData, int mode)
        {
            printf("webxr_init: Session end callback\n");
        },
        /* Error callback */
        [](void *userData, int error)
        {
            printf("webxr_init: Errord callback\n");
            OnError(error);
        },
        /* userData */
        this);

    webxr_request_session(WEBXR_SESSION_MODE_IMMERSIVE_VR, WEBXR_SESSION_FEATURE_LOCAL, WEBXR_SESSION_FEATURE_LOCAL);
#endif

    gl::CubeMap::Sampler sam;
    sam.magFilter = gl::Texture::FilterMode::LINEAR;
    sam.minFilter = gl::Texture::FilterMode::LINEAR;
    sam.wrapModeS = gl::Texture::WrapMode::CLAMP_TO_EDGE;
    sam.wrapModeT = gl::Texture::WrapMode::CLAMP_TO_EDGE;
    sam.wrapModeR = gl::Texture::WrapMode::CLAMP_TO_EDGE;

    mCubeMap.SetSampler(sam);
}

void View::OnSceneDraw()
{

    gl::RenderContext::SetViewport(mWidth, mHeight);
    gl::RenderContext::SetClearColor(.3f, .5f, .8f, 1.f);
    gl::RenderContext::Clear(gl::BufferBit::COLOR, gl::BufferBit::DEPTH);

    mProgram->Use();

    
    glm::mat4 rightControllerModel = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 0.1f));

    mRightController.draw(mProgram.get(), _controllerMatrix[0] * rightControllerModel);


    // glm::mat4 model1 = glm::scale(model, glm::vec3(1.f, 5.f, 5.f));

    // mProgram->SetMatrix4(mProgram->Uniform("view"), mCamera.GetViewMat());

    gl::Pipeline::EnableBlending();
    gl::Pipeline::SetBlendFunc(gl::ComputOption::SRC_ALPHA, gl::ComputOption::ONE_MINUS_SRC_ALPHA);

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
    mProgram->SetMatrix4(mProgram->Uniform("view"), this->_viewMatrices[0]);
    mProgram->SetMatrix4(mProgram->Uniform("projection"), this->_projectionMatrices[0]);
#endif

   

    mProgram->StopUsing();

#ifndef __EMSCRIPTEN__
    mCubeMap.Draw(mCamera.GetViewMat(), mCamera.GetProjectMat());
    mMenu3D.RenderOut(mCamera.GetProjectMat() * mCamera.GetViewMat());
#else
    mCubeMap.Draw(this->_viewMatrices[0], this->_projectionMatrices[0]);
    mMenu3D.RenderOut(this->_projectionMatrices[0] * this->_viewMatrices[0]);
#endif

    

    mCamera.Update();
}

void View::OnGUIDraw()
{
    {
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        float fps = (*GImGui).IO.Framerate;
        
        if (mFrames.size() > 100) //Max seconds to show
        {
            for (size_t i = 1; i < mFrames.size(); i++)
            {
                mFrames[i - 1] = mFrames[i];
            }
            mFrames[mFrames.size() - 1] = fps;
        }
        else
        {
            mFrames.push_back(fps);
        }
        float average_fps = 0.f;
        for (float val : mFrames)
        {
            average_fps += val / mFrames.size();
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

        ImGui::SetNextWindowSize(ImVec2(360.f, 110.f));
        ImGui::SetNextWindowPos(ImVec2(10.f, 10.f));
        ImGui::Begin("FPS Graph", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoNav);
        {
            char text[13] = {};
            sprintf(text, "FPS: %.3f", average_fps);
            ImGui::Text(text);

            ImGui::SetNextItemWidth(ImGui::GetWindowWidth() - 30.f);
            ImGui::PlotLines("##Frame Times", &mFrames[0], mFrames.size());
           

#ifndef __EMSCRIPTEN__
            _controllerDir = mCamera.GetLook();
            _controllerPos = mCamera.GetPosition();
#else
            std::string controllerOrientationStr("ControlleOrientation:{" + std::to_string(_controllerOrientation[0].x) + ", " + std::to_string(_controllerOrientation[0].y) + ", " + std::to_string(_controllerOrientation[0].z) + ", " + std::to_string(_controllerOrientation[0].w) + "}");
            ImGui::Text(controllerOrientationStr.c_str());
#endif

            std::string controllerDirStr("ControllerDir:{" + std::to_string(_controllerDir[0].x) + ", " + std::to_string(_controllerDir[0].y) + ", " + std::to_string(_controllerDir[0].z) + "}");
            ImGui::Text(controllerDirStr.c_str());

            std::string controllerPosStr("ControllerPos:{" + std::to_string(_controllerPos[0].x) + ", " + std::to_string(_controllerPos[0].y) + ", " + std::to_string(_controllerPos[0].z) + "}");
            ImGui::Text(controllerPosStr.c_str());


        }
        ImGui::End();
        ImGui::PopStyleColor();

        // Rendering
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    }
    {
#ifndef __EMSCRIPTEN__
        mMenu3D.RenderIn(mCamera.GetPosition(), mMousePos, glm::vec2(mWidth, mHeight), mCamera.GetViewMat(), mCamera.GetProjectMat());
#else
        mMenu3D.RenderIn(_controllerPos[0], _controllerDir[0], mMousePos, glm::vec2(mWidth, mHeight), this->_viewMatrices[0], this->_projectionMatrices[0]);
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
    mProgram->Use();
    // mProgram->SetMatrix4(mProgram->Uniform("projection"), mCamera.GetProjectMat());
#ifndef __EMSCRIPTEN__
    mProgram->SetMatrix4(mProgram->Uniform("projection"), mCamera.GetProjectMat());
#else
    mProgram->SetMatrix4(mProgram->Uniform("projection"), this->_projectionMatrices[0]);
#endif

    mProgram->StopUsing();
}