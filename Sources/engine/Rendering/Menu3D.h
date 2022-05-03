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
	static uint32_t mQuadIdx[];
	static uint32_t mObjectCount;
public:
	Menu3D();
	~Menu3D();

	static void Initialize();

	void Create(float width, float height);

	void RenderIn(float window_width, float window_height);
	void RenderOut(const glm::mat4& view_proj);

private:

};



#endif