#ifndef _GL_INDEX_BUFFER_H_
#define _GL_INDEX_BUFFER_H_

#include <GLObjects/Buffer.h>

namespace gl
{
	class IndexBuffer : public Buffer
	{
		public:
			IndexBuffer();
			~IndexBuffer();

			void Bind();

			void Data(const unsigned int& size, const void* data, UsageMode accessFreq);
			void SubData(const unsigned short& offset, const unsigned short& size, const void* data);
		private:

	};

}

#endif