#include "VertexArray.h"
#include <GLObjects/Program.h>
#include <cassert>

namespace gl
{
	VertexArray::VertexArray() : mID(0), mEBO(nullptr)
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

	/*VertexArray::VertexArray(VertexArray&& array) noexcept
	{

	}

	VertexArray& VertexArray::operator=(VertexArray&& array) noexcept
	{

	}*/

	void VertexArray::Bind()
	{
		GL(BindVertexArray(mID));
	}

	void VertexArray::UnBind()
	{
		GL(BindVertexArray(0));
	}


	void VertexArray::LinkVBO(Program* program, VertexBuffer* buffer, uint32_t vertexCount)
	{
		mVertexCount = vertexCount;
		this->Bind();
		buffer->Bind();
		unsigned short shift = 0;
		for (VertexBuffer::Attribute& attrib : buffer->GetAttributes())
		{
			if(program != nullptr)
				attrib.location = program->GetAttribLocation(attrib.name.c_str());
			GL(VertexAttribPointer(attrib.location, attrib.numOfComponents, (GLenum)attrib.dataType, GL_FALSE, buffer->GetStrideSize(), (const void*)(attrib.offset)));
			GL(EnableVertexAttribArray(attrib.location));
		}
		buffer->UnBind();
		this->UnBind();
	}

	/*void VertexArray::UnLinkVBO(VertexBuffer& buffer)
	{
		this->Bind();
		buffer.Bind();
		for (VertexBuffer::Attribute& attrib : buffer.GetAttributes())
		{
			GL(DisableVertexAttribArray(attrib.location));
		}
		buffer.UnBind();
		this->UnBind();
	}*/

	void VertexArray::LinkEBO(IndexBuffer* buffer)
	{
		mEBO = buffer;
		this->Bind();
		mEBO->Bind();
		this->UnBind();
	}

	void VertexArray::Draw(const Primitive& mode)
	{
		this->Bind();
		if (!mEBO)
		{
			GL(DrawArrays(static_cast<GLenum>(mode), 0, mVertexCount));
		}
		else
		{
			GL(DrawElements(static_cast<GLenum>(mode), mVertexCount, GL_UNSIGNED_BYTE, nullptr));
		}
		this->UnBind();
	}

	void VertexArray::Draw(const Primitive& mode, const unsigned int& instanceCount)
	{
		this->Bind();
		if (!mEBO)
		{
			GL(DrawArraysInstanced(static_cast<GLenum>(mode), 0, mVertexCount, instanceCount));
		}
		else
		{
			GL(DrawElementsInstanced(static_cast<GLenum>(mode), mVertexCount, GL_UNSIGNED_BYTE, nullptr, instanceCount));
		}
		this->UnBind();
	}

	/*void VertexArray::SetAttributePointer(const unsigned short& layoutLocation, const unsigned short& attribSize, const DataType& dtaType, const bool& normalize, const unsigned short& strideSize, const void* pointer)
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
	}*/
}
