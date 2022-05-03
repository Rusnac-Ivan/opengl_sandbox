#ifndef _GL_VERTEX_ARRAY_H_
#define _GL_VERTEX_ARRAY_H_

#include <Core/Platform.h>
#include <GLObjects/VertexBuffer.h>
#include <GLObjects/IndexBuffer.h>


namespace gl
{
	class Program;

	enum struct Primitive
	{
		POINTS = GL_POINTS,
		LINE_STRIP = GL_LINE_STRIP,
		LINE_LOOP = GL_LINE_LOOP,
		LINES = GL_LINES,
		TRIANGLE_STRIP = GL_TRIANGLE_STRIP,
		TRIANGLE_FAN = GL_TRIANGLE_FAN,
		TRIANGLES = GL_TRIANGLES
	};

	class VertexArray
	{
	private:
		unsigned int mID;
		IndexBuffer* mEBO;
		VertexBuffer* mVBO;
	public:
		VertexArray();
		~VertexArray();

		VertexArray(const VertexArray&) = delete;
		//VertexArray(VertexArray&& array) noexcept;
		VertexArray& operator=(const VertexArray&) = delete;
		//VertexArray& operator=(VertexArray&& array) noexcept;

		void Bind();
		void UnBind();

		void LinkVBO(Program* program, VertexBuffer* buffer);
		//void UnLinkVBO(VertexBuffer& buffer);
		void LinkEBO(IndexBuffer* buffer);

		void Draw(const Primitive& mode);
		void Draw(const Primitive& mode, const unsigned int& instanceCount);

		//void SetAttributePointer(const unsigned short &layoutLocation, const unsigned short &attribSize, const DataType &dtaType, const bool &normalize, const unsigned short &strideSize, const void *pointer);
		//void EnableAttribute(const unsigned short &layoutLocation);
		//void DisableAttribute(const unsigned short &layoutLocation);
	};
}

#endif