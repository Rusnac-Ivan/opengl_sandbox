#ifndef _GL_INDEX_BUFFER_H_
#define _GL_INDEX_BUFFER_H_

#include <GLObjects/Buffer.h>

namespace gl
{
	class IndexBuffer : public Buffer
	{
			uint32_t mIndexCount;
		public:
			IndexBuffer();
			~IndexBuffer();

			void SetIndexCount(uint32_t count) { mIndexCount = count; }
			uint32_t GetIndexCount() { return  mIndexCount; }

			void Bind();

			void Data(const unsigned int& size, const void* data, UsageMode accessFreq);
			void SubData(const unsigned short& offset, const unsigned short& size, const void* data);
		private:

	};

}

#endif