#include "VertexArray.h"
#include <cassert>

namespace gl
{
	VertexArray::VertexArray() : mID(0)
	{
		GL(GenVertexArrays(1, &mID));
	}

	VertexArray::~VertexArray()
	{
		if (!mID)
		{
			GL(DeleteVertexArrays(1, &mID));
			mID = 0;
		}
	}

	void VertexArray::Bind()
	{
		GL(BindVertexArray(mID));
	}

	void VertexArray::UnBind()
	{
		GL(BindVertexArray(0));
	}

	void VertexArray::SetAttributePointer(const unsigned short &layoutLocation, const unsigned short &attribSize, const DataType &dtaType, const bool &normalize, const unsigned short &strideSize, const void *pointer)
	{
#ifndef NDEBUG
		int currentBinding;
		GL(GetIntegerv(GL_VERTEX_ARRAY_BINDING, &currentBinding));
		assert(currentBinding == mID && "Not conformity Vertex Array Object!");
#endif

		GL(VertexAttribPointer(layoutLocation, attribSize, static_cast<GLenum>(dtaType), normalize, strideSize, (const GLvoid*)pointer));
	}

	void VertexArray::EnableAttribute(const unsigned short &layoutLocation)
	{
#ifndef NDEBUG
		int currentBinding;
		GL(GetIntegerv(GL_VERTEX_ARRAY_BINDING, &currentBinding));
		assert(currentBinding == mID && "Not conformity Vertex Array Object!");
#endif

		GL(EnableVertexAttribArray(layoutLocation));
	}

	void VertexArray::DisableAttribute(const unsigned short &layoutLocation)
	{
#ifndef NDEBUG
		int currentBinding;
		GL(GetIntegerv(GL_VERTEX_ARRAY_BINDING, &currentBinding));
		assert(currentBinding == mID && "Not conformity Vertex Array Object!");
#endif

		GL(DisableVertexAttribArray(layoutLocation));
	}
}
