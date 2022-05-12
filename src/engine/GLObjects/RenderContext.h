#ifndef _GL_RENDER_CONTEXT_H_
#define _GL_RENDER_CONTEXT_H_

#include <Core/Platform.h>
#include <glm/glm.hpp>

namespace gl
{
	enum  BufferBit
	{
		COLOR = GL_COLOR_BUFFER_BIT,
		DEPTH = GL_DEPTH_BUFFER_BIT,
		STENCIL = GL_STENCIL_BUFFER_BIT
	};
	

	class RenderContext
	{
		RenderContext() = delete;
		~RenderContext() = delete;
		RenderContext(RenderContext &) = delete;

	public:
		static void Clear(const BufferBit buffBit);
		static void Clear(const BufferBit buffBit_0, const BufferBit buffBit_1);
		static void Clear(const BufferBit buffBit_0, const BufferBit buffBit_1, const BufferBit buffBit_2);

		static void SetViewport(unsigned int width, unsigned int height);
		static void SetViewport(unsigned int x, unsigned int y, unsigned int width, unsigned int height);
		static void SetClearColor(const float &red, const float &green, const float &blue, const float &alpha);
		static void SetClearColor(const glm::vec4 &color);
		static void SetClearDepth(const float &val);
		static void SetClearStencil(const int &val);

	private:
	};

	inline void RenderContext::Clear(const BufferBit buffBit)
	{
		GL(Clear(buffBit));
	}

	inline void RenderContext::Clear(const BufferBit buffBit_0, const BufferBit buffBit_1)
	{
		GL(Clear(buffBit_0 | buffBit_1));
	}
	inline void RenderContext::Clear(const BufferBit buffBit_0, const BufferBit buffBit_1, const BufferBit buffBit_2)
	{
		GL(Clear(buffBit_0 | buffBit_1 | buffBit_2));
	}

	inline void RenderContext::SetViewport(unsigned int width, unsigned int height)
	{
		GL(Viewport(0, 0, width, height));
	}

	inline void RenderContext::SetViewport(unsigned int x, unsigned int y, unsigned int width, unsigned int height)
	{
		GL(Viewport(x, y, width, height));
	}

	inline void RenderContext::SetClearColor(const float &red, const float &green, const float &blue, const float &alpha)
	{
		GL(ClearColor(red, green, blue, alpha));
	}

	inline void RenderContext::SetClearColor(const glm::vec4 &color)
	{
		GL(ClearColor(color.r, color.g, color.b, color.a));
	}

	inline void RenderContext::SetClearDepth(const float &val)
	{
		GL(ClearDepth(val));
	}

	inline void RenderContext::SetClearStencil(const int &val)
	{
		GL(ClearStencil(val));
	}
}

#endif