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

//https://github.com/KhronosGroup/glTF


#ifdef __EMSCRIPTEN__
    #define GLSL(str) (const char *)"#version 300 es\n" #str
#define GL_ES
#else
    #define GLSL(str) (const char *)"#version 330 core\n" #str
#endif


float vertices[] = {
        -1.0f, -1.0f, 0.0f,     0.f, 0.f, 1.f,  0.f, 0.f,
        -1.0f,  1.0f, 0.0f,     0.f, 0.f, 1.f,  0.f, 1.f,
         1.0f,  1.0f, 0.0f,     0.f, 0.f, 1.f,  1.f, 1.f,
         1.0f, -1.0f, 0.0f,     0.f, 0.f, 1.f,  1.f, 0.f
};
unsigned int indices[] = {  // note that we start from 0!
    0, 1, 2,  // first Triangle
    0, 2, 3   // second Triangle
};

Scene::Model mModel;

View::View()
{
}
View::~View()
{
}

void View::OnCreate(int width, int height)
{
    mWidth = width;
    mHeight = height;

    mMenuWidth = 500;
    mMenuHeight = 500;
}
void View::OnDestroy()
{
}

void View::OnInitialize()
{
    //mBishopVBO = std::make_unique<gl::VertexBuffer>();
    //mKnightVBO = std::make_unique<gl::VertexBuffer>();
    //mBishopVAO = std::make_unique<gl::VertexArray>();
    //mKnightVAO = std::make_unique<gl::VertexArray>();

    mProgram = std::make_unique<gl::Program>();

    gl::RenderContext::SetClearColor(0.0f, 0.3f, 0.2f, 1.00f);
    gl::Pipeline::EnableDepthTest();

    //mBishopVBO->Data(sizeof(__bishop_vert), __bishop_vert, gl::Buffer::UsageMode::STATIC_DRAW);
    //mBishopVBO->AttributesPattern({ gl::VertexBuffer::AttribType::POSITION, gl::VertexBuffer::AttribType::NORMAL });

    //mKnightVBO->Data(sizeof(__knight_vert), __knight_vert, gl::Buffer::UsageMode::STATIC_DRAW);
    //mKnightVBO->AttributesPattern({ gl::VertexBuffer::AttribType::POSITION, gl::VertexBuffer::AttribType::NORMAL });

    //mBishop->AddVBO(std::vector<gl::AttribType>({gl::AttribType::POSITION, gl::AttribType::NORMAL}), __bishop_vert_count, sizeof(__bishop_vert), __bishop_vert);
    //mKnight->AddVBO(std::vector<gl::AttribType>({gl::AttribType::POSITION, gl::AttribType::NORMAL}), __knight_vert_count, sizeof(__knight_vert), __knight_vert);

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

        void main()
        {
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
        //uniform vec3 objectColor;

        

        void main()
        {
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


    //mBishopVAO->LinkVBO(mProgram.get(), mBishopVBO.get(), __bishop_vert_count);
    //mKnightVAO->LinkVBO(mProgram.get(), mKnightVBO.get(), __knight_vert_count);

    glm::mat4 model(1.f);

    mCamera.Init(glm::vec2(mWidth, mHeight), glm::vec3(0.f, 0.f, 2.f), glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f, 1.f, 0.f));

    mProgram->Use();
    mProgram->SetMatrix4(mProgram->Uniform("model"), model);
    mProgram->SetMatrix4(mProgram->Uniform("projection"), mCamera.GetProjectMat());
    mProgram->SetFloat3(mProgram->Uniform("lightColor"), glm::vec3(1.f, 1.f, 1.f));
    //mProgram->SetFloat3(mProgram->Uniform("objectColor"), glm::vec3(0.714f, 0.4284, 0.18144));
    mProgram->StopUsing();


    //mModel.loadFromFile("D:/Libraries/glTF-Sample-Models/2.0/DamagedHelmet/glTF/DamagedHelmet.gltf");

    mMenuVBO.Data(4, sizeof(vertices), vertices, gl::Buffer::UsageMode::STATIC_DRAW);
    mMenuVBO.AttributesPattern({gl::VertexBuffer::AttribType::POSITION, gl::VertexBuffer::AttribType::NORMAL, gl::VertexBuffer::AttribType::UV_0 });
    mMenuEBO.Data(sizeof(indices), indices, DataType::UNSIGNED_INT, gl::Buffer::UsageMode::STATIC_DRAW);
    mMenuEBO.SetIndexCount(6);

    mMenuVAO.LinkVBO(mProgram.get(), &mMenuVBO);
    mMenuVAO.LinkEBO(&mMenuEBO);


    mFBMenu.Init(nullptr, mMenuWidth, mMenuHeight);

    gl::Texture2D::Sampler sampler;
    sampler.minFilter = gl::Texture::FilterMode::LINEAR;
    sampler.magFilter = gl::Texture::FilterMode::LINEAR;
    sampler.wrapModeS = gl::Texture::WrapMode::CLAMP_TO_EDGE;
    sampler.wrapModeT = gl::Texture::WrapMode::CLAMP_TO_EDGE;
    mMenuColor = mFBMenu.AttachTexture(gl::AttachType::COLOR0, gl::Texture::Format::RGBA, gl::Texture::Format::RGBA, DataType::UNSIGNED_BYTE, sampler);

    if (!mFBMenu.CheckFramebufferStatus())
    {
        assert("Failed frame buffer !");
    }
}


void View::OnSceneDraw()
{
    
    gl::RenderContext::SetViewport(mWidth, mHeight);
    gl::RenderContext::SetClearColor(.3f, .5f, .8f, 1.f);
    gl::RenderContext::Clear(gl::BufferBit::COLOR, gl::BufferBit::DEPTH);

    mProgram->Use();

    mProgram->SetFloat3(mProgram->Uniform("lightPos"), mCamera.GetPosition());
    mProgram->SetFloat3(mProgram->Uniform("viewPos"), mCamera.GetPosition());

    glm::mat4 model = glm::mat4(1.0f);


    glm::mat4 model1 = glm::translate(model, glm::vec3(-0.4f, 0.f, 0.f));
    glm::mat4 model2 = glm::translate(model, glm::vec3(0.4f, 0.f, 0.f));

    //glm::mat4 model1 = glm::scale(model, glm::vec3(1.f, 5.f, 5.f));

    mProgram->SetMatrix4(mProgram->Uniform("view"), mCamera.GetViewMat());

    
    gl::Pipeline::EnableBlending();
    gl::Pipeline::SetBlendFunc(gl::ComputOption::SRC_ALPHA, gl::ComputOption::ONE_MINUS_SRC_ALPHA);

    mProgram->SetMatrix4(mProgram->Uniform("model"), model);
    mMenuColor->Bind();
    mMenuVAO.Draw(gl::Primitive::TRIANGLES);

    //mBishopVAO->Draw(gl::Primitive::TRIANGLES);

    //mModel.draw(mProgram.get());
    //mProgram->SetMatrix4(mProgram->Uniform("model"), model2);
    //mKnightVAO->Draw(gl::Primitive::TRIANGLES);

    mProgram->StopUsing();

    mCamera.Update();
}
void View::OnGUIDraw()
{
    /* {
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        

        //static bool show_demo_window = true;
        //ImGui::ShowDemoWindow(&show_demo_window);

        // Rendering
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    }*/
    {
        mFBMenu.Bind(gl::BindType::ReadAndDraw);
        //gl::RenderContext::SetViewport(0, mHeight - mMenuHeight, mMenuWidth, mHeight);
        gl::RenderContext::SetViewport(0.f, 0.f, mMenuWidth, mMenuHeight);
        gl::RenderContext::SetClearColor(0.f, 0.f, 0.f, 0.f);
        gl::RenderContext::Clear(gl::BufferBit::COLOR, gl::BufferBit::DEPTH);
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0.f, mHeight - mMenuHeight), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(mMenuWidth, mMenuHeight), ImGuiCond_Always);
        if (ImGui::Begin("My Menu rt5", nullptr))
        {
            ImGui::Button("Ok", ImVec2(90.f, 30.f));
            ImGui::Button("Save", ImVec2(90.f, 30.f));
            ImGui::Button("Cancel", ImVec2(90.f, 30.f));

            static bool animate = true;
            ImGui::Checkbox("Animate", &animate);

            static float arr[] = { 0.6f, 0.1f, 1.0f, 0.5f, 0.92f, 0.1f, 0.2f };
            ImGui::PlotLines("Frame Times", arr, IM_ARRAYSIZE(arr));

            static float values[90] = {};
            static int values_offset = 0;
            static double refresh_time = 0.0;
            if (!animate || refresh_time == 0.0)
                refresh_time = ImGui::GetTime();
            while (refresh_time < ImGui::GetTime()) // Create data at fixed 60 Hz rate for the demo
            {
                static float phase = 0.0f;
                values[values_offset] = cosf(phase);
                values_offset = (values_offset + 1) % IM_ARRAYSIZE(values);
                phase += 0.10f * values_offset;
                refresh_time += 1.0f / 60.0f;
            }

            {
                float average = 0.0f;
                for (int n = 0; n < IM_ARRAYSIZE(values); n++)
                    average += values[n];
                average /= (float)IM_ARRAYSIZE(values);
                char overlay[32];
                sprintf(overlay, "avg %f", average);
                ImGui::PlotLines("Lines", values, IM_ARRAYSIZE(values), values_offset, overlay, -1.0f, 1.0f, ImVec2(0, 80.0f));
            }
            ImGui::PlotHistogram("Histogram", arr, IM_ARRAYSIZE(arr), 0, NULL, 0.0f, 1.0f, ImVec2(0, 80.0f));

            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        mFBMenu.UnBind(gl::BindType::ReadAndDraw);
    }

    
}

void View::OnFinalize()
{
    //mBishopVBO.release();
    //mKnightVBO.release();
    //mBishopVAO.release();
    //mKnightVAO.release();
    mProgram.release();
    mModel.destroy();
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
    mProgram->SetMatrix4(mProgram->Uniform("projection"), mCamera.GetProjectMat());
    mProgram->StopUsing();
}