#include "Menu3D.h"
#include <GLObjects/Shader.h>
#include <backends/imgui_impl_opengl3.h>
#include <GLObjects/RenderContext.h>
#include "imgui_impl_3d_to_2d.h"
#include <glm/ext/matrix_transform.hpp>
//#include <backends/imgui_impl_glfw.h>


uint32_t Menu3D::mObjectCount = 0;
std::unique_ptr<gl::Program> Menu3D::mProgram;


Menu3D::Menu3D()
{
    if (mObjectCount == 0)
        Initialize();
    mObjectCount++;
}

Menu3D::~Menu3D()
{
    mObjectCount--;
    if (mObjectCount == 0)
        mProgram.release();
}


float Menu3D::mQuadVert[] =
{
    -1.0f, -1.0f, 0.0f,    0.f, 0.f,
    -1.0f,  1.0f, 0.0f,    0.f, 1.f,
     1.0f,  1.0f, 0.0f,    1.f, 1.f,
     1.0f, -1.0f, 0.0f,    1.f, 0.f
};
uint32_t Menu3D::mQuadIdx[] =
{
    0, 1, 2,  // first Triangle
    0, 2, 3   // second Triangle
};

void Menu3D::Initialize()
{
    mProgram = std::make_unique<gl::Program>();

    const char* vertShader = GLSL(
#ifdef GL_ES
        \nprecision highp int; \n
        precision highp float; \n
#endif \n
        layout(location = 0) in vec3 aPos;
        layout(location = 2) in vec2 aUV0;

        float rand(float x) {
            return fract(sin(x) * 100000.0);
        }

        out vec3 FragPos;
        out vec2 UV0;

        uniform mat4 model;
        uniform mat4 proj_view;

        void main()
        {
            FragPos = vec3(model * vec4(aPos, 1.0));
            UV0 = aUV0;
            gl_Position = proj_view * vec4(FragPos, 1.0);
        }
    );

    int vertShSize = strlen(vertShader);
    const char* fragShader = GLSL(
#ifdef GL_ES
        \nprecision highp int; \n
        precision highp float; \n
#endif \n
        out vec4 FragColor;

        uniform sampler2D uBaseColor;

        in vec3 FragPos;
        in vec2 UV0;

        //uniform vec3 objectColor;

        void main()
        {
            vec4 base_col = texture(uBaseColor, UV0);
            FragColor = vec4(base_col.rgb, base_col.a);
        }
    );
    int fragShSize = strlen(fragShader);

    gl::Shader<gl::ShaderType::VERTEX> vertSh;
    gl::Shader<gl::ShaderType::FRAGMENT> fragSh;

    vertSh.LoadSources(1, &vertShader, &vertShSize);
    fragSh.LoadSources(1, &fragShader, &fragShSize);

    mProgram->Attach(&vertSh, &fragSh, NULL);

    mProgram->Link();
}

void Menu3D::Create(float width, float height)
{
    mWidth = width;
    mHeight = height;

    float quad_width = 1.f;
    float quad_height = quad_width * (mHeight / mWidth);
    float quadW_half = quad_width / 2.f;
    float quadH_half = quad_height / 2.f;

    float QuadVert[] =
    {
        -quadW_half, -quadH_half, 0.0f,    0.f, 0.f,
        -quadW_half,  quadH_half, 0.0f,    0.f, 1.f,
         quadW_half,  quadH_half, 0.0f,    1.f, 1.f,
         quadW_half, -quadH_half, 0.0f,    1.f, 0.f
    };
    uint32_t QuadIdx[] =
    {
        0, 1, 2,  // first Triangle
        0, 2, 3   // second Triangle
    };


    mVBO.Data(4, sizeof(QuadVert), QuadVert, gl::Buffer::UsageMode::STATIC_DRAW);
    mVBO.AttributesPattern({ gl::VertexBuffer::AttribType::POSITION, gl::VertexBuffer::AttribType::UV_0 });
    mEBO.Data(sizeof(QuadIdx), QuadIdx, DataType::UNSIGNED_INT, gl::Buffer::UsageMode::STATIC_DRAW);
    mEBO.SetIndexCount(6);

    mVAO.LinkVBO(mProgram.get(), &mVBO);
    mVAO.LinkEBO(&mEBO);

    mFBO.Init(nullptr, mWidth, mHeight);

    gl::Texture2D::Sampler sampler;
    sampler.minFilter = gl::Texture::FilterMode::LINEAR;
    sampler.magFilter = gl::Texture::FilterMode::LINEAR;
    sampler.wrapModeS = gl::Texture::WrapMode::CLAMP_TO_EDGE;
    sampler.wrapModeT = gl::Texture::WrapMode::CLAMP_TO_EDGE;
    mColorBuff = mFBO.AttachTexture(gl::AttachType::COLOR0, gl::Texture::Format::RGBA, gl::Texture::Format::RGBA, DataType::UNSIGNED_BYTE, sampler);

    if (!mFBO.CheckFramebufferStatus())
    {
        assert("Failed frame buffer !");
    }
}

void Menu3D::RenderIn(float window_width, float window_height)
{
    mFBO.Bind(gl::BindType::ReadAndDraw);
    //gl::RenderContext::SetViewport(0, mHeight - mMenuHeight, mMenuWidth, mHeight);
    //gl::RenderContext::SetViewport(0.f, 0.f, mWidth, mHeight);
    
    gl::RenderContext::SetClearColor(0.f, 0.f, 0.f, 0.f);
    gl::RenderContext::Clear(gl::BufferBit::COLOR, gl::BufferBit::DEPTH);
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    //ImGui_ImplGlfw_NewFrame();
    ImGui_ImplGlfw_3d_to_2d_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(0.f, window_height - mHeight), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(mWidth,  mHeight), ImGuiCond_Always);
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
    mFBO.UnBind(gl::BindType::ReadAndDraw);
}

void Menu3D::RenderOut(const glm::mat4& proj_view)
{

    glm::mat4 model = glm::mat4(1.0f);
    //static float angle = 0.f;
    //model = glm::rotate(model, angle, glm::vec3(0.f, 0.f, 1.f));
    //angle += 0.01f;
    mProgram->Use();
    mProgram->SetMatrix4(mProgram->Uniform("proj_view"), proj_view);
    mProgram->SetMatrix4(mProgram->Uniform("model"),  model);
    mColorBuff->Bind();
    mVAO.Draw(gl::Primitive::TRIANGLES);
    mProgram->StopUsing();
}