#ifndef _FRAME_BUFFER_H_
#define _FRAME_BUFFER_H_

#include <Core/Platform.h>
#include <cassert>
#include <GLObjects/Texture.h>
#include <GLObjects/Program.h>
#include <GLObjects/VertexBuffer.h>
#include <GLObjects/VertexArray.h>
#include <vector>
#include <memory>
#include <map>

namespace gl
{
	enum struct BindType
	{
		ReadAndDraw = GL_FRAMEBUFFER,
		Draw = GL_DRAW_FRAMEBUFFER,
		Read = GL_READ_FRAMEBUFFER,
	};

	enum struct AttachType
	{
		COLOR0 = GL_COLOR_ATTACHMENT0,
		COLOR1 = GL_COLOR_ATTACHMENT1,
		COLOR2 = GL_COLOR_ATTACHMENT2,
		COLOR3 = GL_COLOR_ATTACHMENT3,
		COLOR4 = GL_COLOR_ATTACHMENT4,
		COLOR5 = GL_COLOR_ATTACHMENT5,
		COLOR6 = GL_COLOR_ATTACHMENT6,
		COLOR7 = GL_COLOR_ATTACHMENT7,
		COLOR8 = GL_COLOR_ATTACHMENT8,
		COLOR9 = GL_COLOR_ATTACHMENT9,
		COLOR10 = GL_COLOR_ATTACHMENT10,
		COLOR11 = GL_COLOR_ATTACHMENT11,
		COLOR12 = GL_COLOR_ATTACHMENT12,
		COLOR13 = GL_COLOR_ATTACHMENT13,
		COLOR14 = GL_COLOR_ATTACHMENT14,
		COLOR15 = GL_COLOR_ATTACHMENT15,
		COLOR16 = GL_COLOR_ATTACHMENT16,

		DEPTH = GL_DEPTH_ATTACHMENT,
		STENCIL = GL_STENCIL_ATTACHMENT,
		DEPTH_STENCIL = GL_DEPTH_STENCIL_ATTACHMENT
	};


	struct Attachment
	{
		Texture2D texture;
		AttachType type;
		Texture::Format internal_format;
		Texture::Format format;
		DataType data_type;
	};

	class Framebuffer
	{

		unsigned int mID;

		gl::VertexBuffer mQuadVBO;
		gl::VertexArray mQuadVAO;

		Program* mProgram;

		uint32_t mWidth;
		uint32_t mHeight;

		int mPosAttribLoc;
		int mUVAttribLoc;

		std::vector<std::unique_ptr<Attachment>> mAttachments;
	public:
		Framebuffer();
		~Framebuffer();

		void Bind(BindType type);
		void UnBind(BindType type);

		void Init(Program* program, uint32_t width, uint32_t height);

		bool CheckFramebufferStatus();

		Texture2D* AttachTexture(AttachType attach_type, Texture::Format internal_format, Texture::Format format, DataType type, Texture2D::Sampler sampler);
		void SetSizeBuffers(uint32_t width, uint32_t height);
		void DrawQuad();

		
	};
}


#endif