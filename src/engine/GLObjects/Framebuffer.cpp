#include "Framebuffer.h"

namespace gl
{
	Framebuffer::Framebuffer() : mWidth(0), mHeight(0), mProgram(nullptr), mPosAttribLoc(-1), mUVAttribLoc(-1)
	{
		GL(GenFramebuffers(1, &mID));
		assert(mID != 0 && "Failed to generate Framebuffer!");
	}

	Framebuffer::~Framebuffer()
	{
		GL(DeleteFramebuffers(1, &mID));
	}

	void Framebuffer::Bind(BindType type)
	{
		GLenum bind_type = static_cast<GLenum>(type);
		GL(BindFramebuffer(bind_type, mID));
	}

	void Framebuffer::UnBind(BindType type)
	{
		GLenum bind_type = static_cast<GLenum>(type);
		GL(BindFramebuffer(bind_type, NULL));
	}

	bool Framebuffer::CheckFramebufferStatus()
	{
		GLenum status = GL(CheckFramebufferStatus(GL_FRAMEBUFFER));
		if (status == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT)
		{
			assert("GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT");
		}
		else if (status == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT)
		{
			assert("GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT");
		}
		else if (status == GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER)
		{
			assert("GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER");
		}
		else if (status == GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER)
		{
			assert("GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER");
		}
		else if (status == GL_FRAMEBUFFER_UNSUPPORTED)
		{
			assert("GL_FRAMEBUFFER_UNSUPPORTED");
		}

		if (status == GL_FRAMEBUFFER_COMPLETE)
			return true;
		else
			return false;
	}

	void Framebuffer::Init(Program* program, uint32_t width, uint32_t height)
	{
		mProgram = program;

		if (mProgram)
		{
			//mPositionHandle = program->Attrib((ndsi8*)"a_position");
			//mTextureHandle = program->Attrib((ndsi8*)"a_texcoord0");
			mPosAttribLoc = mProgram->GetAttribLocation("a_Pos");
			mUVAttribLoc = mProgram->GetAttribLocation("a_UV");

			if (mPosAttribLoc == -1 || mUVAttribLoc == -1)
			{
				assert("Failed program for frame buffer quad.");
			}
		}

		const float fdim = 1.f;
		const int vert_count = 6;
		float quad_vert[] = {
			-fdim,  fdim, 0.f,  0.0f, 1.0f,
			-fdim, -fdim, 0.f,  0.0f, 0.0f,
			 fdim, -fdim, 0.f,  1.0f, 0.0f,

			-fdim,  fdim, 0.f,  0.0f, 1.0f,
			 fdim, -fdim, 0.f,  1.0f, 0.0f,
			 fdim,  fdim, 0.f,  1.0f, 1.0f
		};


		mQuadVBO.Data(vert_count, sizeof(quad_vert), quad_vert, gl::Buffer::UsageMode::STATIC_DRAW);
		mQuadVBO.AttributesPattern({ gl::VertexBuffer::AttribType::POSITION, gl::VertexBuffer::AttribType::UV_0 });

		mQuadVAO.LinkVBO(program, &mQuadVBO);

		mWidth = width;
		mHeight = height;
	}

	Texture2D* Framebuffer::AttachTexture(AttachType attach_type, Texture::Format internal_format, Texture::Format format, DataType type, Texture2D::Sampler sampler)
	{
		std::unique_ptr<Attachment> attachment = std::make_unique<Attachment>();
		attachment->internal_format = internal_format;
		attachment->format = format;
		attachment->data_type = type;
		attachment->type = attach_type;
		attachment->texture.SetSampler(sampler);
		attachment->texture.SetTexture2D(0, internal_format, mWidth, mHeight, 0, format, type, nullptr);


		Bind(BindType::ReadAndDraw);
		GLenum texture_type = static_cast<GLenum>(Texture::Type::TARGET_2D);
		GLenum bind_type = static_cast<GLenum>(BindType::ReadAndDraw);
		GL(FramebufferTexture2D(bind_type, (GLenum)attach_type, texture_type, attachment->texture.GetId(), 0));
		UnBind(BindType::ReadAndDraw);

		Texture2D* ptr_texture = &attachment->texture;

		mAttachments.push_back(std::move(attachment));

		return ptr_texture;
	}





	void Framebuffer::SetSizeBuffers(uint32_t width, uint32_t height)
	{
		mWidth = width;
		mHeight = height;

		for (auto& attached : mAttachments)
		{
			attached->texture.SetTexture2D(0, attached->internal_format, mWidth, mHeight, 0, attached->format, attached->data_type, nullptr);
		}
	}

	void Framebuffer::DrawQuad()
	{
		if (mProgram)
		{
			mProgram->Use();
			mQuadVAO.Draw(Primitive::TRIANGLES, 6, DataType::UNSIGNED_INT, NULL);
			mProgram->StopUsing();
		}
	}
}