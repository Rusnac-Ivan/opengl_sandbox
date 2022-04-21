#include "Buffer.h"
#include <cassert>

namespace gl
{

	Buffer::Buffer() : mID(0), mCurrentTarget(Target::UNKNOWN), mAccessFreq(Access::UNKNOWN)
	{
		GL(GenBuffers(1, &mID));
		assert(mID != 0 && "Failed to generate buffer!");
	}

	Buffer::~Buffer()
	{
		if (!mID)
		{
			GL(DeleteBuffers(1, &mID));
			mID = 0;
		}
	}

	void Buffer::Bind(Target target)
	{
		mCurrentTarget = target;
		//assert(mBindingTarget != BindingTarget::UNDEFINED && "Use undefined binding target!");
		GL(BindBuffer(static_cast<GLenum>(mCurrentTarget), mID));
	}

	void Buffer::UnBind()
	{
		//assert(mBindingTarget != BindingTarget::UNDEFINED && "Use undefined binding target!");
		GL(BindBuffer(static_cast<GLenum>(mCurrentTarget), 0));
		mCurrentTarget = Target::UNKNOWN;
	}

	void Buffer::Data(const unsigned int& size, const void* data, Access accessFreq)
	{
		mAccessFreq = accessFreq;
#ifndef NDEBUG
		int currentBinding = 0;
		switch (mCurrentTarget)
		{
			case Target::ARRAY:
			{
				GL(GetIntegerv(GL_ARRAY_BUFFER_BINDING, &currentBinding));
			}
			break;
			case Target::ELEMENT_ARRAY:
			{
				GL(GetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &currentBinding));
			}
			break;
			default:
				break;
		}
		assert(currentBinding == mID && "Not matching Buffer Object!");
		assert(mCurrentTarget != Target::UNKNOWN && "Use undefined binding target!");
		assert(mAccessFreq != Access::UNKNOWN && "Use undefined access frequency!");
#endif

		GL(BufferData(static_cast<GLenum>(mCurrentTarget), size, data, static_cast<GLenum>(mAccessFreq)));
	}

	void Buffer::SubData(const unsigned short &offset, const unsigned short &size, const void *data)
	{
#ifndef NDEBUG
		int currentBinding;
		switch (mCurrentTarget)
		{
			case Target::ARRAY:
			{
				GL(GetIntegerv(GL_ARRAY_BUFFER_BINDING, &currentBinding));
			}
			break;
			case Target::ELEMENT_ARRAY:
			{
				GL(GetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &currentBinding));
			}
			break;
			default:
				break;
		}
		assert(currentBinding == mID && "Not matching Buffer Object!");
		assert(mCurrentTarget != Target::UNKNOWN && "Use undefined binding target!");
#endif

		GL(BufferSubData(static_cast<GLenum>(mCurrentTarget), offset, size, data));
	}
}

