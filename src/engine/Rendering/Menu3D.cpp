#include "Menu3D.h"
#include <GLObjects/Shader.h>
#include <backends/imgui_impl_opengl3.h>
#include <GLObjects/RenderContext.h>
#include "imgui_impl_3d_to_2d.h"
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/ext/matrix_projection.hpp>
#include <GLObjects/CubeMap.h>

// In a header somewhere.

uint32_t Menu3D::mObjectCount = 0;
std::unique_ptr<gl::Program> Menu3D::mProgram;
glm::vec3 Menu3D::mVertices[4];

Menu3D::Menu3D()
{
    if (mObjectCount == 0)
        Initialize();
    mObjectCount++;

    mPoint = glm::vec3();
}

Menu3D::~Menu3D()
{
    mObjectCount--;
    if (mObjectCount == 0)
        mProgram.release();
}

float Menu3D::mQuadVert[] =
    {
        -1.0f, -1.0f, 0.0f, 0.f, 0.f,
        -1.0f, 1.0f, 0.0f, 0.f, 1.f,
        1.0f, 1.0f, 0.0f, 1.f, 1.f,
        1.0f, -1.0f, 0.0f, 1.f, 0.f};
uint32_t Menu3D::mQuadIdx[] =
    {
        0, 1, 2, // first Triangle
        0, 2, 3  // second Triangle
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
    out vec3 Pos;

    uniform vec3 pos;
    uniform mat4 model;
    uniform mat4 proj_view;

    void main() {
        FragPos = vec3(model * vec4(aPos, 1.0));
        Pos = vec3(model * vec4(pos, 1.0));
        UV0 = aUV0;
        gl_Position = proj_view * vec4(FragPos, 1.0);
    });

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
    in vec3 Pos;

    void main() {

        if (dot(Pos - FragPos, Pos - FragPos) < 0.0001)
            FragColor = vec4(1.0, 1.0, 1.0, 1.0);
        else
        {
            vec4 base_col = texture(uBaseColor, UV0);
            FragColor = vec4(base_col.rgb, base_col.a);
        }

    });
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

    float quad_width = 1.5f;
    float quad_height = quad_width * (mHeight / mWidth);
    float quadW_half = quad_width / 2.f;
    float quadH_half = quad_height / 2.f;

    float QuadVert[] =
        {
            -quadW_half, -quadH_half, -1.5f, 0.f, 0.f,
            -quadW_half, quadH_half, -1.5f, 0.f, 1.f,
            quadW_half, quadH_half, -1.5f, 1.f, 1.f,
            quadW_half, -quadH_half, -1.5f, 1.f, 0.f};

    mVertices[0] = glm::vec3(QuadVert[0], QuadVert[1], QuadVert[2]);
    mVertices[1] = glm::vec3(QuadVert[5], QuadVert[6], QuadVert[7]);
    mVertices[2] = glm::vec3(QuadVert[10], QuadVert[11], QuadVert[12]);
    mVertices[3] = glm::vec3(QuadVert[15], QuadVert[16], QuadVert[17]);

    uint32_t QuadIdx[] =
        {
            0, 1, 2, // first Triangle
            0, 2, 3  // second Triangle
        };

    mVBO.Data(4, sizeof(QuadVert), QuadVert, gl::Buffer::UsageMode::STATIC_DRAW);
    mVBO.AttributesPattern({gl::VertexBuffer::AttribType::POSITION, gl::VertexBuffer::AttribType::UV_0});
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

glm::vec3 Menu3D::CreateRay(glm::vec2 mouse_pos, glm::vec2 window_size, const glm::mat4 &view, const glm::mat4 &proj)
{
    return glm::unProject(glm::vec3(mouse_pos.x, window_size.y - mouse_pos.y, 0.f), view, proj, glm::vec4(0.f, 0.f, window_size.x, window_size.y));

    /*float mouseX = mouse_pos.x / (window_size.x * 0.5f) - 1.0f;
    float mouseY = mouse_pos.y / (window_size.y * 0.5f) - 1.0f;

    glm::mat4 invVP = glm::inverse(proj_view);
    glm::vec4 screenPos = glm::vec4(mouseX, -mouseY, 1.0f, 1.0f);

    glm::vec4 worldPos = invVP * screenPos;

    glm::vec3 dir = glm::normalize(glm::vec3(worldPos));

    return dir;*/
}

#ifndef __EMSCRIPTEN__
    void Menu3D::RenderIn(gl::CubeMap* cubemap, glm::vec3 cam_pos, glm::vec2 mouse_pos, glm::vec2 window_size, const glm::mat4 &view, const glm::mat4 &proj)
#else
    void Menu3D::RenderIn(gl::CubeMap* cubemap, glm::vec3 controllerPos, glm::vec3 controllerDir, glm::vec2 mouse_pos, glm::vec2 window_size, const glm::mat4& view, const glm::mat4& proj)
#endif
{

    mFBO.Bind(gl::BindType::ReadAndDraw);


    gl::RenderContext::SetClearColor(0.f, 0.f, 0.f, 0.f);
    gl::RenderContext::Clear(gl::BufferBit::COLOR);


#ifndef __EMSCRIPTEN__
    glm::vec3 ray = glm::normalize(CreateRay(mouse_pos, window_size, view, proj) - cam_pos);
    glm::vec3 controllerPos = cam_pos;
#else
    glm::vec3 ray = glm::normalize(controllerDir);
#endif
    float distance = 0.f;
    

    glm::vec2 new_mouse_pos = glm::vec2(window_size.x, 0.f);
    if (glm::intersectRayPlane(controllerPos, ray, mVertices[0], glm::vec3(0.f, 0.f, 1.f), distance))
    {
        int a = 0;
        glm::vec3 P1 = controllerPos + ray * distance;

        mPoint = P1;

        glm::vec3 Vx = glm::normalize(mVertices[3] - mVertices[0]);
        glm::vec3 Vy = glm::normalize(mVertices[1] - mVertices[0]);

        glm::vec3 P1_V0 = P1 - mVertices[0];

        float x = glm::dot(Vx, P1_V0);
        // glm::vec3 X = x * Vx;

        float y = glm::dot(Vy, P1_V0);
        // glm::vec3 Y = y * Vy;

        new_mouse_pos = glm::vec2((mWidth / glm::length(mVertices[3] - mVertices[0])) * x, mHeight - (mHeight / glm::length(mVertices[1] - mVertices[0])) * y);
    }
    std::string info(std::to_string(new_mouse_pos.x) + ", " + std::to_string(new_mouse_pos.y));

    ImGui_ImplGlfw_3d_to_2d_NewFrame(new_mouse_pos.x, new_mouse_pos.y, mWidth, mHeight);
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(0.f, 0.f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(mWidth, mHeight), ImGuiCond_Always);
    if (ImGui::Begin("My Menu Rel: 30/05/2022, v0.4", nullptr))
    {
        if(ImGui::Button("Ok", ImVec2(90.f, 30.f)))
        {
            printf("Button Ok\n");
        }
        if(ImGui::Button("Save", ImVec2(90.f, 30.f)))
        {
            printf("Button Save\n");
        }
        if(ImGui::Button("Cancel", ImVec2(90.f, 30.f)))
        {
            printf("Button Cancel\n");
        }
        float speed = cubemap->GetSpeed();
        if (ImGui::SliderFloat("Clouds Speed", &speed, 0.f, 1.f, "%.3f"))
            cubemap->SetSpeed(speed);
        float amount = cubemap->GetAmount();
        if (ImGui::SliderFloat("Clouds Amount", &amount, 0.f, 1.f, "%.3f"))
            cubemap->SetAmount(amount);

        float vert_angle = cubemap->GetVert();
        if (ImGui::SliderFloat("Sun pos vertical", &vert_angle, -90.f, 90.f, "%.3f"))
            cubemap->SetVert(vert_angle);
        float hor_angle = cubemap->GetHoriz();
        if (ImGui::SliderFloat("Sun pos horizontal", &hor_angle, 0.f, 360.f, "%.3f"))
            cubemap->SetHoriz(hor_angle);

        ImDrawList *draw_list = ImGui::GetWindowDrawList();

        draw_list->AddCircleFilled(ImVec2(new_mouse_pos.x, new_mouse_pos.y), 10.f, ImColor(ImVec4(0.f, 1.f, 1.f, 0.5f)));

        static char buff[64] = {};
        ImGui::InputText("in text", buff, IM_ARRAYSIZE(buff));

        static bool animate = true;
        ImGui::Checkbox("Animate", &animate);

        static float arr[] = {0.6f, 0.1f, 1.0f, 0.5f, 0.92f, 0.1f, 0.2f};
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
    }
    ImGui::End();

    ImGui::EndFrame();

    // Rendering
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    mFBO.UnBind(gl::BindType::ReadAndDraw);
}

void Menu3D::RenderOut(const glm::mat4 &proj_view)
{

    glm::mat4 model = glm::mat4(1.0f);
    // static float angle = 0.f;
    // model = glm::rotate(model, angle, glm::vec3(0.f, 0.f, 1.f));
    // angle += 0.01f;
    mProgram->Use();
    mProgram->SetFloat3(mProgram->Uniform("pos"), mPoint);
    mProgram->SetMatrix4(mProgram->Uniform("proj_view"), proj_view);
    mProgram->SetMatrix4(mProgram->Uniform("model"), model);
    mColorBuff->Bind();
    mVAO.Draw(gl::Primitive::TRIANGLES, 6, DataType::UNSIGNED_INT, NULL);
    mProgram->StopUsing();
}