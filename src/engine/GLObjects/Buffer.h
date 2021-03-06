#ifndef _GL_BUFFER_H_
#define _GL_BUFFER_H_

#include <Core/Platform.h>

namespace gl
{
	class Buffer
	{
		public:
			enum struct Type
			{
				ARRAY				= GL_ARRAY_BUFFER,
				ELEMENT_ARRAY		= GL_ELEMENT_ARRAY_BUFFER,
				UNIFORM				= GL_UNIFORM_BUFFER,

				UNKNOWN				= GL_FALSE,
			};
			enum struct UsageMode
			{
				STREAM_DRAW		= GL_STREAM_DRAW,
				STREAM_READ		= GL_STREAM_READ,
				STREAM_COPY		= GL_STREAM_COPY,
				
				STATIC_DRAW		= GL_STATIC_DRAW,
				STATIC_READ		= GL_STATIC_READ,
				STATIC_COPY		= GL_STATIC_COPY,
				
				DYNAMIC_DRAW	= GL_DYNAMIC_DRAW,
				DYNAMIC_READ	= GL_DYNAMIC_READ,
				DYNAMIC_COPY	= GL_DYNAMIC_COPY,
			};
		private:
			using ObjectId = unsigned int;

			ObjectId	mID;
			Type		mCurrentTarget;
			UsageMode	mAccessFreq;
			size_t		mByteSize;
		public:
			Buffer();
			~Buffer();

			Buffer(const Buffer&) = delete;
			Buffer(Buffer&&) noexcept;
			Buffer& operator=(const Buffer&) = delete;
			Buffer& operator=(Buffer&&) noexcept;

			
			void UnBind();
			void UnBindBase(size_t index);

			ObjectId GetObjectId() { return mID; }
			Type GetCurrentType() const { return mCurrentTarget; }
			UsageMode GetUsageMode() const { return mAccessFreq; }

		protected:
			void Bind(Type target);
			void BindBase(Type target, size_t index);

			void Data(const unsigned int& size, const void* data, UsageMode accessFreq);
			void SubData(const unsigned short& offset, const unsigned short& size, const void* data);

		private:
			void Generate();
			void Delete();

			
	};
}

#endif
