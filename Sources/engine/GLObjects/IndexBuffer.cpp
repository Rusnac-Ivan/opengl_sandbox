#include "IndexBuffer.h"

namespace gl
{
	IndexBuffer::IndexBuffer()
	{
	}

	IndexBuffer::~IndexBuffer()
	{
	}

	void IndexBuffer::Bind()
	{
		Buffer::Bind(Buffer::Type::ELEMENT_ARRAY);
	}

	void IndexBuffer::Data(const unsigned int& size, const void* data, DataType type, UsageMode accessFreq)
	{
		mDataType = type;
		this->Bind();
		Buffer::Data(size, data, accessFreq);
		this->UnBind();
	}
	void IndexBuffer::SubData(const unsigned short& offset, const unsigned short& size, const void* data)
	{
		this->Bind();
		Buffer::SubData(offset, size, data);
		this->UnBind();
	}
}