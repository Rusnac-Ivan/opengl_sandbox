#ifndef _MENU_3D_H_
#define _MENU_3D_H_

#include <Core/Platform.h>
#include <GLObjects/VertexBuffer.h>
#include <GLObjects/IndexBuffer.h>
#include <GLObjects/VertexArray.h>
#include <GLObjects/Framebuffer.h>
#include <memory>

class Menu3D
{
	gl::VertexBuffer mVBO;
	gl::VertexArray mVAO;
	gl::IndexBuffer mEBO;

	gl::Framebuffer mFBO;
	gl::Texture2D* mColorBuff;

	float mWidth;
	float mHeight;

	static std::unique_ptr<gl::Program> mProgram;
	static float mQuadVert[];
	static glm::vec3 mVertices[4];
	static uint32_t mQuadIdx[];
	static uint32_t mObjectCount;

	gl::VertexBuffer mLineVBO;
	gl::VertexArray mLineVAO;
	glm::vec3	mPoint;
public:
	Menu3D();
	~Menu3D();

	static void Initialize();

	glm::vec3 GetIntersectPoint(){ return mPoint; }

	void Create(float width, float height);
#ifndef __EMSCRIPTEN__
	void RenderIn(glm::vec3 cam_pos, glm::vec2 mouse_pos, glm::vec2 window_size, const glm::mat4& view, const glm::mat4& proj);
#else
	void RenderIn(glm::vec3 controllerPos, glm::vec3 controllerDir, glm::vec2 mouse_pos, glm::vec2 window_size, const glm::mat4& view, const glm::mat4& proj);
#endif
	void RenderOut(const glm::mat4& view_proj);

private:
	glm::vec3 CreateRay(glm::vec2 mouse_pos, glm::vec2 window_size, const glm::mat4& view, const glm::mat4& proj);
};



#endif